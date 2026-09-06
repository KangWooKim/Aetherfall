#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeLevelGoal.generated.h"

class UBoxComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;

/** 필수 구간 완료 조건을 만족한 플레이어 진입을 레벨 완료 요청과 연출 이벤트로 연결한다. */
UCLASS()
class AETHERFALL_API AAetherPrototypeLevelGoal : public AActor
{
	GENERATED_BODY()

public:
	AAetherPrototypeLevelGoal();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Level Goal")
	void ResetLevelGoal();

	/** 저장된 완료 여부에 맞춰 목표 트리거의 재진입 가능 상태를 복원한다. */
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
	/** 플레이어 진입과 선행 조건을 확인하고 완료 상태를 기록한 뒤 게임 모드·Blueprint 연출에 알린다. */
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/** 선행 구간 조건이 없으면 허용하고, 있으면 게임 모드의 해당 구간 완료 이력을 확인한다. */
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
