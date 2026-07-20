#pragma once

#include "CoreMinimal.h"
#include "AetherInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeChest.generated.h"

class UStaticMeshComponent;

UCLASS()
class AETHERFALL_API AAetherPrototypeChest : public AActor, public IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	AAetherPrototypeChest();

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Chest")
	void OpenChest(AActor* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Chest")
	void RestorePrototypeCheckpointState(bool bShouldBeOpened);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Chest")
	bool IsChestOpened() const { return bChestOpened; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Chest")
	FName GetChestLabel() const { return ChestLabel; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Prototype|Chest")
	void OnChestOpened();

private:
	void ShowChestMessage(const FString& Message, const FColor& Color) const;

	UPROPERTY(VisibleAnywhere, Category = "Aetherfall|Prototype|Chest")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Prototype|Chest", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ChestMesh;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	FName ChestLabel = TEXT("PrototypeChest");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	FName RewardLabel = TEXT("PrototypeChestReward");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	FText OpenPrompt = FText::FromString(TEXT("OPEN CHEST"));

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	FText OpenedPrompt = FText::FromString(TEXT("CHEST OPENED"));

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	int32 GrantedPrototypeHealingItemCount = 1;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	bool bStartOpened = false;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	bool bAllowRepeatedOpening = false;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest|Debug")
	bool bShowChestDebugMessages = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest|Debug")
	bool bRouteChestMessagesToHudOnly = true;

	bool bChestOpened = false;
};
