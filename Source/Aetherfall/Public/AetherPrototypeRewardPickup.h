#pragma once

#include "CoreMinimal.h"
#include "AetherInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeRewardPickup.generated.h"

class UStaticMeshComponent;

UCLASS()
class AETHERFALL_API AAetherPrototypeRewardPickup : public AActor, public IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	AAetherPrototypeRewardPickup();

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Reward")
	bool HasBeenCollected() const { return bCollected; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Reward")
	FName GetRewardLabel() const { return RewardLabel; }

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Reward")
	void RestorePrototypeCheckpointState(bool bShouldBeCollected);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Prototype|Reward")
	void OnRewardCollected();

private:
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
