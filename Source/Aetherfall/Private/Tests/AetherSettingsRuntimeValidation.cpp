/** 배포 빌드에서 제외되는 설정 검증 콘솔 명령이다. 적용·취소·화면 자동 복구와 별도 프로세스 재시작 복원을 점검한다. */
#include "AetherSettingsSaveGame.h"
#include "AetherSettingsSubsystem.h"

#if !UE_BUILD_SHIPPING

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogAetherSettingsValidation, Log, All);

namespace
{
constexpr TCHAR SettingsSlotName[] = TEXT("AetherSettings");
constexpr int32 SettingsUserIndex = 0;
constexpr TCHAR RestartValidationSlotName[] = TEXT("AetherSettings_Validation_Sprint154Restart");

struct FAetherSettingsValidationState
{
	bool bRunning = false;
	bool bFailed = false;
	bool bSettingsSlotExisted = false;
	FAetherSettingsSnapshot OriginalSettings;
	TWeakObjectPtr<UAetherSettingsSubsystem> SettingsSubsystem;
	TWeakObjectPtr<UWorld> World;
	FTimerHandle CompletionTimerHandle;
};

FAetherSettingsValidationState ValidationState;

bool AreCustomSettingsEqual(const FAetherCustomSettings& A, const FAetherCustomSettings& B)
{
	return FMath::IsNearlyEqual(A.Gamma, B.Gamma)
		&& A.bMotionBlurEnabled == B.bMotionBlurEnabled
		&& FMath::IsNearlyEqual(A.MasterVolume, B.MasterVolume)
		&& FMath::IsNearlyEqual(A.MusicVolume, B.MusicVolume)
		&& FMath::IsNearlyEqual(A.SfxVolume, B.SfxVolume)
		&& FMath::IsNearlyEqual(A.VoiceVolume, B.VoiceVolume)
		&& FMath::IsNearlyEqual(A.UiVolume, B.UiVolume)
		&& A.bMuteAll == B.bMuteAll
		&& A.bSubtitlesEnabled == B.bSubtitlesEnabled
		&& A.SubtitleSize == B.SubtitleSize
		&& A.bDialogueAutoAdvance == B.bDialogueAutoAdvance
		&& FMath::IsNearlyEqual(A.CameraSensitivityX, B.CameraSensitivityX)
		&& FMath::IsNearlyEqual(A.CameraSensitivityY, B.CameraSensitivityY)
		&& A.bInvertCameraY == B.bInvertCameraY
		&& A.bVibrationEnabled == B.bVibrationEnabled
		&& FMath::IsNearlyEqual(A.ScreenShakeScale, B.ScreenShakeScale);
}

void RecordCheck(bool bCondition, const TCHAR* CheckName)
{
	ValidationState.bFailed |= !bCondition;
	UE_LOG(
		LogAetherSettingsValidation,
		Display,
		TEXT("[AetherSettingsValidation] %s: %s"),
		bCondition ? TEXT("PASS") : TEXT("FAIL"),
		CheckName);
}

UWorld* FindRuntimeWorld()
{
	if (!GEngine)
	{
		return nullptr;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* Candidate = WorldContext.World();
		if (Candidate && (WorldContext.WorldType == EWorldType::Game || WorldContext.WorldType == EWorldType::PIE))
		{
			return Candidate;
		}
	}
	return nullptr;
}

bool IsUsingRestartValidationSlot()
{
	FString SlotOverride;
	return FParse::Value(FCommandLine::Get(), TEXT("AetherSettingsSlot="), SlotOverride)
		&& SlotOverride == RestartValidationSlotName;
}

UAetherSettingsSubsystem* FindSettingsSubsystem()
{
	UWorld* World = FindRuntimeWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UAetherSettingsSubsystem>() : nullptr;
}

void ExitValidation(int32 ExitCode)
{
	ValidationState.bRunning = false;
	FPlatformMisc::RequestExitWithStatus(false, ExitCode);
}

/** 확인 대기 중인 화면 설정을 되돌린 뒤 검증 시작 전 스냅샷을 다시 적용한다. */
void RestoreOriginalSettings(UAetherSettingsSubsystem& SettingsSubsystem)
{
	if (SettingsSubsystem.IsAwaitingVideoConfirmation())
	{
		SettingsSubsystem.RevertVideoSettings();
	}

	SettingsSubsystem.BeginSettingsEdit();
	SettingsSubsystem.SetPendingSettings(ValidationState.OriginalSettings);
	if (SettingsSubsystem.ApplyPendingSettings() && SettingsSubsystem.IsAwaitingVideoConfirmation())
	{
		SettingsSubsystem.ConfirmVideoSettings();
	}
}

/** 시간 초과 복구를 확인하고 원래 설정 및 임시 슬롯을 정리한 뒤 결과 코드로 종료한다. */
void FinishSettingsRuntimeValidation()
{
	UAetherSettingsSubsystem* SettingsSubsystem = ValidationState.SettingsSubsystem.Get();
	if (!SettingsSubsystem)
	{
		UE_LOG(LogAetherSettingsValidation, Error, TEXT("[AetherSettingsValidation] Settings subsystem expired"));
		ExitValidation(1);
		return;
	}

	const FAetherSettingsSnapshot RevertedSettings = SettingsSubsystem->GetCurrentSettings();
	RecordCheck(!SettingsSubsystem->IsAwaitingVideoConfirmation(), TEXT("video confirmation timer completed"));
	RecordCheck(
		RevertedSettings.Video.WindowMode == ValidationState.OriginalSettings.Video.WindowMode
			&& RevertedSettings.Video.Resolution == ValidationState.OriginalSettings.Video.Resolution,
		TEXT("video mode automatically reverted"));
	RecordCheck(
		AreCustomSettingsEqual(RevertedSettings.Custom, ValidationState.OriginalSettings.Custom),
		TEXT("custom settings restored with video revert"));

	RestoreOriginalSettings(*SettingsSubsystem);
	RecordCheck(
		AreCustomSettingsEqual(SettingsSubsystem->GetCurrentSettings().Custom, ValidationState.OriginalSettings.Custom),
		TEXT("original settings restored after validation"));

	if (!ValidationState.bSettingsSlotExisted)
	{
		RecordCheck(
			UGameplayStatics::DeleteGameInSlot(SettingsSlotName, SettingsUserIndex),
			TEXT("temporary settings slot removed"));
	}

	UE_LOG(
		LogAetherSettingsValidation,
		Display,
		TEXT("[AetherSettingsValidation] RESULT: %s"),
		ValidationState.bFailed ? TEXT("FAIL") : TEXT("PASS"));
	ExitValidation(ValidationState.bFailed ? 1 : 0);
}

/** 설정 적용과 런타임 반영을 검사하고 화면 모드 변경 후 자동 복구를 기다린다. 실제 화면 설정이 바뀌므로 격리 실행이 필요하다. */
void RunSettingsRuntimeValidation()
{
	if (ValidationState.bRunning)
	{
		UE_LOG(LogAetherSettingsValidation, Warning, TEXT("[AetherSettingsValidation] Validation already running"));
		return;
	}

	UWorld* World = FindRuntimeWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UAetherSettingsSubsystem* SettingsSubsystem = GameInstance ? GameInstance->GetSubsystem<UAetherSettingsSubsystem>() : nullptr;
	if (!World || !SettingsSubsystem)
	{
		UE_LOG(LogAetherSettingsValidation, Error, TEXT("[AetherSettingsValidation] Runtime world or settings subsystem unavailable"));
		ExitValidation(1);
		return;
	}

	ValidationState = FAetherSettingsValidationState();
	ValidationState.bRunning = true;
	ValidationState.World = World;
	ValidationState.SettingsSubsystem = SettingsSubsystem;
	ValidationState.OriginalSettings = SettingsSubsystem->GetCurrentSettings();
	ValidationState.bSettingsSlotExisted = UGameplayStatics::DoesSaveGameExist(SettingsSlotName, SettingsUserIndex);

	if (ValidationState.bSettingsSlotExisted)
	{
		const UAetherSettingsSaveGame* ExistingSave = Cast<UAetherSettingsSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SettingsSlotName, SettingsUserIndex));
		RecordCheck(
			ExistingSave && AreCustomSettingsEqual(ExistingSave->Settings, ValidationState.OriginalSettings.Custom),
			TEXT("startup settings match persisted SaveGame"));
	}

	SettingsSubsystem->BeginSettingsEdit();
	FAetherSettingsSnapshot CancelCandidate = SettingsSubsystem->GetPendingSettings();
	CancelCandidate.Custom.MasterVolume = FMath::IsNearlyEqual(CancelCandidate.Custom.MasterVolume, 0.27f) ? 0.73f : 0.27f;
	SettingsSubsystem->SetPendingSettings(CancelCandidate);
	SettingsSubsystem->CancelPendingSettings();
	RecordCheck(
		AreCustomSettingsEqual(SettingsSubsystem->GetPendingSettings().Custom, ValidationState.OriginalSettings.Custom),
		TEXT("cancel restores pending snapshot"));

	SettingsSubsystem->BeginSettingsEdit();
	SettingsSubsystem->RestorePendingDefaults();
	const FAetherSettingsSnapshot Defaults = SettingsSubsystem->GetPendingSettings();
	RecordCheck(
		FMath::IsNearlyEqual(Defaults.Custom.Gamma, 2.2f)
			&& FMath::IsNearlyEqual(Defaults.Custom.MasterVolume, 1.0f)
			&& Defaults.Custom.bSubtitlesEnabled,
		TEXT("restore defaults produces canonical values"));
	const FAetherSettingsSnapshot LowPreset = SettingsSubsystem->BuildPerformancePresetSettings(0, Defaults);
	RecordCheck(
		LowPreset.Video.OverallQuality == 0
			&& LowPreset.Video.ViewDistanceQuality == 0
			&& LowPreset.Video.GlobalIlluminationQuality == 0
			&& LowPreset.Video.ReflectionQuality == 0
			&& FMath::IsNearlyEqual(LowPreset.Video.ResolutionScale, 70.0f)
			&& FMath::IsNearlyEqual(LowPreset.Video.FrameRateLimit, 60.0f)
			&& !LowPreset.Custom.bMotionBlurEnabled,
		TEXT("low performance preset reduces render cost drivers"));
	const FAetherSettingsSnapshot HighPreset = SettingsSubsystem->BuildPerformancePresetSettings(2, Defaults);
	RecordCheck(
		HighPreset.Video.OverallQuality == 2
			&& HighPreset.Video.ShadowQuality == 2
			&& HighPreset.Video.EffectsQuality == 2
			&& FMath::IsNearlyEqual(HighPreset.Video.ResolutionScale, 90.0f)
			&& FMath::IsNearlyEqual(HighPreset.Video.FrameRateLimit, 60.0f),
		TEXT("high performance preset keeps a bounded render budget"));

	FAetherSettingsSnapshot ApplyCandidate = ValidationState.OriginalSettings;
	ApplyCandidate.Video.OverallQuality = -1;
	ApplyCandidate.Video.ViewDistanceQuality = 0;
	ApplyCandidate.Video.ShadowQuality = 0;
	ApplyCandidate.Video.FoliageQuality = 0;
	ApplyCandidate.Custom.Gamma = FMath::IsNearlyEqual(ValidationState.OriginalSettings.Custom.Gamma, 2.35f) ? 2.1f : 2.35f;
	ApplyCandidate.Custom.bMotionBlurEnabled = !ValidationState.OriginalSettings.Custom.bMotionBlurEnabled;
	ApplyCandidate.Custom.MasterVolume = 0.65f;
	ApplyCandidate.Custom.MusicVolume = 0.5f;
	ApplyCandidate.Custom.SfxVolume = 0.4f;
	ApplyCandidate.Custom.VoiceVolume = 0.3f;
	ApplyCandidate.Custom.UiVolume = 0.2f;
	ApplyCandidate.Custom.bSubtitlesEnabled = !ValidationState.OriginalSettings.Custom.bSubtitlesEnabled;
	ApplyCandidate.Custom.SubtitleSize = EAetherSubtitleSize::Large;
	ApplyCandidate.Custom.bDialogueAutoAdvance = !ValidationState.OriginalSettings.Custom.bDialogueAutoAdvance;
	ApplyCandidate.Custom.CameraSensitivityX = 1.75f;
	ApplyCandidate.Custom.CameraSensitivityY = 0.85f;
	ApplyCandidate.Custom.bInvertCameraY = !ValidationState.OriginalSettings.Custom.bInvertCameraY;
	ApplyCandidate.Custom.bVibrationEnabled = !ValidationState.OriginalSettings.Custom.bVibrationEnabled;
	ApplyCandidate.Custom.ScreenShakeScale = 0.45f;
	SettingsSubsystem->SetPendingSettings(ApplyCandidate);
	RecordCheck(SettingsSubsystem->ApplyPendingSettings(), TEXT("custom settings apply accepted"));

	const FAetherSettingsSnapshot Applied = SettingsSubsystem->GetCurrentSettings();
	RecordCheck(
		FMath::IsNearlyEqual(Applied.Custom.MasterVolume, 0.65f)
			&& FMath::IsNearlyEqual(Applied.Custom.MusicVolume, 0.5f)
			&& Applied.Custom.bSubtitlesEnabled == ApplyCandidate.Custom.bSubtitlesEnabled
			&& Applied.Custom.SubtitleSize == EAetherSubtitleSize::Large
			&& Applied.Custom.bDialogueAutoAdvance == ApplyCandidate.Custom.bDialogueAutoAdvance
			&& FMath::IsNearlyEqual(Applied.Custom.CameraSensitivityX, 1.75f)
			&& FMath::IsNearlyEqual(Applied.Custom.CameraSensitivityY, 0.85f)
			&& Applied.Custom.bInvertCameraY == ApplyCandidate.Custom.bInvertCameraY
			&& Applied.Custom.bVibrationEnabled == ApplyCandidate.Custom.bVibrationEnabled
			&& FMath::IsNearlyEqual(Applied.Custom.ScreenShakeScale, 0.45f),
		TEXT("custom settings became current runtime state"));
	const UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings();
	RecordCheck(
		UserSettings && UserSettings->GetViewDistanceQuality() == ApplyCandidate.Video.ViewDistanceQuality,
		TEXT("view distance quality reached GameUserSettings"));
	const IConsoleVariable* ViewDistanceScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ViewDistanceScale"));
	const IConsoleVariable* FoliageDensityScale = IConsoleManager::Get().FindConsoleVariable(TEXT("foliage.DensityScale"));
	const IConsoleVariable* ShadowDistanceScale = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.DistanceScale"));
	RecordCheck(
		ViewDistanceScale && FMath::IsNearlyEqual(ViewDistanceScale->GetFloat(), 0.55f),
		TEXT("low view distance budget reached runtime console variable"));
	RecordCheck(
		FoliageDensityScale && FMath::IsNearlyEqual(FoliageDensityScale->GetFloat(), 0.35f),
		TEXT("low foliage density budget reached runtime console variable"));
	RecordCheck(
		ShadowDistanceScale && FMath::IsNearlyEqual(ShadowDistanceScale->GetFloat(), 0.55f),
		TEXT("low shadow distance budget reached runtime console variable"));
	RecordCheck(
		GEngine && FMath::IsNearlyEqual(GEngine->DisplayGamma, ApplyCandidate.Custom.Gamma),
		TEXT("gamma reached the active engine display"));
	const IConsoleVariable* MotionBlurQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MotionBlurQuality"));
	RecordCheck(
		MotionBlurQuality
			&& MotionBlurQuality->GetInt() == (ApplyCandidate.Custom.bMotionBlurEnabled ? 4 : 0),
		TEXT("motion blur reached the runtime console variable"));
	RecordCheck(
		FMath::IsNearlyEqual(SettingsSubsystem->GetEffectiveAudioVolume(EAetherAudioCategory::Music), 0.325f),
		TEXT("master and music volume affect effective output"));
	const float EffectiveSfx = SettingsSubsystem->GetEffectiveAudioVolume(EAetherAudioCategory::Sfx);
	const float EffectiveVoice = SettingsSubsystem->GetEffectiveAudioVolume(EAetherAudioCategory::Voice);
	const float EffectiveUi = SettingsSubsystem->GetEffectiveAudioVolume(EAetherAudioCategory::Ui);
	UE_LOG(
		LogAetherSettingsValidation,
		Display,
		TEXT("[AetherSettingsValidation] Effective category volume / SFX %.6f / Voice %.6f / UI %.6f"),
		EffectiveSfx,
		EffectiveVoice,
		EffectiveUi);
	RecordCheck(FMath::IsNearlyEqual(EffectiveSfx, 0.26f, KINDA_SMALL_NUMBER), TEXT("SFX volume affects effective output"));
	RecordCheck(FMath::IsNearlyEqual(EffectiveVoice, 0.195f, KINDA_SMALL_NUMBER), TEXT("Voice volume affects effective output"));
	RecordCheck(FMath::IsNearlyEqual(EffectiveUi, 0.13f, KINDA_SMALL_NUMBER), TEXT("UI volume affects effective output"));

	const UAetherSettingsSaveGame* AppliedSave = Cast<UAetherSettingsSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SettingsSlotName, SettingsUserIndex));
	RecordCheck(
		AppliedSave
			&& AppliedSave->SchemaVersion == 1
			&& AppliedSave->SchemaLabel == FName(TEXT("AetherSettingsV1"))
			&& FMath::IsNearlyEqual(AppliedSave->Settings.MasterVolume, 0.65f)
			&& FMath::IsNearlyEqual(AppliedSave->Settings.MusicVolume, 0.5f),
		TEXT("custom settings persisted with versioned schema"));

	RestoreOriginalSettings(*SettingsSubsystem);

	FAetherSettingsSnapshot VideoCandidate = ValidationState.OriginalSettings;
	VideoCandidate.Video.WindowMode = ValidationState.OriginalSettings.Video.WindowMode == EAetherWindowMode::Windowed
		? EAetherWindowMode::Borderless
		: EAetherWindowMode::Windowed;
	SettingsSubsystem->BeginSettingsEdit();
	SettingsSubsystem->SetPendingSettings(VideoCandidate);
	RecordCheck(SettingsSubsystem->ApplyPendingSettings(), TEXT("video mode change apply accepted"));
	RecordCheck(SettingsSubsystem->IsAwaitingVideoConfirmation(), TEXT("video confirmation timer started"));
	RecordCheck(SettingsSubsystem->GetVideoConfirmationSecondsRemaining() > 10.0f, TEXT("video confirmation exposes remaining time"));

	World->GetTimerManager().SetTimer(
		ValidationState.CompletionTimerHandle,
		FTimerDelegate::CreateStatic(&FinishSettingsRuntimeValidation),
		16.0f,
		false);
	UE_LOG(LogAetherSettingsValidation, Display, TEXT("[AetherSettingsValidation] Waiting for automatic video revert"));
}

/** 명령행에 지정된 전용 슬롯인지 확인한 뒤 다음 프로세스에서 읽을 설정을 기록한다. */
void PrepareSettingsRestartValidation()
{
	if (!IsUsingRestartValidationSlot())
	{
		UE_LOG(LogAetherSettingsValidation, Error, TEXT("[AetherSettingsRestartValidation] Missing isolated slot override"));
		FPlatformMisc::RequestExitWithStatus(false, 1);
		return;
	}

	UGameplayStatics::DeleteGameInSlot(RestartValidationSlotName, SettingsUserIndex);
	UAetherSettingsSubsystem* SettingsSubsystem = FindSettingsSubsystem();
	if (!SettingsSubsystem)
	{
		UE_LOG(LogAetherSettingsValidation, Error, TEXT("[AetherSettingsRestartValidation] Settings subsystem unavailable"));
		FPlatformMisc::RequestExitWithStatus(false, 1);
		return;
	}

	SettingsSubsystem->BeginSettingsEdit();
	FAetherSettingsSnapshot Candidate = SettingsSubsystem->GetPendingSettings();
	Candidate.Custom.MasterVolume = 0.37f;
	Candidate.Custom.MusicVolume = 0.41f;
	Candidate.Custom.bSubtitlesEnabled = false;
	Candidate.Custom.CameraSensitivityX = 1.83f;
	Candidate.Custom.bInvertCameraY = true;
	SettingsSubsystem->SetPendingSettings(Candidate);
	const bool bApplied = SettingsSubsystem->ApplyPendingSettings();
	const bool bSaved = UGameplayStatics::DoesSaveGameExist(RestartValidationSlotName, SettingsUserIndex);
	const bool bSuccess = bApplied && bSaved;

	UE_LOG(
		LogAetherSettingsValidation,
		Display,
		TEXT("[AetherSettingsRestartValidation] PREPARE RESULT: %s"),
		bSuccess ? TEXT("PASS") : TEXT("FAIL"));
	if (!bSuccess)
	{
		UGameplayStatics::DeleteGameInSlot(RestartValidationSlotName, SettingsUserIndex);
	}
	FPlatformMisc::RequestExitWithStatus(false, bSuccess ? 0 : 1);
}

/** 시작 시 전용 슬롯의 설정이 복원되었는지 확인하고 검증 슬롯을 삭제한 뒤 종료한다. */
void ValidateSettingsRestartRestore()
{
	if (!IsUsingRestartValidationSlot())
	{
		UE_LOG(LogAetherSettingsValidation, Error, TEXT("[AetherSettingsRestartValidation] Missing isolated slot override"));
		FPlatformMisc::RequestExitWithStatus(false, 1);
		return;
	}

	UAetherSettingsSubsystem* SettingsSubsystem = FindSettingsSubsystem();
	const UAetherSettingsSaveGame* SaveGame = Cast<UAetherSettingsSaveGame>(
		UGameplayStatics::LoadGameFromSlot(RestartValidationSlotName, SettingsUserIndex));
	const FAetherSettingsSnapshot RestoredSnapshot = SettingsSubsystem
		? SettingsSubsystem->GetCurrentSettings()
		: FAetherSettingsSnapshot();
	const FAetherCustomSettings* Restored = SettingsSubsystem ? &RestoredSnapshot.Custom : nullptr;
	const bool bSuccess = Restored
		&& SaveGame
		&& SaveGame->SchemaVersion == 1
		&& SaveGame->SchemaLabel == FName(TEXT("AetherSettingsV1"))
		&& FMath::IsNearlyEqual(Restored->MasterVolume, 0.37f)
		&& FMath::IsNearlyEqual(Restored->MusicVolume, 0.41f)
		&& !Restored->bSubtitlesEnabled
		&& FMath::IsNearlyEqual(Restored->CameraSensitivityX, 1.83f)
		&& Restored->bInvertCameraY;
	const bool bDeleted = UGameplayStatics::DeleteGameInSlot(RestartValidationSlotName, SettingsUserIndex);

	UE_LOG(
		LogAetherSettingsValidation,
		Display,
		TEXT("[AetherSettingsRestartValidation] RESTORE RESULT: %s / cleanup: %s"),
		bSuccess ? TEXT("PASS") : TEXT("FAIL"),
		bDeleted ? TEXT("PASS") : TEXT("FAIL"));
	FPlatformMisc::RequestExitWithStatus(false, bSuccess && bDeleted ? 0 : 1);
}

FAutoConsoleCommand RunSettingsValidationCommand(
	TEXT("Aether.Settings.ValidateRuntime"),
	TEXT("Runs the non-shipping Aetherfall settings runtime validation and exits with its result."),
	FConsoleCommandDelegate::CreateStatic(&RunSettingsRuntimeValidation));

FAutoConsoleCommand PrepareSettingsRestartValidationCommand(
	TEXT("Aether.Settings.PrepareRestartValidation"),
	TEXT("Writes isolated custom settings for the next validation process and exits."),
	FConsoleCommandDelegate::CreateStatic(&PrepareSettingsRestartValidation));

FAutoConsoleCommand ValidateSettingsRestartRestoreCommand(
	TEXT("Aether.Settings.ValidateRestartRestore"),
	TEXT("Validates isolated startup settings restoration, removes the fixture, and exits."),
	FConsoleCommandDelegate::CreateStatic(&ValidateSettingsRestartRestore));
}

#endif
