#include "AetherSettingsSubsystem.h"

#include "HAL/PlatformTime.h"

#include "AetherSettingsSaveGame.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "TimerManager.h"

namespace
{
	EAetherWindowMode ToAetherWindowMode(EWindowMode::Type WindowMode)
	{
		switch (WindowMode)
		{
		case EWindowMode::Fullscreen:
			return EAetherWindowMode::Fullscreen;
		case EWindowMode::Windowed:
			return EAetherWindowMode::Windowed;
		default:
			return EAetherWindowMode::Borderless;
		}
	}

	EWindowMode::Type ToEngineWindowMode(EAetherWindowMode WindowMode)
	{
		switch (WindowMode)
		{
		case EAetherWindowMode::Fullscreen:
			return EWindowMode::Fullscreen;
		case EAetherWindowMode::Windowed:
			return EWindowMode::Windowed;
		default:
			return EWindowMode::WindowedFullscreen;
		}
	}

	int32 ClampQuality(int32 Value)
	{
		return FMath::Clamp(Value, 0, 4);
	}

	struct FAetherPerformancePresetPolicy
	{
		float ResolutionScale = 100.0f;
		float FrameRateLimit = 60.0f;
		bool bMotionBlurEnabled = false;
	};

	FAetherPerformancePresetPolicy GetPerformancePresetPolicy(int32 QualityLevel)
	{
		switch (ClampQuality(QualityLevel))
		{
		case 0:
			return {70.0f, 60.0f, false};
		case 1:
			return {80.0f, 60.0f, false};
		case 2:
			return {90.0f, 60.0f, false};
		case 3:
			return {100.0f, 60.0f, false};
		case 4:
		default:
			return {100.0f, 0.0f, false};
		}
	}

	float QualityToViewDistanceScale(int32 QualityLevel)
	{
		static constexpr float Values[] = {0.55f, 0.70f, 0.85f, 1.0f, 1.0f};
		return Values[ClampQuality(QualityLevel)];
	}

	float QualityToFoliageDistanceScale(int32 QualityLevel)
	{
		static constexpr float Values[] = {0.60f, 0.75f, 0.90f, 1.0f, 1.0f};
		return Values[ClampQuality(QualityLevel)];
	}

	float QualityToFoliageDensityScale(int32 QualityLevel)
	{
		static constexpr float Values[] = {0.35f, 0.55f, 0.80f, 1.0f, 1.0f};
		return Values[ClampQuality(QualityLevel)];
	}

	float QualityToShadowDistanceScale(int32 QualityLevel)
	{
		static constexpr float Values[] = {0.55f, 0.70f, 0.90f, 1.0f, 1.0f};
		return Values[ClampQuality(QualityLevel)];
	}

	int32 QualityToSkeletalMeshLodBias(int32 QualityLevel)
	{
		static constexpr int32 Values[] = {2, 1, 0, 0, 0};
		return Values[ClampQuality(QualityLevel)];
	}

	void SetGameSettingConsoleVariable(const TCHAR* Name, float Value)
	{
		if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			Variable->Set(Value, ECVF_SetByGameSetting);
		}
	}

	void SetGameSettingConsoleVariable(const TCHAR* Name, int32 Value)
	{
		if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			Variable->Set(Value, ECVF_SetByGameSetting);
		}
	}
}

void UAetherSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if !UE_BUILD_SHIPPING
	FString ValidationSlotOverride;
	if (FParse::Value(FCommandLine::Get(), TEXT("AetherSettingsSlot="), ValidationSlotOverride)
		&& ValidationSlotOverride.StartsWith(TEXT("AetherSettings_Validation_")))
	{
		SettingsSlotName = MoveTemp(ValidationSlotOverride);
		UE_LOG(LogTemp, Display, TEXT("[AetherSettings] Using isolated validation slot %s"), *SettingsSlotName);
	}
#endif

	if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		UserSettings->LoadSettings(false);
		UserSettings->ValidateSettings();
	}

	CurrentSettings = CaptureCurrentSettings();
	MasterSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Engine/EngineSounds/Master.Master"));
	MusicSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Engine/EngineSounds/Music.Music"));
	SfxSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Engine/EngineSounds/SFX.SFX"));
	VoiceSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Engine/EngineSounds/Voice.Voice"));
	UiSoundClass = NewObject<USoundClass>(this, TEXT("AetherUiSoundClass"));
	if (UiSoundClass)
	{
		UiSoundClass->ParentClass = MasterSoundClass;
	}
	RuntimeSoundMix = NewObject<USoundMix>(this, TEXT("AetherRuntimeSettingsMix"));
	LoadCustomSettings();
	SanitizeSettings(CurrentSettings);
	PendingSettings = CurrentSettings;
	ApplyDistanceRenderingBudget(CurrentSettings.Video);
	ApplyRuntimeCustomSettings(CurrentSettings.Custom);
}

void UAetherSettingsSubsystem::Deinitialize()
{
	ClearVideoConfirmationTimer();
	if (bSoundMixPushed && RuntimeSoundMix && GetWorld())
	{
		UGameplayStatics::PopSoundMixModifier(GetWorld(), RuntimeSoundMix);
	}
	bSoundMixPushed = false;
	Super::Deinitialize();
}

void UAetherSettingsSubsystem::BeginSettingsEdit()
{
	if (!bAwaitingVideoConfirmation)
	{
		CurrentSettings.Video = CaptureCurrentSettings().Video;
	}
	PendingSettings = CurrentSettings;
}

void UAetherSettingsSubsystem::SetPendingSettings(const FAetherSettingsSnapshot& InSettings)
{
	PendingSettings = InSettings;
	SanitizeSettings(PendingSettings);
}

bool UAetherSettingsSubsystem::ApplyPendingSettings()
{
	if (bAwaitingVideoConfirmation)
	{
		return false;
	}

	SanitizeSettings(PendingSettings);
	VideoRevertSettings = CurrentSettings;
	const bool bDisplayModeChanged = HasDisplayModeChanged(CurrentSettings.Video, PendingSettings.Video);

	ApplyVideoSettings(PendingSettings.Video);
	ApplyRuntimeCustomSettings(PendingSettings.Custom);
	CurrentSettings = PendingSettings;
	SaveCustomSettings();
	OnSettingsApplied.Broadcast();

	if (bDisplayModeChanged)
	{
		bAwaitingVideoConfirmation = true;
		ClearVideoConfirmationTimer();
		VideoConfirmationDeadlineSeconds = FPlatformTime::Seconds() + VideoConfirmationDurationSeconds;
		VideoConfirmationTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UAetherSettingsSubsystem::TickVideoConfirmation),
			0.1f);
		OnVideoConfirmationChanged.Broadcast(true);
	}
	else if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		UserSettings->ConfirmVideoMode();
		UserSettings->SaveSettings();
	}

	return true;
}

void UAetherSettingsSubsystem::CancelPendingSettings()
{
	if (!bAwaitingVideoConfirmation)
	{
		PendingSettings = CurrentSettings;
	}
}

void UAetherSettingsSubsystem::RestorePendingDefaults()
{
	if (!bAwaitingVideoConfirmation)
	{
		PendingSettings = BuildDefaultSettings();
		SanitizeSettings(PendingSettings);
	}
}

FAetherSettingsSnapshot UAetherSettingsSubsystem::BuildPerformancePresetSettings(int32 QualityLevel, const FAetherSettingsSnapshot& BaseSettings) const
{
	FAetherSettingsSnapshot Preset = BaseSettings;
	const int32 ClampedQuality = ClampQuality(QualityLevel);
	const FAetherPerformancePresetPolicy Policy = GetPerformancePresetPolicy(ClampedQuality);

	Preset.Video.OverallQuality = ClampedQuality;
	Preset.Video.ViewDistanceQuality = ClampedQuality;
	Preset.Video.AntiAliasingQuality = ClampedQuality;
	Preset.Video.ShadowQuality = ClampedQuality;
	Preset.Video.GlobalIlluminationQuality = ClampedQuality;
	Preset.Video.ReflectionQuality = ClampedQuality;
	Preset.Video.TextureQuality = ClampedQuality;
	Preset.Video.EffectsQuality = ClampedQuality;
	Preset.Video.PostProcessQuality = ClampedQuality;
	Preset.Video.FoliageQuality = ClampedQuality;
	Preset.Video.ShadingQuality = ClampedQuality;
	Preset.Video.ResolutionScale = Policy.ResolutionScale;
	Preset.Video.FrameRateLimit = Policy.FrameRateLimit;
	Preset.Custom.bMotionBlurEnabled = Policy.bMotionBlurEnabled;

	SanitizeSettings(Preset);
	return Preset;
}

void UAetherSettingsSubsystem::ConfirmVideoSettings()
{
	if (!bAwaitingVideoConfirmation)
	{
		return;
	}

	ClearVideoConfirmationTimer();
	bAwaitingVideoConfirmation = false;
	if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		UserSettings->ConfirmVideoMode();
		UserSettings->SaveSettings();
	}
	OnVideoConfirmationChanged.Broadcast(false);
}

void UAetherSettingsSubsystem::RevertVideoSettings()
{
	if (!bAwaitingVideoConfirmation)
	{
		return;
	}

	ClearVideoConfirmationTimer();
	bAwaitingVideoConfirmation = false;
	ApplyVideoSettings(VideoRevertSettings.Video);
	ApplyRuntimeCustomSettings(VideoRevertSettings.Custom);
	CurrentSettings = VideoRevertSettings;
	PendingSettings = CurrentSettings;
	SaveCustomSettings();

	if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		UserSettings->ConfirmVideoMode();
		UserSettings->SaveSettings();
	}

	OnSettingsApplied.Broadcast();
	OnVideoConfirmationChanged.Broadcast(false);
}

float UAetherSettingsSubsystem::GetVideoConfirmationSecondsRemaining() const
{
	if (!bAwaitingVideoConfirmation)
	{
		return 0.0f;
	}
	return FMath::Max(0.0, VideoConfirmationDeadlineSeconds - FPlatformTime::Seconds());
}

float UAetherSettingsSubsystem::GetEffectiveAudioVolume(EAetherAudioCategory Category) const
{
	const FAetherCustomSettings& Settings = CurrentSettings.Custom;
	if (Settings.bMuteAll)
	{
		return 0.0f;
	}

	float CategoryVolume = 1.0f;
	switch (Category)
	{
	case EAetherAudioCategory::Music:
		CategoryVolume = Settings.MusicVolume;
		break;
	case EAetherAudioCategory::Sfx:
		CategoryVolume = Settings.SfxVolume;
		break;
	case EAetherAudioCategory::Voice:
		CategoryVolume = Settings.VoiceVolume;
		break;
	case EAetherAudioCategory::Ui:
		CategoryVolume = Settings.UiVolume;
		break;
	default:
		break;
	}
	return FMath::Clamp(Settings.MasterVolume * CategoryVolume, 0.0f, 1.0f);
}

TArray<FIntPoint> UAetherSettingsSubsystem::GetSupportedResolutions() const
{
	TArray<FIntPoint> Resolutions;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B)
	{
		return A.X == B.X ? A.Y < B.Y : A.X < B.X;
	});
	return Resolutions;
}

USoundClass* UAetherSettingsSubsystem::GetSoundClassForCategory(EAetherAudioCategory Category) const
{
	switch (Category)
	{
	case EAetherAudioCategory::Music:
		return MusicSoundClass;
	case EAetherAudioCategory::Sfx:
		return SfxSoundClass;
	case EAetherAudioCategory::Voice:
		return VoiceSoundClass;
	case EAetherAudioCategory::Ui:
		return UiSoundClass;
	default:
		return MasterSoundClass;
	}
}

void UAetherSettingsSubsystem::EnsureSoundMixApplied()
{
	UWorld* World = GetWorld();
	if (!World || !RuntimeSoundMix || !MasterSoundClass)
	{
		return;
	}

	if (!bSoundMixPushed)
	{
		UGameplayStatics::PushSoundMixModifier(World, RuntimeSoundMix);
		bSoundMixPushed = true;
	}

	const FAetherCustomSettings& Settings = CurrentSettings.Custom;
	const float MasterVolume = Settings.bMuteAll ? 0.0f : Settings.MasterVolume;
	UGameplayStatics::SetSoundMixClassOverride(World, RuntimeSoundMix, MasterSoundClass, MasterVolume, 1.0f, 0.05f, false);
	if (MusicSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, RuntimeSoundMix, MusicSoundClass, MasterVolume * Settings.MusicVolume, 1.0f, 0.05f, true);
	}
	if (SfxSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, RuntimeSoundMix, SfxSoundClass, MasterVolume * Settings.SfxVolume, 1.0f, 0.05f, true);
	}
	if (VoiceSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, RuntimeSoundMix, VoiceSoundClass, MasterVolume * Settings.VoiceVolume, 1.0f, 0.05f, true);
	}
	if (UiSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, RuntimeSoundMix, UiSoundClass, MasterVolume * Settings.UiVolume, 1.0f, 0.05f, true);
	}
}

FAetherSettingsSnapshot UAetherSettingsSubsystem::CaptureCurrentSettings() const
{
	FAetherSettingsSnapshot Snapshot;
	if (const UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		Snapshot.Video.WindowMode = ToAetherWindowMode(UserSettings->GetFullscreenMode());
		Snapshot.Video.Resolution = UserSettings->GetScreenResolution();
		Snapshot.Video.bVSyncEnabled = UserSettings->IsVSyncEnabled();
		Snapshot.Video.FrameRateLimit = UserSettings->GetFrameRateLimit();

		float NormalizedScale = 1.0f;
		float CurrentScale = 100.0f;
		float MinScale = 25.0f;
		float MaxScale = 100.0f;
		UserSettings->GetResolutionScaleInformationEx(NormalizedScale, CurrentScale, MinScale, MaxScale);
		Snapshot.Video.ResolutionScale = CurrentScale;
		Snapshot.Video.OverallQuality = UserSettings->GetOverallScalabilityLevel();
		Snapshot.Video.ViewDistanceQuality = UserSettings->GetViewDistanceQuality();
		Snapshot.Video.AntiAliasingQuality = UserSettings->GetAntiAliasingQuality();
		Snapshot.Video.ShadowQuality = UserSettings->GetShadowQuality();
		Snapshot.Video.GlobalIlluminationQuality = UserSettings->GetGlobalIlluminationQuality();
		Snapshot.Video.ReflectionQuality = UserSettings->GetReflectionQuality();
		Snapshot.Video.TextureQuality = UserSettings->GetTextureQuality();
		Snapshot.Video.EffectsQuality = UserSettings->GetVisualEffectQuality();
		Snapshot.Video.PostProcessQuality = UserSettings->GetPostProcessingQuality();
		Snapshot.Video.FoliageQuality = UserSettings->GetFoliageQuality();
		Snapshot.Video.ShadingQuality = UserSettings->GetShadingQuality();
	}
	return Snapshot;
}

FAetherSettingsSnapshot UAetherSettingsSubsystem::BuildDefaultSettings() const
{
	FAetherSettingsSnapshot Defaults;
	Defaults.Video.WindowMode = EAetherWindowMode::Borderless;
	Defaults.Video.Resolution = CaptureCurrentSettings().Video.Resolution;
	Defaults.Video.bVSyncEnabled = false;
	Defaults.Video.FrameRateLimit = 60.0f;
	Defaults.Video.ResolutionScale = 100.0f;
	Defaults.Video.OverallQuality = 3;
	Defaults.Video.ViewDistanceQuality = 3;
	Defaults.Video.AntiAliasingQuality = 3;
	Defaults.Video.ShadowQuality = 3;
	Defaults.Video.GlobalIlluminationQuality = 3;
	Defaults.Video.ReflectionQuality = 3;
	Defaults.Video.TextureQuality = 3;
	Defaults.Video.EffectsQuality = 3;
	Defaults.Video.PostProcessQuality = 3;
	Defaults.Video.FoliageQuality = 3;
	Defaults.Video.ShadingQuality = 3;
	Defaults.Custom = FAetherCustomSettings();
	return Defaults;
}

void UAetherSettingsSubsystem::SanitizeSettings(FAetherSettingsSnapshot& Settings) const
{
	Settings.Video.Resolution.X = FMath::Clamp(Settings.Video.Resolution.X, 640, 7680);
	Settings.Video.Resolution.Y = FMath::Clamp(Settings.Video.Resolution.Y, 360, 4320);
	const TArray<FIntPoint> SupportedResolutions = GetSupportedResolutions();
	if (Settings.Video.WindowMode != EAetherWindowMode::Windowed &&
		SupportedResolutions.Num() > 0 &&
		!SupportedResolutions.Contains(Settings.Video.Resolution))
	{
		const FIntPoint CurrentResolution = CaptureCurrentSettings().Video.Resolution;
		Settings.Video.Resolution = SupportedResolutions.Contains(CurrentResolution)
			? CurrentResolution
			: SupportedResolutions.Last();
	}
	Settings.Video.FrameRateLimit = FMath::Clamp(Settings.Video.FrameRateLimit, 0.0f, 360.0f);
	Settings.Video.ResolutionScale = FMath::Clamp(Settings.Video.ResolutionScale, 25.0f, 100.0f);
	Settings.Video.OverallQuality = FMath::Clamp(Settings.Video.OverallQuality, -1, 4);
	Settings.Video.ViewDistanceQuality = ClampQuality(Settings.Video.ViewDistanceQuality);
	Settings.Video.AntiAliasingQuality = ClampQuality(Settings.Video.AntiAliasingQuality);
	Settings.Video.ShadowQuality = ClampQuality(Settings.Video.ShadowQuality);
	Settings.Video.GlobalIlluminationQuality = ClampQuality(Settings.Video.GlobalIlluminationQuality);
	Settings.Video.ReflectionQuality = ClampQuality(Settings.Video.ReflectionQuality);
	Settings.Video.TextureQuality = ClampQuality(Settings.Video.TextureQuality);
	Settings.Video.EffectsQuality = ClampQuality(Settings.Video.EffectsQuality);
	Settings.Video.PostProcessQuality = ClampQuality(Settings.Video.PostProcessQuality);
	Settings.Video.FoliageQuality = ClampQuality(Settings.Video.FoliageQuality);
	Settings.Video.ShadingQuality = ClampQuality(Settings.Video.ShadingQuality);

	Settings.Custom.Gamma = FMath::Clamp(Settings.Custom.Gamma, 1.6f, 2.8f);
	Settings.Custom.MasterVolume = FMath::Clamp(Settings.Custom.MasterVolume, 0.0f, 1.0f);
	Settings.Custom.MusicVolume = FMath::Clamp(Settings.Custom.MusicVolume, 0.0f, 1.0f);
	Settings.Custom.SfxVolume = FMath::Clamp(Settings.Custom.SfxVolume, 0.0f, 1.0f);
	Settings.Custom.VoiceVolume = FMath::Clamp(Settings.Custom.VoiceVolume, 0.0f, 1.0f);
	Settings.Custom.UiVolume = FMath::Clamp(Settings.Custom.UiVolume, 0.0f, 1.0f);
	Settings.Custom.CameraSensitivityX = FMath::Clamp(Settings.Custom.CameraSensitivityX, 0.1f, 4.0f);
	Settings.Custom.CameraSensitivityY = FMath::Clamp(Settings.Custom.CameraSensitivityY, 0.1f, 4.0f);
	Settings.Custom.ScreenShakeScale = FMath::Clamp(Settings.Custom.ScreenShakeScale, 0.0f, 1.0f);
}

void UAetherSettingsSubsystem::LoadCustomSettings()
{
	if (!UGameplayStatics::DoesSaveGameExist(SettingsSlotName, SettingsUserIndex))
	{
		return;
	}

	const UAetherSettingsSaveGame* SaveGame = Cast<UAetherSettingsSaveGame>(UGameplayStatics::LoadGameFromSlot(SettingsSlotName, SettingsUserIndex));
	if (!SaveGame || SaveGame->SchemaVersion > CurrentSettingsSchemaVersion)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AetherSettings] Unsupported or unreadable settings save; defaults retained"));
		return;
	}

	CurrentSettings.Custom = SaveGame->Settings;
}

bool UAetherSettingsSubsystem::SaveCustomSettings() const
{
	UAetherSettingsSaveGame* SaveGame = Cast<UAetherSettingsSaveGame>(UGameplayStatics::CreateSaveGameObject(UAetherSettingsSaveGame::StaticClass()));
	if (!SaveGame)
	{
		return false;
	}

	SaveGame->SchemaVersion = CurrentSettingsSchemaVersion;
	SaveGame->SchemaLabel = FName(TEXT("AetherSettingsV1"));
	SaveGame->Settings = CurrentSettings.Custom;
	return UGameplayStatics::SaveGameToSlot(SaveGame, SettingsSlotName, SettingsUserIndex);
}

void UAetherSettingsSubsystem::ApplyVideoSettings(const FAetherVideoSettings& Settings)
{
	UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	if (!UserSettings)
	{
		return;
	}

	UserSettings->SetScreenResolution(Settings.Resolution);
	UserSettings->SetFullscreenMode(ToEngineWindowMode(Settings.WindowMode));
	UserSettings->SetVSyncEnabled(Settings.bVSyncEnabled);
	UserSettings->SetFrameRateLimit(Settings.FrameRateLimit);
	UserSettings->SetResolutionScaleValueEx(Settings.ResolutionScale);

	if (Settings.OverallQuality >= 0)
	{
		UserSettings->SetOverallScalabilityLevel(Settings.OverallQuality);
	}
	else
	{
		UserSettings->SetViewDistanceQuality(Settings.ViewDistanceQuality);
		UserSettings->SetAntiAliasingQuality(Settings.AntiAliasingQuality);
		UserSettings->SetShadowQuality(Settings.ShadowQuality);
		UserSettings->SetGlobalIlluminationQuality(Settings.GlobalIlluminationQuality);
		UserSettings->SetReflectionQuality(Settings.ReflectionQuality);
		UserSettings->SetTextureQuality(Settings.TextureQuality);
		UserSettings->SetVisualEffectQuality(Settings.EffectsQuality);
		UserSettings->SetPostProcessingQuality(Settings.PostProcessQuality);
		UserSettings->SetFoliageQuality(Settings.FoliageQuality);
		UserSettings->SetShadingQuality(Settings.ShadingQuality);
	}

	UserSettings->ApplyNonResolutionSettings();
	UserSettings->ApplyResolutionSettings(false);
	ApplyDistanceRenderingBudget(Settings);
}

void UAetherSettingsSubsystem::ApplyDistanceRenderingBudget(const FAetherVideoSettings& Settings)
{
	const int32 ViewDistanceQuality = Settings.OverallQuality >= 0 ? Settings.OverallQuality : Settings.ViewDistanceQuality;
	const int32 FoliageQuality = Settings.OverallQuality >= 0 ? Settings.OverallQuality : Settings.FoliageQuality;
	const int32 ShadowQuality = Settings.OverallQuality >= 0 ? Settings.OverallQuality : Settings.ShadowQuality;

	SetGameSettingConsoleVariable(TEXT("r.ViewDistanceScale"), QualityToViewDistanceScale(ViewDistanceQuality));
	SetGameSettingConsoleVariable(TEXT("foliage.LODDistanceScale"), QualityToFoliageDistanceScale(FoliageQuality));
	SetGameSettingConsoleVariable(TEXT("foliage.DensityScale"), QualityToFoliageDensityScale(FoliageQuality));
	SetGameSettingConsoleVariable(TEXT("grass.DensityScale"), QualityToFoliageDensityScale(FoliageQuality));
	SetGameSettingConsoleVariable(TEXT("r.Shadow.DistanceScale"), QualityToShadowDistanceScale(ShadowQuality));
	SetGameSettingConsoleVariable(TEXT("r.SkeletalMeshLODBias"), QualityToSkeletalMeshLodBias(ViewDistanceQuality));
}

void UAetherSettingsSubsystem::ApplyRuntimeCustomSettings(const FAetherCustomSettings& Settings)
{
	if (GEngine)
	{
		GEngine->DisplayGamma = Settings.Gamma;
	}

	if (IConsoleVariable* MotionBlurQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality")))
	{
		MotionBlurQuality->Set(Settings.bMotionBlurEnabled ? 4 : 0, ECVF_SetByGameSetting);
	}

	ApplyControllerRuntimeSettings(Settings);
	EnsureSoundMixApplied();
}

void UAetherSettingsSubsystem::ApplyControllerRuntimeSettings(const FAetherCustomSettings& Settings) const
{
	if (!GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* PlayerController = Iterator->Get())
		{
			PlayerController->SetDisableHaptics(!Settings.bVibrationEnabled);
			if (Settings.ScreenShakeScale <= KINDA_SMALL_NUMBER && PlayerController->PlayerCameraManager)
			{
				PlayerController->PlayerCameraManager->StopAllCameraShakes(true);
			}
		}
	}
}

bool UAetherSettingsSubsystem::HasDisplayModeChanged(const FAetherVideoSettings& A, const FAetherVideoSettings& B) const
{
	return A.WindowMode != B.WindowMode || A.Resolution != B.Resolution;
}

bool UAetherSettingsSubsystem::TickVideoConfirmation(float DeltaTime)
{
	if (!bAwaitingVideoConfirmation)
	{
		return false;
	}
	if (FPlatformTime::Seconds() < VideoConfirmationDeadlineSeconds)
	{
		return true;
	}

	VideoConfirmationTickerHandle.Reset();
	RevertVideoSettings();
	return false;
}

void UAetherSettingsSubsystem::ClearVideoConfirmationTimer()
{
	if (VideoConfirmationTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(VideoConfirmationTickerHandle);
		VideoConfirmationTickerHandle.Reset();
	}
	VideoConfirmationDeadlineSeconds = 0.0;
}
