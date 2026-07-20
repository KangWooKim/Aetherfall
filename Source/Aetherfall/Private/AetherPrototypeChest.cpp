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
