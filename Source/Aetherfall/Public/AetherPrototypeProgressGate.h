#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeProgressGate.generated.h"

class AAetherGameModeBase;
class UBoxComponent;
class USoundBase;
class UStaticMeshComponent;

/** 전투 완료·보상 수집 조건을 구독하고 진행 문을 열거나 복원하는 레벨 액터다. */
UCLASS()
class AETHERFALL_API AAetherPrototypeProgressGate : public AActor
{
	GENERATED_BODY()

public:
	AAetherPrototypeProgressGate();

	/** 중복 해금을 막고 진행 라벨을 기록한 뒤 충돌·효과·블루프린트 알림을 반영한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Progression")
	void UnlockGate();

	/** 현재 문을 닫고 충돌을 복구한다. 이미 기록된 해금 라벨 자체를 지우지는 않는다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Progression")
	void LockGate();

	/** 저장 상태와 보상 조건으로 문을 복원하며 일반 해금 연출은 재생하지 않는다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Progression")
	void RestorePrototypeCheckpointState(bool bShouldBeUnlocked);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Progression")
	bool IsGateUnlocked() const { return bGateUnlocked; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Progression")
	FName GetGateLabel() const { return GateLabel; }

protected:
	/** 저장된 해금 상태와 보상 조건을 먼저 반영한 뒤 해당 전투 및 보상 이벤트를 구독한다. */
	virtual void BeginPlay() override;
	/** 연결했던 게임 모드의 이벤트를 해제해 종료된 문으로 콜백이 전달되지 않게 한다. */
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

	/** 해금 여부에 맞춰 문 메시와 차단 영역의 충돌 및 가시성을 함께 갱신한다. */
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
