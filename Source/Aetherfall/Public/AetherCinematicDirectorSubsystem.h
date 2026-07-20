#pragma once

#include "CoreMinimal.h"
#include "AetherCinematicTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerManager.h"
#include "AetherCinematicDirectorSubsystem.generated.h"

class APlayerController;
class UWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherCinematicChangedSignature, const FAetherCinematicRuntimeState&, RuntimeState);
DECLARE_MULTICAST_DELEGATE_OneParam(FAetherCinematicNativeChangedSignature, const FAetherCinematicRuntimeState&);

UCLASS()
class AETHERFALL_API UAetherCinematicDirectorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAetherCinematicDirectorSubsystem();

	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Cinematic")
	void RegisterCinematicDefinition(const FAetherCinematicDefinition& Definition);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Cinematic")
	bool RequestCinematicByTrigger(EAetherCinematicTrigger Trigger, FName EventLabel);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Cinematic")
	bool RequestCinematic(const FAetherCinematicDefinition& Definition);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Cinematic")
	bool SkipActiveCinematic();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Cinematic")
	void FinishActiveCinematic();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	bool IsCinematicActive() const { return ActiveState.bActive; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	bool CanSkipActiveCinematic() const { return ActiveState.bActive && ActiveState.Definition.bSkippable; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	bool ShouldBlockGameplayInput() const { return ActiveState.bActive && ActiveState.Definition.bLockPlayerInput; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	bool ShouldBlockCameraControl() const { return ActiveState.bActive && ActiveState.Definition.bLockCameraControl; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	bool ShouldHideHud() const { return ActiveState.bActive && ActiveState.Definition.bHideHud; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	FAetherCinematicRuntimeState GetActiveCinematicState() const;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Cinematic")
	FAetherCinematicChangedSignature OnCinematicStarted;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Cinematic")
	FAetherCinematicChangedSignature OnCinematicPresentationRequested;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Cinematic")
	FAetherCinematicChangedSignature OnCinematicSkipped;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Cinematic")
	FAetherCinematicChangedSignature OnCinematicFinished;

	FAetherCinematicNativeChangedSignature OnCinematicFinishedNative;

private:
	void ApplyCinematicLocks(const FAetherCinematicDefinition& Definition);
	void RestoreCinematicLocks(const FAetherCinematicDefinition& Definition);
	void FinishActiveCinematicInternal(bool bSkipped);
	void ScheduleFallbackFinish(const FAetherCinematicDefinition& Definition);
	UWorld* ResolveRuntimeWorld() const;

	TMap<EAetherCinematicTrigger, FAetherCinematicDefinition> CinematicDefinitions;

	FAetherCinematicRuntimeState ActiveState;

	TArray<TWeakObjectPtr<APlayerController>> LockedControllers;

	FTimerHandle FallbackFinishTimerHandle;
	double ActiveCinematicStartTime = 0.0;
};
