#pragma once

#include "CoreMinimal.h"
#include "AetherInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeKeyGate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class AETHERFALL_API AAetherPrototypeKeyGate : public AActor, public IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	AAetherPrototypeKeyGate();

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Key Gate")
	void UnlockGate();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Key Gate")
	void RestorePrototypeCheckpointState(bool bShouldBeUnlocked);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Key Gate")
	bool IsGateUnlocked() const { return bGateUnlocked; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Key Gate")
	FName GetGateLabel() const { return GateLabel; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Prototype|Key Gate")
	void OnGateUnlocked();

private:
	void ApplyGateState();
	void ShowGateMessage(const FString& Message, const FColor& Color) const;

	UPROPERTY(VisibleAnywhere, Category = "Aetherfall|Prototype|Key Gate")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Prototype|Key Gate", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> GateMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Prototype|Key Gate", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> BlockerVolume;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key Gate")
	FName GateLabel = TEXT("PrototypeKeyGate");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key Gate")
	FName RequiredKeyLabel = TEXT("PrototypeKey");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key Gate")
	bool bStartLocked = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key Gate|Visual")
	bool bHideMeshWhenUnlocked = false;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key Gate|Collision")
	bool bDisableCollisionWhenUnlocked = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key Gate|Debug")
	bool bShowGateDebugMessages = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Key Gate|Debug")
	bool bRouteGateMessagesToHudOnly = true;

	bool bGateUnlocked = false;
};
