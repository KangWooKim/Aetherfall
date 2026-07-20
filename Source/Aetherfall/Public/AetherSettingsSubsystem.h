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

UCLASS()
class AETHERFALL_API UAetherSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void BeginSettingsEdit();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	FAetherSettingsSnapshot GetCurrentSettings() const { return CurrentSettings; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	FAetherSettingsSnapshot GetPendingSettings() const { return PendingSettings; }

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void SetPendingSettings(const FAetherSettingsSnapshot& InSettings);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	bool ApplyPendingSettings();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void CancelPendingSettings();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void RestorePendingDefaults();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	FAetherSettingsSnapshot BuildPerformancePresetSettings(int32 QualityLevel, const FAetherSettingsSnapshot& BaseSettings) const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void ConfirmVideoSettings();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	void RevertVideoSettings();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	bool IsAwaitingVideoConfirmation() const { return bAwaitingVideoConfirmation; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	float GetVideoConfirmationSecondsRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	float GetEffectiveAudioVolume(EAetherAudioCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Settings")
	float GetScreenShakeScale() const { return CurrentSettings.Custom.ScreenShakeScale; }

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Settings")
	TArray<FIntPoint> GetSupportedResolutions() const;

	USoundClass* GetSoundClassForCategory(EAetherAudioCategory Category) const;
	void EnsureSoundMixApplied();

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Settings")
	FAetherSettingsChangedSignature OnSettingsApplied;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Settings")
	FAetherVideoConfirmationSignature OnVideoConfirmationChanged;

private:
	static constexpr int32 CurrentSettingsSchemaVersion = 1;
	static constexpr float VideoConfirmationDurationSeconds = 15.0f;

	FAetherSettingsSnapshot CaptureCurrentSettings() const;
	FAetherSettingsSnapshot BuildDefaultSettings() const;
	void SanitizeSettings(FAetherSettingsSnapshot& Settings) const;
	void LoadCustomSettings();
	bool SaveCustomSettings() const;
	void ApplyVideoSettings(const FAetherVideoSettings& Settings);
	void ApplyDistanceRenderingBudget(const FAetherVideoSettings& Settings);
	void ApplyRuntimeCustomSettings(const FAetherCustomSettings& Settings);
	void ApplyControllerRuntimeSettings(const FAetherCustomSettings& Settings) const;
	bool HasDisplayModeChanged(const FAetherVideoSettings& A, const FAetherVideoSettings& B) const;
	bool TickVideoConfirmation(float DeltaTime);
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
