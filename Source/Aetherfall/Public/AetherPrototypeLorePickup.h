#pragma once

#include "CoreMinimal.h"
#include "AetherInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeLorePickup.generated.h"

class UStaticMeshComponent;

UCLASS()
class AETHERFALL_API AAetherPrototypeLorePickup : public AActor, public IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	AAetherPrototypeLorePickup();

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Lore")
	bool HasBeenCollected() const { return bCollected; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Lore")
	FName GetLoreLabel() const { return LoreLabel; }

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Lore")
	void RestorePrototypeCheckpointState(bool bShouldBeCollected);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Prototype|Lore")
	void OnLoreCollected();

private:
	void ApplyCollectedState(bool bNewCollected);
	void ShowLoreMessage(const FString& Message, const FLinearColor& Color) const;

	UPROPERTY(VisibleAnywhere, Category = "Aetherfall|Prototype|Lore")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Prototype|Lore", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> LoreMesh;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lore")
	FName LoreLabel = TEXT("PrototypeLore");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lore")
	FText LoreTitle = FText::FromString(TEXT("LORE RECORD"));

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lore", meta = (MultiLine = "true"))
	FText LoreBody = FText::GetEmpty();

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lore")
	FText PickupPrompt = FText::FromString(TEXT("READ LORE"));

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lore")
	bool bHideAfterCollection = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lore")
	bool bDisableCollisionAfterCollection = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lore|Debug")
	bool bShowLoreDebugMessages = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lore|Debug")
	bool bRouteLoreMessagesToHudOnly = true;

	bool bCollected = false;
};
