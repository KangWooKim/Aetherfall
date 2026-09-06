#pragma once

#include "CoreMinimal.h"
#include "AetherPrototypeEncounterConfig.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeEncounterTrigger.generated.h"

class UBoxComponent;
class UAetherPrototypeEncounterDataAsset;

/** 플레이어의 영역 진입을 구간 전투 시작으로 연결하고 저장된 구간 이력에 따라 재작동 여부를 복원한다. */
UCLASS()
class AETHERFALL_API AAetherPrototypeEncounterTrigger : public AActor
{
	GENERATED_BODY()

public:
	AAetherPrototypeEncounterTrigger();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Encounter")
	void ResetEncounterTrigger();

	/** 저장된 작동 이력과 재작동 설정에 맞춰 트리거 충돌을 복원한다. */
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
	/** 일회성 작동 조건을 확인한 뒤 선택적 구간 설정을 적용하고 게임 모드에 해당 구간 시작을 요청한다. */
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void SetTriggerActive(bool bNewActive);
	/** 데이터 에셋이 있으면 그 설정을 우선하고, 없으면 액터에 편집된 구간 설정을 사용한다. */
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
