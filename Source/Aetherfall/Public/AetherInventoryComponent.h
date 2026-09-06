#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherInventoryComponent.generated.h"

/** 프로토타입 회복 아이템 수량을 관리하고 실제 체력 회복이 발생한 경우에만 아이템을 소비한다. */
UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Inventory")
	void AddPrototypeHealingItem(int32 Amount = 1);

	/** 아이템 보유와 생존을 확인하고 실제 회복량이 양수일 때 수량을 차감한 뒤 전투 위험 알림 상태를 갱신한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Inventory")
	bool UsePrototypeHealingItem();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Inventory")
	int32 GetPrototypeHealingItemCount() const { return PrototypeHealingItemCount; }

	/** 저장된 아이템 수량을 0 이상으로 보정해 복원한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Inventory")
	void SetPrototypeHealingItemCount(int32 NewCount);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Inventory")
	float GetPrototypeHealingItemHealAmount() const { return PrototypeHealingItemHealAmount; }

private:
	void ShowInventoryMessage(const FString& Message, const FColor& Color) const;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Inventory|Prototype", meta = (ClampMin = "0"))
	int32 PrototypeHealingItemCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Inventory|Prototype", meta = (ClampMin = "0.0"))
	float PrototypeHealingItemHealAmount = 35.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Inventory|Prototype|Debug")
	bool bShowInventoryDebugMessages = true;
};
