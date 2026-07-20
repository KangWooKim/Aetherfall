#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeProgressGate.generated.h"

class AAetherGameModeBase;
class UBoxComponent;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class AETHERFALL_API AAetherPrototypeProgressGate : public AActor
{
	GENERATED_BODY()

public:
	AAetherPrototypeProgressGate();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Progression")
	void UnlockGate();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Progression")
	void LockGate();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Progression")
	void RestorePrototypeCheckpointState(bool bShouldBeUnlocked);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Progression")
	bool IsGateUnlocked() const { return bGateUnlocked; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Progression")
	FName GetGateLabel() const { return GateLabel; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Progression")
	void OnGateUnlocked();

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Progression")
	void OnGateLocked();

private:
	UFUNCTION()
	void HandlePrototypeRoundCompleted();

	UFUNCTION()
	void HandlePrototypeRoundReset();

	UFUNCTION()
	void HandlePrototypeEncounterCompleted(FName EncounterLabel);

	UFUNCTION()
	void HandlePrototypeEncounterReset(FName EncounterLabel);

	UFUNCTION()
	void HandlePrototypeRewardCollected(FName RewardLabel);

	void ApplyGateState();
	void PlayGateSound(USoundBase* Sound, FName CueName) const;
	void ShowGateMessage(const FString& Message) const;
	bool ShouldUnlockFromCollectedReward(const AAetherGameModeBase* GameMode) const;

	UPROPERTY(VisibleAnywhere, Category = "Aetherfall|Progression")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Progression", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> GateMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Progression", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> BlockerVolume;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression")
	FName GateLabel = TEXT("PrototypeGate");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression")
	bool bStartLocked = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression")
	bool bUnlockOnPrototypeRoundClear = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression", meta = (EditCondition = "bUnlockOnPrototypeRoundClear"))
	bool bFilterByEncounterLabel = false;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression", meta = (EditCondition = "bUnlockOnPrototypeRoundClear && bFilterByEncounterLabel"))
	FName RequiredEncounterLabel = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression")
	bool bRelockOnPrototypeRoundReset = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression|Reward")
	bool bUnlockOnPrototypeRewardCollected = false;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression|Reward", meta = (EditCondition = "bUnlockOnPrototypeRewardCollected"))
	FName RequiredRewardLabel = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression|Feedback")
	FString GateUnlockFeedbackLabel;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression|Visual")
	bool bHideMeshWhenUnlocked = false;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression|Collision")
	bool bDisableCollisionWhenUnlocked = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression|Audio")
	TObjectPtr<USoundBase> GateUnlockSound;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression|Audio")
	TObjectPtr<USoundBase> GateLockSound;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression|Audio", meta = (ClampMin = "0.0"))
	float GateSoundVolume = 0.85f;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression|Debug")
	bool bShowGateDebugMessages = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Progression|Debug")
	bool bRouteGateMessagesToHudOnly = true;

	TWeakObjectPtr<AAetherGameModeBase> BoundGameMode;
	bool bGateUnlocked = false;
};
