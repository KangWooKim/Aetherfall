/** 레버 작동 이력을 관리하고 편집된 대상 진행 문들을 열어 숏컷을 연결한다. */
#include "AetherPrototypeLever.h"

#include "AetherGameModeBase.h"
#include "AetherPrototypeProgressGate.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

AAetherPrototypeLever::AAetherPrototypeLever()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	LeverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverMesh"));
	LeverMesh->SetupAttachment(SceneRoot);
	LeverMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAetherPrototypeLever::BeginPlay()
{
	Super::BeginPlay();

	bLeverActivated = bStartActivated;
	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr))
	{
		bLeverActivated = bLeverActivated || GameMode->HasActivatedPrototypeLever(LeverLabel);
	}
	if (bLeverActivated)
	{
		for (AAetherPrototypeProgressGate* TargetGate : TargetGates)
		{
			if (TargetGate)
			{
				TargetGate->UnlockGate();
			}
		}
	}
}

FText AAetherPrototypeLever::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return bLeverActivated && !bAllowRepeatedActivation ? ActivatedPrompt : UsePrompt;
}

void AAetherPrototypeLever::Interact_Implementation(AActor* Interactor)
{
	if (bLeverActivated && !bAllowRepeatedActivation)
	{
		ShowLeverMessage(FString::Printf(TEXT("Prototype lever already activated (%s)"), *LeverLabel.ToString()), FColor::Silver);
		return;
	}

	ActivateLever();
}

/** 반복 작동 정책을 확인하고 라벨 기록 후 대상 문을 열며 새로 열린 문 개수를 표시한다. */
void AAetherPrototypeLever::ActivateLever()
{
	if (bLeverActivated && !bAllowRepeatedActivation)
	{
		return;
	}

	bLeverActivated = true;

	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr))
	{
		GameMode->RecordPrototypeLeverActivated(LeverLabel);
	}

	int32 UnlockedGateCount = 0;
	for (AAetherPrototypeProgressGate* TargetGate : TargetGates)
	{
		if (!TargetGate)
		{
			continue;
		}

		const bool bWasUnlocked = TargetGate->IsGateUnlocked();
		TargetGate->UnlockGate();
		if (!bWasUnlocked && TargetGate->IsGateUnlocked())
		{
			++UnlockedGateCount;
		}
	}

	ShowLeverMessage(
		FString::Printf(TEXT("Prototype lever activated (%s) / gates %d"), *LeverLabel.ToString(), UnlockedGateCount),
		FColor::Green);
	OnLeverActivated();
}

/** 레버의 내부 작동 상태만 복원한다. 연결된 문의 상태는 월드 복원기의 별도 문 복원 단계에서 적용한다. */
void AAetherPrototypeLever::RestorePrototypeCheckpointState(bool bShouldBeActivated)
{
	bLeverActivated = bStartActivated || bShouldBeActivated;
}

void AAetherPrototypeLever::ShowLeverMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherInteraction] %s"), *Message);

	if (bRouteLeverMessagesToHudOnly)
	{
		if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr))
		{
			GameMode->RegisterPrototypeProgressFeedback(Message.ToUpper(), FLinearColor(Color));
		}
	}

	if (bShowLeverDebugMessages && !bRouteLeverMessagesToHudOnly && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
	}
}
