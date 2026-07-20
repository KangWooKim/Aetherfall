#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeLevelGoal.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;

UCLASS()
class AETHERFALL_API AAetherPrototypeLevelGoal : public AActor
{
	GENERATED_BODY()

public:
	AAetherPrototypeLevelGoal();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Level Goal")
	void ResetLevelGoal();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Level Goal")
	void RestorePrototypeCheckpointState(bool bShouldBeCompleted);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Level Goal")
	bool HasLevelGoalCompleted() const { return bHasCompleted; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Level Goal")
	FName GetGoalLabel() const { return GoalLabel; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Level Goal")
	void OnLevelGoalCompleted();

private:
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	bool CanCompleteGoal() const;
	void SetTriggerActive(bool bNewActive);
	void ShowGoalMessage(const FString& Message, const FColor& Color) const;

	UPROPERTY(VisibleAnywhere, Category = "Aetherfall|Level Goal")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Level Goal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Level Goal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> GoalMesh;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Level Goal")
	FName GoalLabel = TEXT("PrototypeExit");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Level Goal")
	bool bCompleteOnce = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Level Goal")
	bool bDisableAfterCompletion = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Level Goal")
	bool bRequireCompletedEncounterLabel = false;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Level Goal", meta = (EditCondition = "bRequireCompletedEncounterLabel"))
	FName RequiredCompletedEncounterLabel = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Level Goal|Ending")
	FName CutsceneEventLabel = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Level Goal|Feedback")
	FString CompletionFeedbackLabel;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Level Goal|Feedback")
	FColor CompletionFeedbackColor = FColor::Cyan;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Level Goal|Debug")
	bool bShowGoalDebugMessages = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Level Goal|Debug")
	bool bRouteGoalMessagesToHudOnly = true;

	bool bHasCompleted = false;
};
