#pragma once

#include "CoreMinimal.h"
#include "AetherInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeLever.generated.h"

class AAetherPrototypeProgressGate;
class UStaticMeshComponent;

UCLASS()
class AETHERFALL_API AAetherPrototypeLever : public AActor, public IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	AAetherPrototypeLever();

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Lever")
	void ActivateLever();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Lever")
	void RestorePrototypeCheckpointState(bool bShouldBeActivated);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Lever")
	bool IsLeverActivated() const { return bLeverActivated; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Lever")
	FName GetLeverLabel() const { return LeverLabel; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Prototype|Lever")
	void OnLeverActivated();

private:
	void ShowLeverMessage(const FString& Message, const FColor& Color) const;

	UPROPERTY(VisibleAnywhere, Category = "Aetherfall|Prototype|Lever")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Prototype|Lever", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> LeverMesh;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lever")
	FName LeverLabel = TEXT("PrototypeLever");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lever")
	FText UsePrompt = FText::FromString(TEXT("USE LEVER"));

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lever")
	FText ActivatedPrompt = FText::FromString(TEXT("LEVER USED"));

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lever")
	bool bStartActivated = false;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lever")
	bool bAllowRepeatedActivation = false;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lever")
	TArray<TObjectPtr<AAetherPrototypeProgressGate>> TargetGates;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lever|Debug")
	bool bShowLeverDebugMessages = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Lever|Debug")
	bool bRouteLeverMessagesToHudOnly = true;

	bool bLeverActivated = false;
};
