#include "AetherPrototypeLevelGoal.h"

#include "AetherGameModeBase.h"
#include "AetherfallCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAetherPrototypeLevelGoal::AAetherPrototypeLevelGoal()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetupAttachment(SceneRoot);
	TriggerVolume->SetBoxExtent(FVector(180.0f, 180.0f, 140.0f));
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	GoalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GoalMesh"));
	GoalMesh->SetupAttachment(SceneRoot);
	GoalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAetherPrototypeLevelGoal::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AAetherPrototypeLevelGoal::HandleTriggerBeginOverlap);
	}

	if (const AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		const bool bShouldBeCompleted =
			GameMode->IsPrototypeLevelComplete() &&
			!GoalLabel.IsNone() &&
			GameMode->GetCompletedPrototypeLevelGoalLabel() == GoalLabel;
		RestorePrototypeCheckpointState(bShouldBeCompleted);
	}
}

void AAetherPrototypeLevelGoal::ResetLevelGoal()
{
	bHasCompleted = false;
	SetTriggerActive(true);
	ShowGoalMessage(FString::Printf(TEXT("Level goal reset (%s)"), *GoalLabel.ToString()), FColor::Cyan);
}

void AAetherPrototypeLevelGoal::RestorePrototypeCheckpointState(bool bShouldBeCompleted)
{
	bHasCompleted = bShouldBeCompleted;
	SetTriggerActive(!(bDisableAfterCompletion && bHasCompleted));
}

void AAetherPrototypeLevelGoal::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bCompleteOnce && bHasCompleted)
	{
		return;
	}

	if (!Cast<AAetherfallCharacter>(OtherActor))
	{
		return;
	}

	if (!CanCompleteGoal())
	{
		if (bRequireCompletedEncounterLabel)
		{
			ShowGoalMessage(
				FString::Printf(TEXT("Level goal locked / requires encounter (%s)"), *RequiredCompletedEncounterLabel.ToString()),
				FColor::Yellow);
		}
		return;
	}

	bHasCompleted = true;

	AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!CutsceneEventLabel.IsNone())
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[AetherLevelGoal] %s triggered (%s)"),
			*CutsceneEventLabel.ToString(),
			*GoalLabel.ToString());
	}

	if (GameMode)
	{
		GameMode->CompletePrototypeLevel(GoalLabel);
	}

	ShowGoalMessage(FString::Printf(TEXT("Level goal completed (%s)"), *GoalLabel.ToString()), FColor::Green);
	if (GameMode && !CompletionFeedbackLabel.IsEmpty())
	{
		GameMode->RegisterPrototypeProgressFeedback(CompletionFeedbackLabel.ToUpper(), FLinearColor(CompletionFeedbackColor));
	}
	OnLevelGoalCompleted();

	if (bDisableAfterCompletion)
	{
		SetTriggerActive(false);
	}
}

bool AAetherPrototypeLevelGoal::CanCompleteGoal() const
{
	if (!bRequireCompletedEncounterLabel)
	{
		return true;
	}

	const AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	return GameMode && GameMode->HasCompletedPrototypeEncounter(RequiredCompletedEncounterLabel);
}

void AAetherPrototypeLevelGoal::SetTriggerActive(bool bNewActive)
{
	if (!TriggerVolume)
	{
		return;
	}

	TriggerVolume->SetCollisionEnabled(bNewActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	TriggerVolume->SetHiddenInGame(true);
}

void AAetherPrototypeLevelGoal::ShowGoalMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherLevelGoal] %s"), *Message);

	if (bRouteGoalMessagesToHudOnly)
	{
		if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			GameMode->RegisterPrototypeProgressFeedback(Message.ToUpper(), FLinearColor(Color));
		}
	}

	if (bShowGoalDebugMessages && !bRouteGoalMessagesToHudOnly && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
	}
}
