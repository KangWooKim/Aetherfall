/** 플레이어 진입을 체크포인트 활성화 요청으로 바꾸고 일회성 작동과 진행 초기화 후 재진입 조건을 관리한다. */
#include "AetherPrototypeCheckpoint.h"

#include "AetherGameModeBase.h"
#include "AetherfallCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

AAetherPrototypeCheckpoint::AAetherPrototypeCheckpoint()
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

	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CheckpointMesh"));
	CheckpointMesh->SetupAttachment(SceneRoot);
	CheckpointMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAetherPrototypeCheckpoint::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AAetherPrototypeCheckpoint::HandleTriggerBeginOverlap);
		TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AAetherPrototypeCheckpoint::HandleTriggerEndOverlap);
	}
}

void AAetherPrototypeCheckpoint::ResetCheckpoint()
{
	bHasActivated = false;
	SetTriggerActive(true);
	ShowCheckpointMessage(FString::Printf(TEXT("Checkpoint reset (%s)"), *CheckpointLabel.ToString()), FColor::Cyan);
}

/** 진행 삭제 후 이미 영역 안에 있는 플레이어는 한 번 나갔다 들어와야 다시 활성화되도록 상태를 설정한다. */
void AAetherPrototypeCheckpoint::ResetCheckpointAfterProgressClear()
{
	bHasActivated = false;
	bRequirePlayerExitBeforeReactivation = false;
	SetTriggerActive(true);

	if (TriggerVolume)
	{
		TriggerVolume->UpdateOverlaps();
		if (AAetherfallCharacter* PlayerCharacter = Cast<AAetherfallCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			bRequirePlayerExitBeforeReactivation = TriggerVolume->IsOverlappingActor(PlayerCharacter);
		}
	}

	ShowCheckpointMessage(FString::Printf(TEXT("Checkpoint reset (%s)"), *CheckpointLabel.ToString()), FColor::Cyan);
}

/** 플레이어와 재활성화 조건을 확인하고 게임 모드에 진행 등급 판정을 위임한 뒤 트리거와 연출 상태를 갱신한다. */
void AAetherPrototypeCheckpoint::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bActivateOnce && bHasActivated)
	{
		return;
	}

	if (!Cast<AAetherfallCharacter>(OtherActor))
	{
		return;
	}

	if (bRequirePlayerExitBeforeReactivation)
	{
		return;
	}

	bool bCheckpointAccepted = true;
	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		bCheckpointAccepted = GameMode->ActivatePrototypeCheckpoint(GetActorTransform(), CheckpointLabel, CheckpointProgressRank);
	}

	bHasActivated = true;

	if (bCheckpointAccepted)
	{
		ShowCheckpointMessage(FString::Printf(TEXT("Checkpoint activated (%s)"), *CheckpointLabel.ToString()), FColor::Green);
		OnCheckpointActivated();
	}
	else
	{
		ShowCheckpointMessage(FString::Printf(TEXT("Checkpoint ignored (%s)"), *CheckpointLabel.ToString()), FColor::Silver);
	}

	if (bDisableAfterActivation)
	{
		SetTriggerActive(false);
	}
}

void AAetherPrototypeCheckpoint::HandleTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (Cast<AAetherfallCharacter>(OtherActor))
	{
		bRequirePlayerExitBeforeReactivation = false;
	}
}

void AAetherPrototypeCheckpoint::SetTriggerActive(bool bNewActive)
{
	if (!TriggerVolume)
	{
		return;
	}

	TriggerVolume->SetCollisionEnabled(bNewActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	TriggerVolume->SetHiddenInGame(true);
}

void AAetherPrototypeCheckpoint::ShowCheckpointMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherCheckpoint] %s"), *Message);

	if (bShowCheckpointDebugMessages && !bRouteCheckpointMessagesToHudOnly && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
	}
}
