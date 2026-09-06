#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "AetherSettingsTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AetherSettingsSubsystem.generated.h"

class USoundClass;
class USoundMix;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAetherSettingsChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherVideoConfirmationSignature, bool, bAwaitingConfirmation);

/** 현재 설정과 편집 중 설정을 분리하고 적용·저장·화면 변경 확인 및 시간 초과 복구를 조정한다. */
UCLASS()
class AETHERFALL_API UAetherSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 엔진 영상 설정과 별도 사용자 저장을 읽고 허용 범위로 보정한 뒤 런타임 설정을 적용한다. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	/** 화면 확인 틱커와 적용했던 사운드 믹스를 정리한다. */
	virtual void Deinitialize() override;

	/** 확인 대기가 아니면 실제 영상 설정을 다시 읽고 현재 스냅샷을 편집본으로 복사한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void BeginSettingsEdit();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	FAetherSettingsSnapshot GetCurrentSettings() const { return CurrentSettings; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	FAetherSettingsSnapshot GetPendingSettings() const { return PendingSettings; }

	/** 외부 편집본을 복사하고 지원 해상도와 수치 범위에 맞춰 보정한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void SetPendingSettings(const FAetherSettingsSnapshot& InSettings);

	/** 복구용 스냅샷을 보존하고 설정을 적용한다. 해상도나 창 모드가 바뀌면 실시간 기준 15초 확인 절차를 시작한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	bool ApplyPendingSettings();

	/** 화면 확인 대기가 아닐 때 편집본을 현재 적용값으로 되돌린다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void CancelPendingSettings();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void RestorePendingDefaults();

	/** 품질 단계에 맞는 렌더링 항목과 해상도 비율·프레임 제한을 구성한다. 실제 성능 측정 결과를 의미하지는 않는다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	FAetherSettingsSnapshot BuildPerformancePresetSettings(int32 QualityLevel, const FAetherSettingsSnapshot& BaseSettings) const;

	/** 확인 기한을 해제하고 현재 화면 모드를 엔진 설정에 확정·저장한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void ConfirmVideoSettings();

	/** 화면 변경 전 영상·사용자 설정을 함께 복원하고 편집본과 저장값도 맞춘다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void RevertVideoSettings();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	bool IsAwaitingVideoConfirmation() const { return bAwaitingVideoConfirmation; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	float GetVideoConfirmationSecondsRemaining() const;

	/** 전체 음소거를 우선 적용하고 마스터 볼륨과 분류별 볼륨의 곱을 반환한다. */
	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	float GetEffectiveAudioVolume(EAetherAudioCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	float GetScreenShakeScale() const { return CurrentSettings.Custom.ScreenShakeScale; }

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	TArray<FIntPoint> GetSupportedResolutions() const;

	USoundClass* GetSoundClassForCategory(EAetherAudioCategory Category) const;
	/** 사운드 믹스를 한 번 등록하고 현재 설정 스냅샷의 마스터·분류별 볼륨을 반영한다. */
	void EnsureSoundMixApplied();

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Settings")
	FAetherSettingsChangedSignature OnSettingsApplied;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Settings")
	FAetherVideoConfirmationSignature OnVideoConfirmationChanged;

private:
	static constexpr int32 CurrentSettingsSchemaVersion = 1;
	static constexpr float VideoConfirmationDurationSeconds = 15.0f;

	/** 엔진의 영상 설정을 스냅샷으로 읽는다. 별도 사용자 설정 저장은 이 함수에서 읽지 않는다. */
	FAetherSettingsSnapshot CaptureCurrentSettings() const;
	FAetherSettingsSnapshot BuildDefaultSettings() const;
	/** 수치 범위를 제한하고 지원하지 않는 전체 화면 해상도를 현재 또는 지원 해상도로 대체한다. */
	void SanitizeSettings(FAetherSettingsSnapshot& Settings) const;
	/** 이전 또는 현재 버전의 사용자 설정만 읽고 미래 버전이나 손상된 저장은 기본값으로 유지한다. */
	void LoadCustomSettings();
	bool SaveCustomSettings() const;
	/** 통합 품질 또는 개별 품질을 엔진에 적용한 뒤 해상도와 거리별 렌더링 예산을 반영한다. */
	void ApplyVideoSettings(const FAetherVideoSettings& Settings);
	/** 선택한 품질을 시야·식생·그림자 거리 및 메시 상세도 콘솔 변수에 연결한다. */
	void ApplyDistanceRenderingBudget(const FAetherVideoSettings& Settings);
	void ApplyRuntimeCustomSettings(const FAetherCustomSettings& Settings);
	void ApplyControllerRuntimeSettings(const FAetherCustomSettings& Settings) const;
	bool HasDisplayModeChanged(const FAetherVideoSettings& A, const FAetherVideoSettings& B) const;
	/** 월드 일시 정지와 별개인 플랫폼 시간을 비교해 확인 기한이 지나면 이전 설정으로 복구한다. */
	bool TickVideoConfirmation(float DeltaTime);
	/** 등록된 코어 틱커를 제거하고 확인 기한을 초기화한다. */
	void ClearVideoConfirmationTimer();

	UPROPERTY(Transient)
	FAetherSettingsSnapshot CurrentSettings;

	UPROPERTY(Transient)
	FAetherSettingsSnapshot PendingSettings;

	UPROPERTY(Transient)
	FAetherSettingsSnapshot VideoRevertSettings;

	bool bAwaitingVideoConfirmation = false;
	bool bSoundMixPushed = false;
	double VideoConfirmationDeadlineSeconds = 0.0;
	FTSTicker::FDelegateHandle VideoConfirmationTickerHandle;
	FString SettingsSlotName = TEXT("AetherSettings");
	int32 SettingsUserIndex = 0;

	UPROPERTY(Transient)
	TObjectPtr<USoundMix> RuntimeSoundMix;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> MusicSoundClass;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> SfxSoundClass;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> VoiceSoundClass;

	UPROPERTY(Transient)
	TObjectPtr<USoundClass> UiSoundClass;
};
