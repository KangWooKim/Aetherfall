#include "AetherInventoryComponent.h"

#include "AetherCombatComponent.h"
#include "AetherHealthComponent.h"
#include "AetherfallCharacter.h"
#include "Engine/Engine.h"

UAetherInventoryComponent::UAetherInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAetherInventoryComponent::AddPrototypeHealingItem(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	PrototypeHealingItemCount += Amount;
	ShowInventoryMessage(
		FString::Printf(TEXT("Prototype healing item +%d / count %d"), Amount, PrototypeHealingItemCount),
		FColor::Yellow);
}

bool UAetherInventoryComponent::UsePrototypeHealingItem()
{
	if (PrototypeHealingItemCount <= 0)
	{
		ShowInventoryMessage(TEXT("No prototype healing item"), FColor::Silver);
		return false;
	}

	AAetherfallCharacter* Character = Cast<AAetherfallCharacter>(GetOwner());
	UAetherHealthComponent* HealthComponent = Character ? Character->GetHealthComponent() : nullptr;
	if (!HealthComponent || HealthComponent->IsDead())
	{
		ShowInventoryMessage(TEXT("Prototype healing item unavailable"), FColor::Silver);
		return false;
	}

	const float ActualHealAmount = HealthComponent->RestoreHealth(PrototypeHealingItemHealAmount);
	if (ActualHealAmount <= KINDA_SMALL_NUMBER)
	{
		ShowInventoryMessage(TEXT("Prototype healing item not needed"), FColor::Silver);
		return false;
	}

	--PrototypeHealingItemCount;
	if (UAetherCombatComponent* CombatComponent = Character->GetCombatComponent())
	{
		CombatComponent->NotifyOwnerHealed();
	}

	ShowInventoryMessage(
		FString::Printf(TEXT("Prototype healing item used / healed %.0f / remaining %d"), ActualHealAmount, PrototypeHealingItemCount),
		FColor::Green);
	return true;
}

void UAetherInventoryComponent::SetPrototypeHealingItemCount(int32 NewCount)
{
	PrototypeHealingItemCount = FMath::Max(0, NewCount);
	ShowInventoryMessage(
		FString::Printf(TEXT("Prototype healing item count restored / count %d"), PrototypeHealingItemCount),
		FColor::Silver);
}

void UAetherInventoryComponent::ShowInventoryMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherInventory] %s"), *Message);

	if (bShowInventoryDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
	}
}
