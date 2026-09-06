#pragma once

#include "CoreMinimal.h"
#include "AetherInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeLever.generated.h"

class AAetherPrototypeProgressGate;
class UStaticMeshComponent;

/** 레버 작동 이력을 관리하고 편집된 대상 진행 문들을 열어 숏컷을 연결한다. */
UCLASS()
class AETHERFALL_API AAetherPrototypeLever : public AActor, public IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	AAetherPrototypeLever();

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	/** 반복 작동 정책을 확인하고 라벨 기록 후 대상 문을 열며 새로 열린 문 개수를 표시한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Lever")
	void ActivateLever();

	/** 레버의 내부 작동 상태만 복원한다. 연결된 문의 상태는 월드 복원기의 별도 문 복원 단계에서 적용한다. */
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
