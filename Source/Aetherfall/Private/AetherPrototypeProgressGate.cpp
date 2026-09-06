/** 전투 완료·보상 수집 조건을 구독하고 진행 문을 열거나 복원하는 레벨 액터다. */
#include "AetherPrototypeProgressGate.h"

#include "AetherAudioSettingsLibrary.h"
#include "AetherGameModeBase.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AAetherPrototypeProgressGate::AAetherPrototypeProgressGate()
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

/** 저장된 해금 상태와 보상 조건을 먼저 반영한 뒤 해당 전투 및 보상 이벤트를 구독한다. */
void AAetherPrototypeProgressGate::BeginPlay()
{
	Super::BeginPlay();

	bGateUnlocked = !bStartLocked;
	BoundGameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (AAetherGameModeBase* GameMode = BoundGameMode.Get())
	{
		bGateUnlocked = bGateUnlocked || GameMode->HasUnlockedPrototypeProgressGate(GateLabel);
		if (ShouldUnlockFromCollectedReward(GameMode))
		{
			bGateUnlocked = true;
			GameMode->RecordPrototypeProgressGateUnlocked(GateLabel);
		}
	}
	ApplyGateState();

	if (AAetherGameModeBase* GameMode = BoundGameMode.Get())
	{
		if (bUnlockOnPrototypeRoundClear)
		{
			if (bFilterByEncounterLabel)
			{
				GameMode->OnPrototypeEncounterCompleted.AddDynamic(this, &AAetherPrototypeProgressGate::HandlePrototypeEncounterCompleted);
				GameMode->OnPrototypeEncounterReset.AddDynamic(this, &AAetherPrototypeProgressGate::HandlePrototypeEncounterReset);
			}
			else
			{
				GameMode->OnPrototypeRoundCompleted.AddDynamic(this, &AAetherPrototypeProgressGate::HandlePrototypeRoundCompleted);
				GameMode->OnPrototypeCombatRoundReset.AddDynamic(this, &AAetherPrototypeProgressGate::HandlePrototypeRoundReset);
			}

			if (GameMode->IsPrototypeCombatRoundComplete())
			{
				if (!bFilterByEncounterLabel || RequiredEncounterLabel == GameMode->GetActivePrototypeEncounterLabel())
				{
					UnlockGate();
				}
			}
		}

		if (bUnlockOnPrototypeRewardCollected)
		{
			GameMode->OnPrototypeRewardCollected.AddDynamic(this, &AAetherPrototypeProgressGate::HandlePrototypeRewardCollected);
		}
	}
}

/** 연결했던 게임 모드의 이벤트를 해제해 종료된 문으로 콜백이 전달되지 않게 한다. */
void AAetherPrototypeProgressGate::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AAetherGameModeBase* GameMode = BoundGameMode.Get())
	{
		GameMode->OnPrototypeRoundCompleted.RemoveDynamic(this, &AAetherPrototypeProgressGate::HandlePrototypeRoundCompleted);
		GameMode->OnPrototypeCombatRoundReset.RemoveDynamic(this, &AAetherPrototypeProgressGate::HandlePrototypeRoundReset);
		GameMode->OnPrototypeEncounterCompleted.RemoveDynamic(this, &AAetherPrototypeProgressGate::HandlePrototypeEncounterCompleted);
		GameMode->OnPrototypeEncounterReset.RemoveDynamic(this, &AAetherPrototypeProgressGate::HandlePrototypeEncounterReset);
		GameMode->OnPrototypeRewardCollected.RemoveDynamic(this, &AAetherPrototypeProgressGate::HandlePrototypeRewardCollected);
	}

	Super::EndPlay(EndPlayReason);
}

/** 중복 해금을 막고 진행 라벨을 기록한 뒤 충돌·효과·블루프린트 알림을 반영한다. */
void AAetherPrototypeProgressGate::UnlockGate()
{
	if (bGateUnlocked)
	{
		return;
	}

	bGateUnlocked = true;
	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->RecordPrototypeProgressGateUnlocked(GateLabel);
	}
	ApplyGateState();
	PlayGateSound(GateUnlockSound, TEXT("Unlock"));
	ShowGateMessage(FString::Printf(TEXT("Progress gate unlocked (%s)"), *GateLabel.ToString()));
	if (!GateUnlockFeedbackLabel.IsEmpty())
	{
		ShowGateMessage(GateUnlockFeedbackLabel);
	}
	OnGateUnlocked();
}

/** 현재 문을 닫고 충돌을 복구한다. 이미 기록된 해금 라벨 자체를 지우지는 않는다. */
void AAetherPrototypeProgressGate::LockGate()
{
	if (!bGateUnlocked)
	{
		return;
	}

	bGateUnlocked = false;
	ApplyGateState();
	PlayGateSound(GateLockSound, TEXT("Lock"));
	ShowGateMessage(FString::Printf(TEXT("Progress gate locked (%s)"), *GateLabel.ToString()));
	OnGateLocked();
}

/** 저장 상태와 보상 조건으로 문을 복원하며 일반 해금 연출은 재생하지 않는다. */
void AAetherPrototypeProgressGate::RestorePrototypeCheckpointState(bool bShouldBeUnlocked)
{
	AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	const bool bShouldUnlockFromReward = ShouldUnlockFromCollectedReward(GameMode);
	bGateUnlocked = !bStartLocked || bShouldBeUnlocked || bShouldUnlockFromReward;
	if (bGateUnlocked && bShouldUnlockFromReward && GameMode)
	{
		GameMode->RecordPrototypeProgressGateUnlocked(GateLabel);
	}
	ApplyGateState();
}

void AAetherPrototypeProgressGate::HandlePrototypeRoundCompleted()
{
	UnlockGate();
}

void AAetherPrototypeProgressGate::HandlePrototypeRoundReset()
{
	if (bRelockOnPrototypeRoundReset && !ShouldUnlockFromCollectedReward(Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this))))
	{
		LockGate();
	}
}

void AAetherPrototypeProgressGate::HandlePrototypeEncounterCompleted(FName EncounterLabel)
{
	if (!bFilterByEncounterLabel || RequiredEncounterLabel == EncounterLabel)
	{
		UnlockGate();
	}
}

void AAetherPrototypeProgressGate::HandlePrototypeEncounterReset(FName EncounterLabel)
{
	if (bRelockOnPrototypeRoundReset &&
		(!bFilterByEncounterLabel || RequiredEncounterLabel == EncounterLabel) &&
		!ShouldUnlockFromCollectedReward(Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this))))
	{
		LockGate();
	}
}

void AAetherPrototypeProgressGate::HandlePrototypeRewardCollected(FName RewardLabel)
{
	if (bUnlockOnPrototypeRewardCollected && !RequiredRewardLabel.IsNone() && RequiredRewardLabel == RewardLabel)
	{
		UnlockGate();
	}
}

bool AAetherPrototypeProgressGate::ShouldUnlockFromCollectedReward(const AAetherGameModeBase* GameMode) const
{
	return bUnlockOnPrototypeRewardCollected &&
		!RequiredRewardLabel.IsNone() &&
		GameMode &&
		GameMode->HasCollectedPrototypeReward(RequiredRewardLabel);
}

/** 해금 여부에 맞춰 문 메시와 차단 영역의 충돌 및 가시성을 함께 갱신한다. */
void AAetherPrototypeProgressGate::ApplyGateState()
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

void AAetherPrototypeProgressGate::PlayGateSound(USoundBase* Sound, FName CueName) const
{
	if (!Sound)
	{
		return;
	}

	UAetherAudioSettingsLibrary::SpawnSoundAtLocationForCategory(
		this,
		Sound,
		GetActorLocation(),
		EAetherAudioCategory::Sfx,
		FMath::Max(0.0f, GateSoundVolume));
	UE_LOG(LogTemp, Log, TEXT("[AetherProgression] Gate sound played (%s / %s)"), *CueName.ToString(), *GateLabel.ToString());
}

void AAetherPrototypeProgressGate::ShowGateMessage(const FString& Message) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherProgression] %s"), *Message);

	if (bRouteGateMessagesToHudOnly)
	{
		if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			const FColor FeedbackColor = bGateUnlocked ? FColor::Green : FColor::Silver;
			GameMode->RegisterPrototypeProgressFeedback(Message.ToUpper(), FLinearColor(FeedbackColor));
		}
	}

	if (bShowGateDebugMessages && !bRouteGateMessagesToHudOnly && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, bGateUnlocked ? FColor::Green : FColor::Silver, Message);
	}
}
