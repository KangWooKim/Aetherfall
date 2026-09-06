/** 라벨로 보상 수집 여부를 기록하고 최초 상호작용에서 회복 아이템과 전투 자원을 지급한다. */
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

/** 게임 모드의 수집 기록을 읽어 이미 획득한 보상을 숨긴다. */
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

/** 미수집 보상만 기록하고 플레이어에게 자원을 지급한 뒤 수집 상태와 피드백을 반영한다. */
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

/** 저장된 수집 상태만 복원하며 아이템과 자원을 다시 지급하지 않는다. */
void AAetherPrototypeRewardPickup::RestorePrototypeCheckpointState(bool bShouldBeCollected)
{
	ApplyCollectedState(bShouldBeCollected);
}

/** 수집된 보상의 표시와 충돌을 비활성화한다. */
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
