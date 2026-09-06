/** 상자 열림 이력과 회복 아이템 보상을 처리하고 체크포인트 복원 시 열림 상태를 맞춘다. */
#include "AetherPrototypeChest.h"

#include "AetherGameModeBase.h"
#include "AetherInventoryComponent.h"
#include "AetherfallCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAetherPrototypeChest::AAetherPrototypeChest()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ChestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChestMesh"));
	ChestMesh->SetupAttachment(SceneRoot);
	ChestMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAetherPrototypeChest::BeginPlay()
{
	Super::BeginPlay();

	bChestOpened = bStartOpened;
	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		bChestOpened = bChestOpened || GameMode->HasOpenedPrototypeChest(ChestLabel);
	}
}

FText AAetherPrototypeChest::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return bChestOpened && !bAllowRepeatedOpening ? OpenedPrompt : OpenPrompt;
}

void AAetherPrototypeChest::Interact_Implementation(AActor* Interactor)
{
	if (bChestOpened && !bAllowRepeatedOpening)
	{
		ShowChestMessage(FString::Printf(TEXT("Prototype chest already opened (%s)"), *ChestLabel.ToString()), FColor::Silver);
		return;
	}

	OpenChest(Interactor);
}

/** 반복 열기 설정을 확인한 뒤 진행 라벨과 보상을 기록하고 상호작용 플레이어에게 회복 아이템을 지급한다. */
void AAetherPrototypeChest::OpenChest(AActor* Interactor)
{
	if (bChestOpened && !bAllowRepeatedOpening)
	{
		return;
	}

	bChestOpened = true;

	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->RecordPrototypeChestOpened(ChestLabel);
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
	}

	ShowChestMessage(
		FString::Printf(TEXT("Prototype chest opened (%s) / heal items %d"), *ChestLabel.ToString(), GrantedPrototypeHealingItemCount),
		FColor::Green);
	OnChestOpened();
}

/** 기본 열림 설정과 저장 상태를 합쳐 내부 열림 상태만 복원하며 보상을 다시 지급하지 않는다. */
void AAetherPrototypeChest::RestorePrototypeCheckpointState(bool bShouldBeOpened)
{
	bChestOpened = bStartOpened || bShouldBeOpened;
}

void AAetherPrototypeChest::ShowChestMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherInteraction] %s"), *Message);

	if (bRouteChestMessagesToHudOnly)
	{
		if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			GameMode->RegisterPrototypeProgressFeedback(Message.ToUpper(), FLinearColor(Color));
		}
	}

	if (bShowChestDebugMessages && !bRouteChestMessagesToHudOnly && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
	}
}
