#pragma once

#include "CoreMinimal.h"
#include "AetherInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeKeyGate.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/** 필요한 열쇠 수집 여부를 확인해 문을 열고 열림 이력·충돌·시각 상태를 관리한다. */
UCLASS()
class AETHERFALL_API AAetherPrototypeKeyGate : public AActor, public IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	AAetherPrototypeKeyGate();

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	/** 열쇠 보유 조건이 충족될 때만 문을 열고, 부족한 경우 안내와 대응 대화를 요청한다. */
	virtual void Interact_Implementation(AActor* Interactor) override;

	/** 열림을 한 번 기록한 뒤 충돌·표시 상태를 적용하고 Blueprint 연출 이벤트를 알린다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Key Gate")
	void UnlockGate();

	/** 기본 잠금 설정과 저장된 해제 이력을 합쳐 충돌·표시를 복원한다. */
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
