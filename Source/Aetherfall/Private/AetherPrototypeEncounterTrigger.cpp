#include "AetherPrototypeEncounterTrigger.h"

#include "AetherGameModeBase.h"
#include "AetherPrototypeEncounterDataAsset.h"
#include "AetherfallCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAetherPrototypeEncounterTrigger::AAetherPrototypeEncounterTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(SceneRoot);
	TriggerVolume->SetBoxExtent(FVector(260.0f, 220.0f, 140.0f));
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AAetherPrototypeEncounterTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AAetherPrototypeEncounterTrigger::HandleTriggerBeginOverlap);
	}

	if (const AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		const bool bShouldBeTriggered =
			(!EncounterLabel.IsNone() && GameMode->GetActivePrototypeEncounterLabel() == EncounterLabel) ||
			GameMode->HasCompletedPrototypeEncounter(EncounterLabel);
		RestorePrototypeCheckpointState(bShouldBeTriggered);
	}
}

void AAetherPrototypeEncounterTrigger::ResetEncounterTrigger()
{
	bHasTriggered = false;
	SetTriggerActive(true);
	ShowEncounterMessage(FString::Printf(TEXT("Encounter trigger reset (%s)"), *EncounterLabel.ToString()), FColor::Cyan);
}

void AAetherPrototypeEncounterTrigger::RestorePrototypeCheckpointState(bool bShouldBeTriggered)
{
	bHasTriggered = bShouldBeTriggered;
	SetTriggerActive(!(bDisableAfterTrigger && bHasTriggered));
}

void AAetherPrototypeEncounterTrigger::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bTriggerOnce && bHasTriggered)
	{
		return;
	}

	if (!Cast<AAetherfallCharacter>(OtherActor))
	{
		return;
	}

	bHasTriggered = true;
	ShowEncounterMessage(FString::Printf(TEXT("Encounter triggered (%s)"), *EncounterLabel.ToString()), FColor::Yellow);
	const FString ResolvedStartFeedbackLabel = ResolveEncounterStartFeedbackLabel();
	if (!ResolvedStartFeedbackLabel.IsEmpty())
	{
		ShowEncounterMessage(ResolvedStartFeedbackLabel, ResolveEncounterStartFeedbackColor().ToFColor(true));
	}

	if (bStartPrototypeRoundOnOverlap)
	{
		if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			if (bApplyEncounterConfigOnOverlap)
			{
				GameMode->ApplyPrototypeEncounterConfig(ResolveEncounterConfig());
			}

			GameMode->StartPrototypeCombatRoundForEncounter(EncounterLabel);
		}
	}

	OnEncounterTriggered();

	if (bDisableAfterTrigger)
	{
		SetTriggerActive(false);
	}
}

void AAetherPrototypeEncounterTrigger::SetTriggerActive(bool bNewActive)
{
	if (!TriggerVolume)
	{
		return;
	}

	TriggerVolume->SetCollisionEnabled(bNewActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	TriggerVolume->SetHiddenInGame(true);
}

const FAetherPrototypeEncounterConfig& AAetherPrototypeEncounterTrigger::ResolveEncounterConfig() const
{
	return EncounterDataAsset ? EncounterDataAsset->GetEncounterConfig() : EncounterConfig;
}

FString AAetherPrototypeEncounterTrigger::ResolveEncounterStartFeedbackLabel() const
{
	return EncounterDataAsset ? EncounterDataAsset->GetEncounterStartFeedbackLabel() : EncounterStartFeedbackLabel;
}

FLinearColor AAetherPrototypeEncounterTrigger::ResolveEncounterStartFeedbackColor() const
{
	return EncounterDataAsset ? EncounterDataAsset->GetEncounterStartFeedbackColor() : EncounterStartFeedbackColor;
}

void AAetherPrototypeEncounterTrigger::ShowEncounterMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherEncounter] %s"), *Message);

	if (bRouteEncounterMessagesToHudOnly)
	{
		if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			GameMode->RegisterPrototypeProgressFeedback(Message.ToUpper(), FLinearColor(Color));
		}
	}

	if (bShowEncounterDebugMessages && !bRouteEncounterMessagesToHudOnly && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
	}
}
