#pragma once

#include "CoreMinimal.h"
#include "AetherPrototypeEncounterConfig.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeEncounterTrigger.generated.h"

class UBoxComponent;
class UAetherPrototypeEncounterDataAsset;

UCLASS()
class AETHERFALL_API AAetherPrototypeEncounterTrigger : public AActor
{
	GENERATED_BODY()

public:
	AAetherPrototypeEncounterTrigger();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Encounter")
	void ResetEncounterTrigger();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Encounter")
	void RestorePrototypeCheckpointState(bool bShouldBeTriggered);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Encounter")
	bool HasEncounterTriggered() const { return bHasTriggered; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Encounter")
	FName GetEncounterLabel() const { return EncounterLabel; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Encounter")
	void OnEncounterTriggered();

private:
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void SetTriggerActive(bool bNewActive);
	const FAetherPrototypeEncounterConfig& ResolveEncounterConfig() const;
	FString ResolveEncounterStartFeedbackLabel() const;
	FLinearColor ResolveEncounterStartFeedbackColor() const;
	void ShowEncounterMessage(const FString& Message, const FColor& Color) const;

	UPROPERTY(VisibleAnywhere, Category = "Aetherfall|Encounter")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Encounter", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Encounter")
	FName EncounterLabel = TEXT("PrototypeEncounter");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Encounter")
	bool bTriggerOnce = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Encounter")
	bool bStartPrototypeRoundOnOverlap = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Encounter")
	bool bApplyEncounterConfigOnOverlap = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Encounter", meta = (EditCondition = "bApplyEncounterConfigOnOverlap"))
	FAetherPrototypeEncounterConfig EncounterConfig;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Encounter", meta = (EditCondition = "bApplyEncounterConfigOnOverlap"))
	TObjectPtr<UAetherPrototypeEncounterDataAsset> EncounterDataAsset;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Encounter|Feedback")
	FString EncounterStartFeedbackLabel;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Encounter|Feedback")
	FLinearColor EncounterStartFeedbackColor = FLinearColor(0.62f, 0.36f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Encounter")
	bool bDisableAfterTrigger = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Encounter|Debug")
	bool bShowEncounterDebugMessages = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Encounter|Debug")
	bool bRouteEncounterMessagesToHudOnly = true;

	bool bHasTriggered = false;
};
