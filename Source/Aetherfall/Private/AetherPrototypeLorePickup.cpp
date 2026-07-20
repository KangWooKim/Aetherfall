#include "AetherPrototypeLorePickup.h"

#include "AetherGameModeBase.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAetherPrototypeLorePickup::AAetherPrototypeLorePickup()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	LoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LoreMesh"));
	LoreMesh->SetupAttachment(SceneRoot);
	LoreMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAetherPrototypeLorePickup::BeginPlay()
{
	Super::BeginPlay();

	const AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	ApplyCollectedState(GameMode && GameMode->HasCollectedPrototypeLore(LoreLabel));
}

FText AAetherPrototypeLorePickup::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return bCollected ? FText::GetEmpty() : PickupPrompt;
}

void AAetherPrototypeLorePickup::Interact_Implementation(AActor* Interactor)
{
	if (bCollected)
	{
		return;
	}

	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->CollectPrototypeLore(LoreLabel);
	}

	ApplyCollectedState(true);

	const FString TitleString = LoreTitle.IsEmpty() ? LoreLabel.ToString() : LoreTitle.ToString();
	ShowLoreMessage(
		FString::Printf(TEXT("Lore collected (%s) / %s"), *LoreLabel.ToString(), *TitleString),
		FLinearColor(0.72f, 0.58f, 1.0f, 1.0f));

	if (!LoreBody.IsEmpty())
	{
		UE_LOG(LogTemp, Log, TEXT("[AetherLore] Lore body (%s): %s"), *LoreLabel.ToString(), *LoreBody.ToString());
	}

	OnLoreCollected();
}

void AAetherPrototypeLorePickup::RestorePrototypeCheckpointState(bool bShouldBeCollected)
{
	ApplyCollectedState(bShouldBeCollected);
}

void AAetherPrototypeLorePickup::ApplyCollectedState(bool bNewCollected)
{
	bCollected = bNewCollected;

	if (!LoreMesh)
	{
		return;
	}

	if (bHideAfterCollection)
	{
		LoreMesh->SetHiddenInGame(bCollected);
		LoreMesh->SetVisibility(!bCollected, true);
	}

	if (bDisableCollisionAfterCollection && bCollected)
	{
		LoreMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AAetherPrototypeLorePickup::ShowLoreMessage(const FString& Message, const FLinearColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherLore] %s"), *Message);

	if (bRouteLoreMessagesToHudOnly)
	{
		if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			GameMode->RegisterPrototypeProgressFeedback(Message.ToUpper(), Color);
		}
	}

	if (bShowLoreDebugMessages && !bRouteLoreMessagesToHudOnly && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color.ToFColor(true), Message);
	}
}
