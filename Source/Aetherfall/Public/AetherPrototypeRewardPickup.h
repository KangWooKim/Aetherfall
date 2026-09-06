#pragma once

#include "CoreMinimal.h"
#include "AetherInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeRewardPickup.generated.h"

class UStaticMeshComponent;

/** 라벨로 보상 수집 여부를 기록하고 최초 상호작용에서 회복 아이템과 전투 자원을 지급한다. */
UCLASS()
class AETHERFALL_API AAetherPrototypeRewardPickup : public AActor, public IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	AAetherPrototypeRewardPickup();

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	/** 미수집 보상만 기록하고 플레이어에게 자원을 지급한 뒤 수집 상태와 피드백을 반영한다. */
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Reward")
	bool HasBeenCollected() const { return bCollected; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Reward")
	FName GetRewardLabel() const { return RewardLabel; }

	/** 저장된 수집 상태만 복원하며 아이템과 자원을 다시 지급하지 않는다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Reward")
	void RestorePrototypeCheckpointState(bool bShouldBeCollected);

protected:
	/** 게임 모드의 수집 기록을 읽어 이미 획득한 보상을 숨긴다. */
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Prototype|Reward")
	void OnRewardCollected();

private:
	/** 수집된 보상의 표시와 충돌을 비활성화한다. */
	void ApplyCollectedState(bool bNewCollected);
	void ShowRewardMessage(const FString& Message) const;

	UPROPERTY(VisibleAnywhere, Category = "Aetherfall|Prototype|Reward")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Prototype|Reward", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> RewardMesh;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Reward")
	FName RewardLabel = TEXT("PrototypeReward");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Reward")
	FText PickupPrompt = FText::FromString(TEXT("TAKE REWARD"));

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Reward")
	int32 GrantedPrototypeHealingItemCount = 1;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Reward", meta = (ClampMin = "0.0"))
	float GrantedAetherGaugeAmount = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Reward")
	bool bHideAfterCollection = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Reward")
	bool bDisableCollisionAfterCollection = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Reward|Debug")
	bool bShowRewardDebugMessages = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Reward|Debug")
	bool bRouteRewardMessagesToHudOnly = true;

	bool bCollected = false;
};
