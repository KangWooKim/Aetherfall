#pragma once

#include "CoreMinimal.h"
#include "AetherInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeKeyPickup.generated.h"

class UStaticMeshComponent;

/** 열쇠 라벨의 수집과 표시 상태를 관리하고 상호작용 인터페이스 및 체크포인트 복원을 지원한다. */
UCLASS()
class AETHERFALL_API AAetherPrototypeKeyPickup : public AActor, public IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	AAetherPrototypeKeyPickup();

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	/** 미수집 상태에서 열쇠 진행 라벨을 기록하고 표시·충돌을 갱신한 뒤 수집 연출을 알린다. */
	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Key")
	bool HasBeenCollected() const { return bCollected; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Key")
	FName GetKeyLabel() const { return KeyLabel; }

	/** 획득 이벤트를 재발행하지 않고 저장된 수집 상태와 표시를 적용한다. */
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
