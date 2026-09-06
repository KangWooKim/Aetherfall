/** 세계관 기록의 수집 라벨·제목·본문과 표시 상태를 관리하는 상호작용 액터다. */
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

/** 미수집 기록을 진행 이력에 추가하고 제목 피드백·본문 로그 및 Blueprint 수집 이벤트를 제공한다. */
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

/** 수집 연출을 다시 실행하지 않고 저장된 기록 수집 상태를 표시 계층에 적용한다. */
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
