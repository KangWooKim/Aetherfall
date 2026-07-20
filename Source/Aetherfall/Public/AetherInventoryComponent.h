#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherInventoryComponent.generated.h"

UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Inventory")
	void AddPrototypeHealingItem(int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Inventory")
	bool UsePrototypeHealingItem();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Inventory")
	int32 GetPrototypeHealingItemCount() const { return PrototypeHealingItemCount; }

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
