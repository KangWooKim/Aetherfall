#include "AetherPrototypeKeyPickup.h"

#include "AetherGameModeBase.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAetherPrototypeKeyPickup::AAetherPrototypeKeyPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	KeyMesh->SetupAttachment(SceneRoot);
	KeyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAetherPrototypeKeyPickup::BeginPlay()
{
	Super::BeginPlay();

	const AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	ApplyCollectedState(GameMode && GameMode->HasCollectedPrototypeKey(KeyLabel));
}

FText AAetherPrototypeKeyPickup::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return bCollected ? FText::GetEmpty() : PickupPrompt;
}

void AAetherPrototypeKeyPickup::Interact_Implementation(AActor* Interactor)
{
	if (bCollected)
	{
		return;
	}

	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->CollectPrototypeKey(KeyLabel);
	}

	ApplyCollectedState(true);

	ShowKeyMessage(FString::Printf(TEXT("Prototype key collected (%s)"), *KeyLabel.ToString()));
	OnKeyCollected();
}

void AAetherPrototypeKeyPickup::RestorePrototypeCheckpointState(bool bShouldBeCollected)
{
	ApplyCollectedState(bShouldBeCollected);
}

void AAetherPrototypeKeyPickup::ApplyCollectedState(bool bNewCollected)
{
	bCollected = bNewCollected;

	if (!KeyMesh)
	{
		return;
	}

	if (bHideAfterCollection)
	{
		KeyMesh->SetHiddenInGame(bCollected);
		KeyMesh->SetVisibility(!bCollected, true);
	}

	if (bDisableCollisionAfterCollection && bCollected)
	{
		KeyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AAetherPrototypeKeyPickup::ShowKeyMessage(const FString& Message) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherInteraction] %s"), *Message);

	if (bRouteKeyMessagesToHudOnly)
	{
		if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			GameMode->RegisterPrototypeProgressFeedback(Message.ToUpper(), FLinearColor(FColor::Yellow));
		}
	}

	if (bShowKeyDebugMessages && !bRouteKeyMessagesToHudOnly && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, Message);
	}
}
