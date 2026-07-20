#include "AetherPrototypeRewardPickup.h"

#include "AetherCombatComponent.h"
#include "AetherGameModeBase.h"
#include "AetherInventoryComponent.h"
#include "AetherfallCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAetherPrototypeRewardPickup::AAetherPrototypeRewardPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	RewardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RewardMesh"));
	RewardMesh->SetupAttachment(SceneRoot);
	RewardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAetherPrototypeRewardPickup::BeginPlay()
{
	Super::BeginPlay();

	const AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	ApplyCollectedState(GameMode && GameMode->HasCollectedPrototypeReward(RewardLabel));
}

FText AAetherPrototypeRewardPickup::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return bCollected ? FText::GetEmpty() : PickupPrompt;
}

void AAetherPrototypeRewardPickup::Interact_Implementation(AActor* Interactor)
{
	if (bCollected)
	{
		return;
	}

	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->CollectPrototypeReward(RewardLabel);
	}

	if (AAetherfallCharacter* Character = Cast<AAetherfallCharacter>(Interactor))
	{
		if (UAetherInventoryComponent* InventoryComponent = Character->GetInventoryComponent())
		{
			InventoryComponent->AddPrototypeHealingItem(GrantedPrototypeHealingItemCount);
		}

		if (UAetherCombatComponent* CombatComponent = Character->GetCombatComponent())
		{
			CombatComponent->GrantPrototypeAetherGauge(
				GrantedAetherGaugeAmount,
				FString::Printf(TEXT("Reward %s"), *RewardLabel.ToString()));
		}
	}

	ApplyCollectedState(true);

	ShowRewardMessage(FString::Printf(TEXT("Prototype reward collected (%s)"), *RewardLabel.ToString()));
	OnRewardCollected();
}

void AAetherPrototypeRewardPickup::RestorePrototypeCheckpointState(bool bShouldBeCollected)
{
	ApplyCollectedState(bShouldBeCollected);
}

void AAetherPrototypeRewardPickup::ApplyCollectedState(bool bNewCollected)
{
	bCollected = bNewCollected;

	if (!RewardMesh)
	{
		return;
	}

	if (bHideAfterCollection)
	{
		RewardMesh->SetHiddenInGame(bCollected);
		RewardMesh->SetVisibility(!bCollected, true);
	}

	if (bDisableCollisionAfterCollection && bCollected)
	{
		RewardMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AAetherPrototypeRewardPickup::ShowRewardMessage(const FString& Message) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherInteraction] %s"), *Message);

	if (bRouteRewardMessagesToHudOnly)
	{
		if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			GameMode->RegisterPrototypeProgressFeedback(Message.ToUpper(), FLinearColor(FColor::Orange));
		}
	}

	if (bShowRewardDebugMessages && !bRouteRewardMessagesToHudOnly && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Orange, Message);
	}
}
