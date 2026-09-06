#pragma once

#include "CoreMinimal.h"
#include "AetherInteractableInterface.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeChest.generated.h"

class UStaticMeshComponent;

/** 상자 열림 이력과 회복 아이템 보상을 처리하고 체크포인트 복원 시 열림 상태를 맞춘다. */
UCLASS()
class AETHERFALL_API AAetherPrototypeChest : public AActor, public IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	AAetherPrototypeChest();

	virtual FText GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	/** 반복 열기 설정을 확인한 뒤 진행 라벨과 보상을 기록하고 상호작용 플레이어에게 회복 아이템을 지급한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Chest")
	void OpenChest(AActor* Interactor);

	/** 기본 열림 설정과 저장 상태를 합쳐 내부 열림 상태만 복원하며 보상을 다시 지급하지 않는다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Chest")
	void RestorePrototypeCheckpointState(bool bShouldBeOpened);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Chest")
	bool IsChestOpened() const { return bChestOpened; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Chest")
	FName GetChestLabel() const { return ChestLabel; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Prototype|Chest")
	void OnChestOpened();

private:
	void ShowChestMessage(const FString& Message, const FColor& Color) const;

	UPROPERTY(VisibleAnywhere, Category = "Aetherfall|Prototype|Chest")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Prototype|Chest", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ChestMesh;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	FName ChestLabel = TEXT("PrototypeChest");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	FName RewardLabel = TEXT("PrototypeChestReward");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	FText OpenPrompt = FText::FromString(TEXT("OPEN CHEST"));

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	FText OpenedPrompt = FText::FromString(TEXT("CHEST OPENED"));

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	int32 GrantedPrototypeHealingItemCount = 1;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	bool bStartOpened = false;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest")
	bool bAllowRepeatedOpening = false;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest|Debug")
	bool bShowChestDebugMessages = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Prototype|Chest|Debug")
	bool bRouteChestMessagesToHudOnly = true;

	bool bChestOpened = false;
};
