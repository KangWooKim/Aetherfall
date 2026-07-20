#pragma once

#include "CoreMinimal.h"
#include "AetherInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeKeyPickup.generated.h"

class UStaticMeshComponent;

UCLASS()
class AETHERFALL_API AAetherPrototypeKeyPickup : public AActor, public IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	AAetherPrototypeKeyPickup();

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Key")
	bool HasBeenCollected() const { return bCollected; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Key")
	FName GetKeyLabel() const { return KeyLabel; }

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Key")
	void RestorePrototypeCheckpointState(bool bShouldBeCollected);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Prototype|Key")
	void OnKeyCollected();

private:
	void ApplyCollectedState(bool bNewCollected);
	void ShowKeyMessage(const FString& Message) const;

	UPROPERTY(VisibleAnywhere, Category = "Aetherfall|Prototype|Key")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Prototype|Key", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> KeyMesh;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key")
	FName KeyLabel = TEXT("PrototypeKey");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key")
	FText PickupPrompt = FText::FromString(TEXT("TAKE KEY"));

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key")
	bool bHideAfterCollection = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key")
	bool bDisableCollisionAfterCollection = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key|Debug")
	bool bShowKeyDebugMessages = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key|Debug")
	bool bRouteKeyMessagesToHudOnly = true;

	bool bCollected = false;
};
