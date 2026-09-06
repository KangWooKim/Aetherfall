/** 영상 설정과 사용자 설정을 정의하고 편집·적용·복구에 사용할 스냅샷으로 묶는다. */
#pragma once

#include "CoreMinimal.h"
#include "AetherSettingsTypes.generated.h"

/** 메뉴의 창 모드 선택값을 엔진 창 모드로 변환하기 위한 구분이다. */
UENUM(BlueprintType)
enum class EAetherWindowMode : uint8
{
	Fullscreen,
	Borderless,
	Windowed
};

/** 자막 표시 계층에서 사용할 크기 선택값이다. */
UENUM(BlueprintType)
enum class EAetherSubtitleSize : uint8
{
	Small,
	Medium,
	Large
};

/** 사운드 재생을 설정 서브시스템의 분류별 볼륨에 연결한다. */
UENUM(BlueprintType)
enum class EAetherAudioCategory : uint8
{
	Master,
	Music,
	Sfx,
	Voice,
	Ui
};

/** 엔진 영상 설정에 적용할 화면 모드·해상도·품질 값을 묶는다. */
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

	/** 0 이상이면 통합 품질을 사용하고 -1이면 개별 품질 항목을 적용한다. */
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

/** 별도 SaveGame으로 저장하는 오디오·자막·카메라·접근성 설정이다. */
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

/** 영상과 사용자 설정을 함께 복사하여 편집·취소·화면 변경 복구에 사용한다. */
USTRUCT(BlueprintType)
struct AETHERFALL_API FAetherSettingsSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FAetherVideoSettings Video;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FAetherCustomSettings Custom;
};
