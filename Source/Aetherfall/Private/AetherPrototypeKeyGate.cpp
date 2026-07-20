#include "AetherPrototypeKeyGate.h"

#include "AetherGameModeBase.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAetherPrototypeKeyGate::AAetherPrototypeKeyGate()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetupAttachment(SceneRoot);
	GateMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GateMesh->SetCollisionObjectType(ECC_WorldStatic);
	GateMesh->SetCollisionResponseToAllChannels(ECR_Block);

	BlockerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockerVolume"));
	BlockerVolume->SetupAttachment(SceneRoot);
	BlockerVolume->SetBoxExtent(FVector(120.0f, 24.0f, 160.0f));
	BlockerVolume->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockerVolume->SetCollisionObjectType(ECC_WorldStatic);
	BlockerVolume->SetCollisionResponseToAllChannels(ECR_Block);
}

void AAetherPrototypeKeyGate::BeginPlay()
{
	Super::BeginPlay();

	bGateUnlocked = !bStartLocked;
	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		bGateUnlocked = bGateUnlocked || GameMode->HasUnlockedPrototypeKeyGate(GateLabel);
	}
	ApplyGateState();
}

FText AAetherPrototypeKeyGate::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	if (bGateUnlocked)
	{
		return FText::FromString(TEXT("GATE OPEN"));
	}

	const AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (GameMode && GameMode->HasCollectedPrototypeKey(RequiredKeyLabel))
	{
		return FText::FromString(TEXT("OPEN GATE"));
	}

	return FText::Format(
		FText::FromString(TEXT("LOCKED - NEED {0}")),
		FText::FromName(RequiredKeyLabel));
}

void AAetherPrototypeKeyGate::Interact_Implementation(AActor* Interactor)
{
	if (bGateUnlocked)
	{
		ShowGateMessage(FString::Printf(TEXT("Key gate already unlocked (%s)"), *GateLabel.ToString()), FColor::Silver);
		return;
	}

	AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!GameMode || !GameMode->HasCollectedPrototypeKey(RequiredKeyLabel))
	{
		ShowGateMessage(
			FString::Printf(TEXT("Key gate locked / requires key (%s)"), *RequiredKeyLabel.ToString()),
			FColor::Yellow);
		if (GameMode)
		{
			GameMode->TryStartPrototypeDialogue(
				FName(*FString::Printf(TEXT("KeyGateLocked_%s"), *GateLabel.ToString())));
		}
		return;
	}

	UnlockGate();
}

void AAetherPrototypeKeyGate::UnlockGate()
{
	if (bGateUnlocked)
	{
		return;
	}

	bGateUnlocked = true;
	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->RecordPrototypeKeyGateUnlocked(GateLabel);
	}
	ApplyGateState();
	ShowGateMessage(FString::Printf(TEXT("Key gate unlocked (%s)"), *GateLabel.ToString()), FColor::Green);
	OnGateUnlocked();
}

void AAetherPrototypeKeyGate::RestorePrototypeCheckpointState(bool bShouldBeUnlocked)
{
	bGateUnlocked = !bStartLocked || bShouldBeUnlocked;
	ApplyGateState();
}

void AAetherPrototypeKeyGate::ApplyGateState()
{
	const bool bShouldBlock = !bGateUnlocked || !bDisableCollisionWhenUnlocked;
	const ECollisionEnabled::Type CollisionState = bShouldBlock ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision;

	if (GateMesh)
	{
		GateMesh->SetCollisionEnabled(CollisionState);
		GateMesh->SetHiddenInGame(bGateUnlocked && bHideMeshWhenUnlocked);
	}

	if (BlockerVolume)
	{
		BlockerVolume->SetCollisionEnabled(CollisionState);
		BlockerVolume->SetHiddenInGame(true);
	}
}

void AAetherPrototypeKeyGate::ShowGateMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherInteraction] %s"), *Message);

	if (bRouteGateMessagesToHudOnly)
	{
		if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			GameMode->RegisterPrototypeProgressFeedback(Message.ToUpper(), FLinearColor(Color));
		}
	}

	if (bShowGateDebugMessages && !bRouteGateMessagesToHudOnly && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
	}
}
