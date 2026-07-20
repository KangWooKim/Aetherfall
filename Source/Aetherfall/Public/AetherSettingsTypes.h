#pragma once

#include "CoreMinimal.h"
#include "AetherSettingsTypes.generated.h"

UENUM(BlueprintType)
enum class EAetherWindowMode : uint8
{
	Fullscreen,
	Borderless,
	Windowed
};

UENUM(BlueprintType)
enum class EAetherSubtitleSize : uint8
{
	Small,
	Medium,
	Large
};

UENUM(BlueprintType)
enum class EAetherAudioCategory : uint8
{
	Master,
	Music,
	Sfx,
	Voice,
	Ui
};

USTRUCT(BlueprintType)
struct AETHERFALL_API FAetherVideoSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video")
	EAetherWindowMode WindowMode = EAetherWindowMode::Borderless;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video")
	FIntPoint Resolution = FIntPoint(1920, 1080);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video")
	bool bVSyncEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float FrameRateLimit = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video", meta = (ClampMin = "25.0", ClampMax = "100.0"))
	float ResolutionScale = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "-1", ClampMax = "4"))
	int32 OverallQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "0", ClampMax = "4"))
	int32 ViewDistanceQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "0", ClampMax = "4"))
	int32 AntiAliasingQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "0", ClampMax = "4"))
	int32 ShadowQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "0", ClampMax = "4"))
	int32 GlobalIlluminationQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "0", ClampMax = "4"))
	int32 ReflectionQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "0", ClampMax = "4"))
	int32 TextureQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "0", ClampMax = "4"))
	int32 EffectsQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "0", ClampMax = "4"))
	int32 PostProcessQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "0", ClampMax = "4"))
	int32 FoliageQuality = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics", meta = (ClampMin = "0", ClampMax = "4"))
	int32 ShadingQuality = 3;
};

USTRUCT(BlueprintType)
struct AETHERFALL_API FAetherCustomSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display", meta = (ClampMin = "1.6", ClampMax = "2.8"))
	float Gamma = 2.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Graphics")
	bool bMotionBlurEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MusicVolume = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SfxVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VoiceVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float UiVolume = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bMuteAll = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	bool bSubtitlesEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility")
	EAetherSubtitleSize SubtitleSize = EAetherSubtitleSize::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bDialogueAutoAdvance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "0.1", ClampMax = "4.0"))
	float CameraSensitivityX = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", meta = (ClampMin = "0.1", ClampMax = "4.0"))
	float CameraSensitivityY = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bInvertCameraY = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bVibrationEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ScreenShakeScale = 1.0f;
};

USTRUCT(BlueprintType)
struct AETHERFALL_API FAetherSettingsSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FAetherVideoSettings Video;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FAetherCustomSettings Custom;
};
