#include "AetherGameModeBase.h"

#include "AetherAudioSettingsLibrary.h"
#include "AetherCinematicDirectorSubsystem.h"
#include "AetherDialogueComponent.h"
#include "AetherHUD.h"
#include "AetherEnemyBase.h"
#include "AetherCombatComponent.h"
#include "AetherHealthComponent.h"
#include "AetherInventoryComponent.h"
#include "AetherLockOnComponent.h"
#include "AetherLoadingScreenSubsystem.h"
#include "AetherPlayerController.h"
#include "AetherPrototypeCheckpoint.h"
#include "AetherPrototypeCheckpointWorldRestorer.h"
#include "AetherPrototypeDamageTarget.h"
#include "AetherPrototypeEnemySpawnPolicy.h"
#include "AetherPrototypeCheckpointSnapshot.h"
#include "AetherPrototypeEncounterConfigPolicy.h"
#include "AetherPrototypeLabels.h"
#include "AetherPrototypeRoundPolicy.h"
#include "AetherPrototypeRoutePacingPresenter.h"
#include "AetherPrototypeSaveGame.h"
#include "AetherPrototypeSaveSchemaPolicy.h"
#include "AetherSaveSubsystem.h"
#include "AetherfallCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

namespace
{
	FName BuildPrototypeDialogueEventLabel(const TCHAR* EventPrefix, FName EventLabel)
	{
		return EventLabel.IsNone()
			? NAME_None
			: FName(*FString::Printf(TEXT("%s_%s"), EventPrefix, *EventLabel.ToString()));
	}
}

AAetherGameModeBase::AAetherGameModeBase()
{
	DefaultPawnClass = AAetherfallCharacter::StaticClass();
	PlayerControllerClass = AAetherPlayerController::StaticClass();
	HUDClass = AAetherHUD::StaticClass();
	PrototypeDamageTargetClass = AAetherPrototypeDamageTarget::StaticClass();
	PrototypeEnemyClass = AAetherEnemyBase::StaticClass();
	DialogueComponent = CreateDefaultSubobject<UAetherDialogueComponent>(TEXT("DialogueComponent"));
	PrototypeEnemyArchetypeSequence = { EAetherEnemyArchetype::Skirmisher, EAetherEnemyArchetype::Brute };
}

void AAetherGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	LoadPrototypeCheckpointSnapshot();
}

void AAetherGameModeBase::StartPrototypeCombatRound()
{
	ActivePrototypeEncounterLabel = NAME_None;
	ResetPrototypeCombatRound();
	ShowPrototypeMessage(TEXT("Combat round started"), FColor::Cyan);
}

void AAetherGameModeBase::StartPrototypeCombatRoundForEncounter(FName EncounterLabel)
{
	if (EncounterLabel == GetPrototypeBossEncounterLabel() && HasCompletedPrototypeEncounter(EncounterLabel))
	{
		UE_LOG(LogTemp, Log, TEXT("[AetherBoss] Aurel spawn skipped / defeated persisted"));
		ShowPrototypeMessage(TEXT("Boss encounter skipped (Aurel defeated persisted)"), FColor::Silver);
		return;
	}

	if (EncounterLabel == GetPrototypeBossEncounterLabel() && !bBypassNextBossIntroCinematic)
	{
		if (TryRequestPrototypeCinematic(EAetherCinematicTrigger::BossIntro, EncounterLabel))
		{
			PendingPrototypeEncounterAfterCinematic = EncounterLabel;
			ShowPrototypeMessage(TEXT("Boss encounter waiting for intro cinematic"), FColor::Cyan);
			return;
		}
	}
	bBypassNextBossIntroCinematic = false;

	ActivePrototypeEncounterLabel = EncounterLabel;
	ResetPrototypeCombatRound();
	ShowPrototypeMessage(
		FString::Printf(TEXT("Combat round started (%s)"), *EncounterLabel.ToString()),
		FColor::Cyan);
	TryStartPrototypeDialogueForLabeledEvent(TEXT("EncounterStart"), EncounterLabel);
}

void AAetherGameModeBase::ApplyPrototypeEncounterConfig(const FAetherPrototypeEncounterConfig& EncounterConfig)
{
	const FAetherPrototypeEncounterConfigApplyResult ApplyResult =
		FAetherPrototypeEncounterConfigPolicy::BuildRuntimeConfig(EncounterConfig, BuildActivePrototypeEncounterRuntimeConfig());
	CommitPrototypeEncounterRuntimeConfig(ApplyResult.RuntimeConfig);

	OnPrototypeRoundProgressChanged.Broadcast(PrototypeRoundDefeatCount, PrototypeRoundKillGoal);
	ShowPrototypeMessage(ApplyResult.SummaryMessage, FColor::Silver);
}

void AAetherGameModeBase::ResetPrototypeCombatRound()
{
	GetWorldTimerManager().ClearTimer(PrototypeEnemyRespawnTimerHandle);
	PrototypeEnemyAttackSlots.Reset();
	PrototypeRoundDefeatCount = 0;
	bPrototypeCombatRoundComplete = false;
	bPrototypeRoundGoalMetCuePlayed = false;
	PrototypeRoundClearBannerExpireTime = -1.0;
	PrototypeLevelCompleteBannerExpireTime = -1.0;
	OnPrototypeRoundProgressChanged.Broadcast(PrototypeRoundDefeatCount, PrototypeRoundKillGoal);
	OnPrototypeCombatRoundReset.Broadcast();
	OnPrototypeEncounterReset.Broadcast(ActivePrototypeEncounterLabel);

	if (bClearPrototypeDamageTargetOnCombatRoundReset)
	{
		DestroyPrototypeDamageTarget();
	}

	DestroyPrototypeEnemies();
	if (ActivePrototypeEncounterLabel == GetPrototypeBossEncounterLabel() && !HasCompletedPrototypeEncounter(ActivePrototypeEncounterLabel))
	{
		const int32 RemovedAurelCount = DestroyPrototypeEnemiesByArchetype(EAetherEnemyArchetype::Aurel);
		if (RemovedAurelCount > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[AetherBoss] Aurel stale instance removed / count %d"), RemovedAurelCount);
		}
	}

	if (AAetherfallCharacter* PlayerCharacter = Cast<AAetherfallCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UAetherCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
		{
			CombatComponent->ResetPlayerPrototypeState();
		}
	}

	SpawnPrototypeEnemy();
	if (bAutoSpawnPrototypeDamageTarget)
	{
		SpawnPrototypeDamageTarget();
	}
	ShowPrototypeMessage(TEXT("Combat round reset"), FColor::Cyan);
}

bool AAetherGameModeBase::ActivatePrototypeCheckpoint(const FTransform& CheckpointTransform, FName CheckpointLabel, int32 CheckpointProgressRank)
{
	const int32 ResolvedCheckpointProgressRank = FAetherPrototypeCheckpointSnapshot::ResolveProgressRank(CheckpointLabel, CheckpointProgressRank);
	if (bPreventPrototypeCheckpointDowngrade &&
		bHasActivePrototypeCheckpoint &&
		ResolvedCheckpointProgressRank < ActivePrototypeCheckpointProgressRank)
	{
		ShowPrototypeMessage(
			FString::Printf(
				TEXT("Checkpoint activation ignored / lower rank %s %d < %s %d"),
				*CheckpointLabel.ToString(),
				ResolvedCheckpointProgressRank,
				*ActivePrototypeCheckpointLabel.ToString(),
				ActivePrototypeCheckpointProgressRank),
			FColor::Silver);
		RegisterPrototypeCheckpointFeedback(
			FString::Printf(TEXT("CHECKPOINT KEPT - %s"), *ActivePrototypeCheckpointLabel.ToString().ToUpper()),
			FLinearColor(0.92f, 0.78f, 0.30f, 1.0f));
		return false;
	}

	bHasActivePrototypeCheckpoint = true;
	ActivePrototypeCheckpointTransform = CheckpointTransform;
	ActivePrototypeCheckpointLabel = CheckpointLabel;
	ActivePrototypeCheckpointProgressRank = ResolvedCheckpointProgressRank;
	OnPrototypeCheckpointActivated.Broadcast(CheckpointLabel);
	ShowPrototypeMessage(
		FString::Printf(TEXT("Checkpoint activated (%s / rank %d)"), *CheckpointLabel.ToString(), ActivePrototypeCheckpointProgressRank),
		FColor::Green);
	TryStartPrototypeDialogueForLabeledEvent(TEXT("Checkpoint"), CheckpointLabel);
	SavePrototypeCheckpointSnapshot();
	return true;
}

void AAetherGameModeBase::ResetPrototypePlayerAtCheckpoint()
{
	GetWorldTimerManager().ClearTimer(PrototypeDefeatRetryTimerHandle);
	PrototypeCheckpointRetry.ClearScheduledRetry();

	AAetherfallCharacter* PlayerCharacter = Cast<AAetherfallCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!PlayerCharacter)
	{
		return;
	}

	const FName RuntimeEncounterLabelBeforeRestore = ActivePrototypeEncounterLabel;
	const bool bRuntimeBossEncounterWasActive = IsPrototypeBossEncounterActive();
	const bool bHasPreparedRetrySnapshot = PreparePrototypeCheckpointRetryRestore();
	const bool bRuntimeBossEncounterDefeatedAfterRestore =
		bRuntimeBossEncounterWasActive &&
		HasCompletedPrototypeEncounter(RuntimeEncounterLabelBeforeRestore);
	FAetherPrototypeCheckpointRetryResetInput RetryResetInput;
	RetryResetInput.bRuntimeBossEncounterWasActive = bRuntimeBossEncounterWasActive;
	RetryResetInput.bRuntimeBossEncounterDefeatedAfterRestore = bRuntimeBossEncounterDefeatedAfterRestore;
	RetryResetInput.bRestartActiveEncounterOnCheckpointReset = bRestartActiveEncounterOnCheckpointReset;
	RetryResetInput.bHasActiveCheckpoint = bHasActivePrototypeCheckpoint;
	RetryResetInput.bHasPreparedRetrySnapshot = bHasPreparedRetrySnapshot;
	RetryResetInput.bPrototypeCombatRoundCompleteAfterRestore = bPrototypeCombatRoundComplete;
	RetryResetInput.RuntimeEncounterLabelBeforeRestore = RuntimeEncounterLabelBeforeRestore;
	RetryResetInput.ActiveEncounterLabelAfterRestore = ActivePrototypeEncounterLabel;
	RetryResetInput.ActiveCheckpointLabel = ActivePrototypeCheckpointLabel;
	const FAetherPrototypeCheckpointRetryResetPlan RetryResetPlan = PrototypeCheckpointRetry.BuildResetPlan(RetryResetInput);

	if (UAetherCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
	{
		CombatComponent->ResetPlayerPrototypeState();
	}

	if (RetryResetPlan.bResetBossEncounterRuntime)
	{
		ResetPrototypeBossEncounterRuntimeForCheckpointRestore(RetryResetPlan.BossEncounterLabelToReset);
	}

	PlayerCharacter->ClearDesiredMovementDirection();
	if (UCharacterMovementComponent* MovementComponent = PlayerCharacter->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	if (RetryResetPlan.bStopAfterPlayerCleanupWithoutCheckpoint)
	{
		ShowPrototypeMessage(RetryResetPlan.NoCheckpointMessage, RetryResetPlan.NoCheckpointColor);
		return;
	}

	const FVector RespawnLocation = ActivePrototypeCheckpointTransform.GetLocation();
	const FRotator RespawnRotation = ActivePrototypeCheckpointTransform.Rotator();
	PlayerCharacter->SetActorLocationAndRotation(RespawnLocation, RespawnRotation, false, nullptr, ETeleportType::TeleportPhysics);

	if (AController* PlayerController = PlayerCharacter->GetController())
	{
		PlayerController->SetControlRotation(RespawnRotation);
	}

	if (RetryResetPlan.bRestartActiveEncounter)
	{
		const FName EncounterLabelToRestart = RetryResetPlan.EncounterLabelToRestart;
		ResetPrototypeCombatRound();
		ShowPrototypeMessage(
			FString::Printf(TEXT("Checkpoint retry restarted encounter (%s)"), *EncounterLabelToRestart.ToString()),
			FColor::Yellow);
	}

	if (RetryResetPlan.bRefreshCheckpointWorldState)
	{
		RefreshPrototypeCheckpointWorldState();
		if (RetryResetPlan.bApplyLoadedPrototypePlayerState)
		{
			ApplyLoadedPrototypePlayerState();
		}
		ShowPrototypeMessage(RetryResetPlan.SnapshotRestoredMessage, RetryResetPlan.SnapshotRestoredColor);
	}

	ShowPrototypeMessage(RetryResetPlan.PlayerResetMessage, RetryResetPlan.PlayerResetColor);
}

void AAetherGameModeBase::SchedulePrototypePlayerRetryAfterDefeat()
{
	const FAetherPrototypeCheckpointRetrySchedulePlan RetrySchedulePlan =
		PrototypeCheckpointRetry.ScheduleAfterDefeat(bHasActivePrototypeCheckpoint, PrototypeDefeatRetryDelay);
	if (!RetrySchedulePlan.bShouldScheduleRetryTimer)
	{
		ShowPrototypeMessage(RetrySchedulePlan.FeedbackMessage, RetrySchedulePlan.FeedbackColor);
		return;
	}

	GetWorldTimerManager().ClearTimer(PrototypeDefeatRetryTimerHandle);
	GetWorldTimerManager().SetTimer(
		PrototypeDefeatRetryTimerHandle,
		this,
		&AAetherGameModeBase::ResetPrototypePlayerAtCheckpoint,
		RetrySchedulePlan.RetryDelaySeconds,
		false);
	ShowPrototypeMessage(RetrySchedulePlan.FeedbackMessage, RetrySchedulePlan.FeedbackColor);
}

bool AAetherGameModeBase::ClearPrototypeCheckpointProgress()
{
	UAetherSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	if (!SaveSubsystem || !SaveSubsystem->ClearPrototypeCheckpointSnapshot())
	{
		ShowPrototypeMessage(TEXT("Prototype checkpoint snapshot clear failed"), FColor::Red);
		RegisterPrototypeCheckpointFeedback(TEXT("CHECKPOINT CLEAR FAILED"), FLinearColor(0.95f, 0.08f, 0.04f, 1.0f));
		return false;
	}

	GetWorldTimerManager().ClearTimer(PrototypeDefeatRetryTimerHandle);
	PrototypeCheckpointRetry.ClearScheduledRetry();
	bHasLoadedPrototypeCheckpointSnapshot = false;
	bDeferPrototypeCheckpointSaveUntilLoadedStateApplied = false;
	bHasActivePrototypeCheckpoint = false;
	ActivePrototypeCheckpointTransform = FTransform::Identity;
	ActivePrototypeCheckpointLabel = NAME_None;
	ActivePrototypeCheckpointProgressRank = 0;
	ActivePrototypeEncounterLabel = NAME_None;
	LastCompletedPrototypeEncounterLabel = NAME_None;
	CompletedPrototypeEncounterLabels.Reset();
	bPrototypeCombatRoundComplete = false;
	PrototypeRoundDefeatCount = 0;
	bPrototypeRoundGoalMetCuePlayed = false;
	PrototypeRoundClearBannerExpireTime = -1.0;
	PrototypeLevelCompleteBannerExpireTime = -1.0;
	bPrototypeLevelComplete = false;
	CompletedPrototypeLevelGoalLabel = NAME_None;
	bCathedralEndingComplete = false;
	LoadedPrototypePlayerCurrentHealth = 100.0f;
	LoadedPrototypeHealingItemCount = 0;
	CollectedPrototypeKeyLabels.Reset();
	CollectedPrototypeRewardLabels.Reset();
	CollectedPrototypeLoreLabels.Reset();
	ActivatedPrototypeLeverLabels.Reset();
	UnlockedPrototypeProgressGateLabels.Reset();
	UnlockedPrototypeKeyGateLabels.Reset();
	OpenedPrototypeChestLabels.Reset();
	if (DialogueComponent)
	{
		DialogueComponent->ResetPlayedDialogueLabels();
	}
	ResetPrototypeRoutePacingTimer();
	OnPrototypeRoundProgressChanged.Broadcast(PrototypeRoundDefeatCount, PrototypeRoundKillGoal);

	RefreshPrototypeCheckpointWorldState();
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AAetherPrototypeCheckpoint> It(World); It; ++It)
		{
			AAetherPrototypeCheckpoint* Checkpoint = *It;
			if (IsValid(Checkpoint))
			{
				Checkpoint->ResetCheckpointAfterProgressClear();
			}
		}
	}

	ShowPrototypeMessage(TEXT("Prototype checkpoint progress cleared"), FColor::Cyan);
	RegisterPrototypeCheckpointFeedback(TEXT("CHECKPOINT PROGRESS CLEARED"), FLinearColor(0.92f, 0.78f, 0.30f, 1.0f));
	return true;
}

float AAetherGameModeBase::GetPrototypeDefeatRetryRemainingTime() const
{
	if (!PrototypeCheckpointRetry.IsRetryScheduled() || !GetWorld())
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, GetWorldTimerManager().GetTimerRemaining(PrototypeDefeatRetryTimerHandle));
}

bool AAetherGameModeBase::HasPrototypeCheckpointFeedback() const
{
	return !PrototypeCheckpointFeedbackLabel.IsEmpty() && GetPrototypeCheckpointFeedbackRemainingTime() > 0.0f;
}

float AAetherGameModeBase::GetPrototypeCheckpointFeedbackRemainingTime() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	return FMath::Max(0.0, PrototypeCheckpointFeedbackExpireTime - World->GetTimeSeconds());
}

void AAetherGameModeBase::RegisterPrototypeProgressFeedback(const FString& FeedbackLabel, const FLinearColor& FeedbackColor)
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	PrototypeProgressFeedbackLabel = FeedbackLabel;
	PrototypeProgressFeedbackColor = FeedbackColor;
	PrototypeProgressFeedbackExpireTime = World->GetTimeSeconds() + FMath::Max(0.1f, PrototypeProgressFeedbackDuration);
}

bool AAetherGameModeBase::HasPrototypeProgressFeedback() const
{
	return !PrototypeProgressFeedbackLabel.IsEmpty() && GetPrototypeProgressFeedbackRemainingTime() > 0.0f;
}

float AAetherGameModeBase::GetPrototypeProgressFeedbackRemainingTime() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	return FMath::Max(0.0, PrototypeProgressFeedbackExpireTime - World->GetTimeSeconds());
}

bool AAetherGameModeBase::HasCompletedPrototypeEncounter(FName EncounterLabel) const
{
	return !EncounterLabel.IsNone() && CompletedPrototypeEncounterLabels.Contains(EncounterLabel);
}

bool AAetherGameModeBase::IsPrototypeBossEncounterActive() const
{
	return ActivePrototypeEncounterLabel == GetPrototypeBossEncounterLabel() && !bPrototypeCombatRoundComplete;
}

bool AAetherGameModeBase::HasCompletedPrototypeBossEncounter() const
{
	return HasCompletedPrototypeEncounter(GetPrototypeBossEncounterLabel());
}

FName AAetherGameModeBase::GetPrototypeBossEncounterLabel() const
{
	return AetherPrototypeLabels::CathedralBoss();
}

FName AAetherGameModeBase::GetPrototypeCathedralEndingGoalLabel() const
{
	return AetherPrototypeLabels::CathedralEnding();
}

float AAetherGameModeBase::GetPrototypeRouteElapsedSeconds() const
{
	if (PrototypeRouteCompletedElapsedSeconds >= 0.0)
	{
		return static_cast<float>(PrototypeRouteCompletedElapsedSeconds);
	}

	const UWorld* World = GetWorld();
	if (!World || PrototypeRouteStartTime < 0.0)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, static_cast<float>(World->GetTimeSeconds() - PrototypeRouteStartTime));
}

FString AAetherGameModeBase::GetPrototypeRoutePacingStatusLabel() const
{
	return FAetherPrototypeRoutePacingPresenter::BuildStatusLabel(
		GetPrototypeRouteElapsedSeconds(),
		PrototypeRouteTargetMinSeconds,
		PrototypeRouteTargetMaxSeconds,
		PrototypeRouteCompletedElapsedSeconds >= 0.0);
}

bool AAetherGameModeBase::TryStartPrototypeDialogue(FName DialogueTriggerLabel, bool bForceReplay)
{
	return DialogueComponent && DialogueComponent->TryStartDialogue(DialogueTriggerLabel, bForceReplay);
}

void AAetherGameModeBase::AdvancePrototypeDialogue()
{
	if (DialogueComponent)
	{
		DialogueComponent->SkipOrAdvanceDialogue();
	}
}

bool AAetherGameModeBase::IsPrototypeDialogueActive() const
{
	return DialogueComponent && DialogueComponent->IsDialogueActive();
}

bool AAetherGameModeBase::ShouldPrototypeDialogueBlockGameplayInput() const
{
	return DialogueComponent && DialogueComponent->ShouldBlockGameplayInput();
}

FText AAetherGameModeBase::GetPrototypeDialogueSpeakerName() const
{
	return DialogueComponent ? DialogueComponent->GetCurrentSpeakerName() : FText::GetEmpty();
}

FText AAetherGameModeBase::GetPrototypeDialogueText() const
{
	return DialogueComponent ? DialogueComponent->GetCurrentDialogueText() : FText::GetEmpty();
}

FText AAetherGameModeBase::GetPrototypeDialogueObjectiveHint() const
{
	return DialogueComponent ? DialogueComponent->GetCurrentObjectiveHint() : FText::GetEmpty();
}

bool AAetherGameModeBase::HasPlayedPrototypeDialogueLabel(FName DialogueLabel) const
{
	return DialogueComponent && DialogueComponent->HasPlayedDialogueLabel(DialogueLabel);
}

bool AAetherGameModeBase::ShouldShowPrototypeRoundClearBanner() const
{
	return GetPrototypeRoundClearBannerRemainingTime() > 0.0f;
}

float AAetherGameModeBase::GetPrototypeRoundClearBannerRemainingTime() const
{
	const UWorld* World = GetWorld();
	if (!World || !bPrototypeCombatRoundComplete || PrototypeRoundClearBannerExpireTime <= 0.0)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, static_cast<float>(PrototypeRoundClearBannerExpireTime - World->GetTimeSeconds()));
}

bool AAetherGameModeBase::ShouldShowPrototypeLevelCompleteBanner() const
{
	return GetPrototypeLevelCompleteBannerRemainingTime() > 0.0f;
}

float AAetherGameModeBase::GetPrototypeLevelCompleteBannerRemainingTime() const
{
	const UWorld* World = GetWorld();
	if (!World || !bPrototypeLevelComplete || PrototypeLevelCompleteBannerExpireTime <= 0.0)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, static_cast<float>(PrototypeLevelCompleteBannerExpireTime - World->GetTimeSeconds()));
}

void AAetherGameModeBase::CompletePrototypeLevel(FName GoalLabel)
{
	const bool bIsCathedralEndingGoal = GoalLabel == GetPrototypeCathedralEndingGoalLabel();
	if (bPrototypeLevelComplete)
	{
		const bool bCanUpgradeLegacyCompletionToCathedralEnding =
			bIsCathedralEndingGoal &&
			CompletedPrototypeLevelGoalLabel != GoalLabel;
		if (!bCanUpgradeLegacyCompletionToCathedralEnding)
		{
			return;
		}
	}

	bPrototypeLevelComplete = true;
	if (UWorld* World = GetWorld())
	{
		PrototypeLevelCompleteBannerExpireTime = World->GetTimeSeconds() + FMath::Max(0.1f, PrototypeLevelCompleteBannerDuration);
	}
	if (PrototypeRouteCompletedElapsedSeconds < 0.0)
	{
		PrototypeRouteCompletedElapsedSeconds = GetPrototypeRouteElapsedSeconds();
	}
	CompletedPrototypeLevelGoalLabel = GoalLabel;
	if (bIsCathedralEndingGoal)
	{
		bCathedralEndingComplete = true;
		ShowPrototypeMessage(TEXT("Cathedral ending complete / rift sealed"), FColor::Cyan);
	}
	OnPrototypeLevelCompleted.Broadcast(GoalLabel);
	LogPrototypeRoutePacingComplete(GoalLabel);
	ShowPrototypeMessage(
		FString::Printf(TEXT("Prototype level complete (%s)"), *GoalLabel.ToString()),
		FColor::Green);
	TryStartPrototypeDialogueForLabeledEvent(TEXT("LevelComplete"), GoalLabel);

	SavePrototypeCheckpointSnapshot();
}

void AAetherGameModeBase::CollectPrototypeKey(FName KeyLabel)
{
	if (KeyLabel.IsNone() || CollectedPrototypeKeyLabels.Contains(KeyLabel))
	{
		return;
	}

	CollectedPrototypeKeyLabels.Add(KeyLabel);
	OnPrototypeKeyCollected.Broadcast(KeyLabel);
	ShowPrototypeMessage(
		FString::Printf(TEXT("Prototype key collected (%s)"), *KeyLabel.ToString()),
		FColor::Yellow);
	TryStartPrototypeDialogueForLabeledEvent(TEXT("KeyCollected"), KeyLabel);
}

bool AAetherGameModeBase::HasCollectedPrototypeKey(FName KeyLabel) const
{
	return !KeyLabel.IsNone() && CollectedPrototypeKeyLabels.Contains(KeyLabel);
}

void AAetherGameModeBase::CollectPrototypeReward(FName RewardLabel)
{
	if (RewardLabel.IsNone() || CollectedPrototypeRewardLabels.Contains(RewardLabel))
	{
		return;
	}

	CollectedPrototypeRewardLabels.Add(RewardLabel);
	OnPrototypeRewardCollected.Broadcast(RewardLabel);
	ShowPrototypeMessage(
		FString::Printf(TEXT("Prototype reward collected (%s)"), *RewardLabel.ToString()),
		FColor::Orange);
	TryStartPrototypeDialogueForLabeledEvent(TEXT("RewardCollected"), RewardLabel);
}

bool AAetherGameModeBase::HasCollectedPrototypeReward(FName RewardLabel) const
{
	return !RewardLabel.IsNone() && CollectedPrototypeRewardLabels.Contains(RewardLabel);
}

void AAetherGameModeBase::CollectPrototypeLore(FName LoreLabel)
{
	if (LoreLabel.IsNone() || CollectedPrototypeLoreLabels.Contains(LoreLabel))
	{
		return;
	}

	CollectedPrototypeLoreLabels.Add(LoreLabel);
	OnPrototypeLoreCollected.Broadcast(LoreLabel);
	ShowPrototypeMessage(
		FString::Printf(TEXT("Prototype lore collected (%s)"), *LoreLabel.ToString()),
		FColor::Purple);
	TryStartPrototypeDialogueForLabeledEvent(TEXT("LoreCollected"), LoreLabel);
}

bool AAetherGameModeBase::HasCollectedPrototypeLore(FName LoreLabel) const
{
	return !LoreLabel.IsNone() && CollectedPrototypeLoreLabels.Contains(LoreLabel);
}

void AAetherGameModeBase::RecordPrototypeLeverActivated(FName LeverLabel)
{
	if (!LeverLabel.IsNone())
	{
		ActivatedPrototypeLeverLabels.Add(LeverLabel);
		TryStartPrototypeDialogueForLabeledEvent(TEXT("LeverActivated"), LeverLabel);
	}
}

bool AAetherGameModeBase::HasActivatedPrototypeLever(FName LeverLabel) const
{
	return !LeverLabel.IsNone() && ActivatedPrototypeLeverLabels.Contains(LeverLabel);
}

void AAetherGameModeBase::RecordPrototypeProgressGateUnlocked(FName GateLabel)
{
	if (!GateLabel.IsNone())
	{
		UnlockedPrototypeProgressGateLabels.Add(GateLabel);
		TryStartPrototypeDialogueForLabeledEvent(TEXT("ProgressGateUnlocked"), GateLabel);
	}
}

bool AAetherGameModeBase::HasUnlockedPrototypeProgressGate(FName GateLabel) const
{
	return !GateLabel.IsNone() && UnlockedPrototypeProgressGateLabels.Contains(GateLabel);
}

void AAetherGameModeBase::RecordPrototypeKeyGateUnlocked(FName GateLabel)
{
	if (!GateLabel.IsNone())
	{
		UnlockedPrototypeKeyGateLabels.Add(GateLabel);
		TryStartPrototypeDialogueForLabeledEvent(TEXT("KeyGateUnlocked"), GateLabel);
	}
}

bool AAetherGameModeBase::HasUnlockedPrototypeKeyGate(FName GateLabel) const
{
	return !GateLabel.IsNone() && UnlockedPrototypeKeyGateLabels.Contains(GateLabel);
}

void AAetherGameModeBase::RecordPrototypeChestOpened(FName ChestLabel)
{
	if (!ChestLabel.IsNone())
	{
		OpenedPrototypeChestLabels.Add(ChestLabel);
		TryStartPrototypeDialogueForLabeledEvent(TEXT("ChestOpened"), ChestLabel);
	}
}

bool AAetherGameModeBase::HasOpenedPrototypeChest(FName ChestLabel) const
{
	return !ChestLabel.IsNone() && OpenedPrototypeChestLabels.Contains(ChestLabel);
}

bool AAetherGameModeBase::TryAcquirePrototypeEnemyAttackSlot(AAetherEnemyBase* RequestingEnemy)
{
	const UWorld* World = GetWorld();
	const double CurrentTimeSeconds = World ? World->GetTimeSeconds() : 0.0;
	return PrototypeEnemyAttackSlots.TryAcquire(RequestingEnemy, CurrentTimeSeconds, bCoordinatePrototypeEnemyAttacks);
}

void AAetherGameModeBase::ReleasePrototypeEnemyAttackSlot(AAetherEnemyBase* ReleasingEnemy)
{
	PrototypeEnemyAttackSlots.Release(ReleasingEnemy, bCoordinatePrototypeEnemyAttacks);
}

void AAetherGameModeBase::DelayPrototypeEnemyAttacks(float DelayDuration)
{
	if (UWorld* World = GetWorld())
	{
		if (PrototypeEnemyAttackSlots.DelayAttacks(World->GetTimeSeconds(), DelayDuration, bCoordinatePrototypeEnemyAttacks))
		{
			ShowPrototypeMessage(FString::Printf(TEXT("Enemy attack delay after player recovery %.2f sec"), DelayDuration), FColor::Silver);
		}
	}
}

void AAetherGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &AAetherGameModeBase::SignalGameplayWorldReady);
	}

	ResetPrototypeRoutePacingTimer();

	if (!bHasLoadedPrototypeCheckpointSnapshot && TryRequestPrototypeCinematic(EAetherCinematicTrigger::GameIntro, FName(TEXT("Story_OpeningWake"))))
	{
		bPrototypeBeginPlayWaitingForIntroCinematic = true;
		return;
	}

	ContinuePrototypeBeginPlayAfterIntro();
}

void AAetherGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAetherCinematicDirectorSubsystem* CinematicDirector = GameInstance->GetSubsystem<UAetherCinematicDirectorSubsystem>())
		{
			if (PrototypeCinematicFinishedHandle.IsValid())
			{
				CinematicDirector->OnCinematicFinishedNative.Remove(PrototypeCinematicFinishedHandle);
				PrototypeCinematicFinishedHandle.Reset();
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AAetherGameModeBase::ContinuePrototypeBeginPlayAfterIntro()
{
	if (bHasLoadedPrototypeCheckpointSnapshot)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AAetherGameModeBase::ApplyLoadedPrototypePlayerState);
	}
	else
	{
		TryStartPrototypeDialogue(FName(TEXT("Story_OpeningWake")));
	}

	if (bAutoPlayPrototypeCombatMusic && PrototypeCombatMusic)
	{
		UAetherAudioSettingsLibrary::SpawnSound2DForCategory(
			this,
			PrototypeCombatMusic,
			EAetherAudioCategory::Music,
			PrototypeCombatMusicVolume);
		ShowPrototypeMessage(TEXT("Prototype combat music started"), FColor::Silver);
	}

	if (bAutoSpawnPrototypeDamageTarget)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AAetherGameModeBase::SpawnPrototypeDamageTarget);
	}

	if (bAutoSpawnPrototypeEnemy)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AAetherGameModeBase::SpawnPrototypeEnemy);
	}
}

void AAetherGameModeBase::SignalGameplayWorldReady()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAetherLoadingScreenSubsystem* LoadingScreen = GameInstance->GetSubsystem<UAetherLoadingScreenSubsystem>())
		{
			LoadingScreen->NotifyGameplayWorldReady();
		}
	}
}

bool AAetherGameModeBase::TryRequestPrototypeCinematic(EAetherCinematicTrigger Trigger, FName EventLabel)
{
	UGameInstance* GameInstance = GetGameInstance();
	UAetherCinematicDirectorSubsystem* CinematicDirector = GameInstance ? GameInstance->GetSubsystem<UAetherCinematicDirectorSubsystem>() : nullptr;
	if (!CinematicDirector)
	{
		return false;
	}

	if (!PrototypeCinematicFinishedHandle.IsValid())
	{
		PrototypeCinematicFinishedHandle = CinematicDirector->OnCinematicFinishedNative.AddUObject(
			this,
			&AAetherGameModeBase::HandlePrototypeCinematicFinished);
	}

	const bool bRequested = CinematicDirector->RequestCinematicByTrigger(Trigger, EventLabel);
	if (bRequested)
	{
		ShowPrototypeMessage(
			FString::Printf(TEXT("Cinematic requested (%d / %s)"), static_cast<int32>(Trigger), *EventLabel.ToString()),
			FColor::Cyan);
	}
	return bRequested;
}

void AAetherGameModeBase::HandlePrototypeCinematicFinished(const FAetherCinematicRuntimeState& RuntimeState)
{
	if (RuntimeState.Definition.Trigger == EAetherCinematicTrigger::GameIntro && bPrototypeBeginPlayWaitingForIntroCinematic)
	{
		bPrototypeBeginPlayWaitingForIntroCinematic = false;
		ContinuePrototypeBeginPlayAfterIntro();
		return;
	}

	if (RuntimeState.Definition.Trigger == EAetherCinematicTrigger::BossIntro && !PendingPrototypeEncounterAfterCinematic.IsNone())
	{
		const FName EncounterLabel = PendingPrototypeEncounterAfterCinematic;
		PendingPrototypeEncounterAfterCinematic = NAME_None;
		bBypassNextBossIntroCinematic = true;
		StartPrototypeCombatRoundForEncounter(EncounterLabel);
		return;
	}
}

void AAetherGameModeBase::ResetPrototypeRoutePacingTimer()
{
	const UWorld* World = GetWorld();
	PrototypeRouteStartTime = World ? World->GetTimeSeconds() : 0.0;
	PrototypeRouteCompletedElapsedSeconds = -1.0;
}

void AAetherGameModeBase::LogPrototypeRoutePacingComplete(FName GoalLabel) const
{
	const float ElapsedSeconds = GetPrototypeRouteElapsedSeconds();
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[AetherGameMode] Prototype route pacing complete / goal %s / elapsed %s (%.1f sec) / target %s-%s / result %s"),
		*GoalLabel.ToString(),
		*FAetherPrototypeRoutePacingPresenter::FormatTime(ElapsedSeconds),
		ElapsedSeconds,
		*FAetherPrototypeRoutePacingPresenter::FormatTime(PrototypeRouteTargetMinSeconds),
		*FAetherPrototypeRoutePacingPresenter::FormatTime(PrototypeRouteTargetMaxSeconds),
		*FAetherPrototypeRoutePacingPresenter::BuildResultLabel(ElapsedSeconds, PrototypeRouteTargetMinSeconds, PrototypeRouteTargetMaxSeconds));
}

FAetherPrototypeCheckpointSnapshotState AAetherGameModeBase::BuildPrototypeCheckpointSnapshotState(const AAetherfallCharacter* PlayerCharacter) const
{
	FAetherPrototypeCheckpointSnapshotState SnapshotState;
	SnapshotState.bHasActiveCheckpoint = bHasActivePrototypeCheckpoint;
	SnapshotState.ActiveCheckpointTransform = ActivePrototypeCheckpointTransform;
	SnapshotState.ActiveCheckpointLabel = ActivePrototypeCheckpointLabel;
	SnapshotState.ActiveCheckpointProgressRank = ActivePrototypeCheckpointProgressRank;
	SnapshotState.PlayerCurrentHealth = PlayerCharacter && PlayerCharacter->GetHealthComponent() ? PlayerCharacter->GetHealthComponent()->GetCurrentHealth() : LoadedPrototypePlayerCurrentHealth;
	SnapshotState.PrototypeHealingItemCount = PlayerCharacter && PlayerCharacter->GetInventoryComponent() ? PlayerCharacter->GetInventoryComponent()->GetPrototypeHealingItemCount() : LoadedPrototypeHealingItemCount;
	SnapshotState.ActivePrototypeEncounterLabel = ActivePrototypeEncounterLabel;
	SnapshotState.LastCompletedPrototypeEncounterLabel = LastCompletedPrototypeEncounterLabel;
	SnapshotState.CompletedPrototypeEncounterLabels = CompletedPrototypeEncounterLabels;
	SnapshotState.bPrototypeCombatRoundComplete = bPrototypeCombatRoundComplete;
	SnapshotState.PrototypeRoundDefeatCount = PrototypeRoundDefeatCount;
	SnapshotState.PrototypeRoundKillGoal = PrototypeRoundKillGoal;
	SnapshotState.bPrototypeRoundGoalMetCuePlayed = bPrototypeRoundGoalMetCuePlayed;
	SnapshotState.bPrototypeLevelComplete = bPrototypeLevelComplete;
	SnapshotState.CompletedPrototypeLevelGoalLabel = CompletedPrototypeLevelGoalLabel;
	SnapshotState.bCathedralEndingComplete = bCathedralEndingComplete;
	SnapshotState.CollectedPrototypeKeyLabels = CollectedPrototypeKeyLabels;
	SnapshotState.CollectedPrototypeRewardLabels = CollectedPrototypeRewardLabels;
	SnapshotState.CollectedPrototypeLoreLabels = CollectedPrototypeLoreLabels;
	SnapshotState.ActivatedPrototypeLeverLabels = ActivatedPrototypeLeverLabels;
	SnapshotState.UnlockedPrototypeProgressGateLabels = UnlockedPrototypeProgressGateLabels;
	SnapshotState.UnlockedPrototypeKeyGateLabels = UnlockedPrototypeKeyGateLabels;
	SnapshotState.OpenedPrototypeChestLabels = OpenedPrototypeChestLabels;
	if (DialogueComponent)
	{
		SnapshotState.PlayedPrototypeDialogueLabels = DialogueComponent->GetPlayedDialogueLabels();
	}
	return SnapshotState;
}

void AAetherGameModeBase::ApplyPrototypeCheckpointSnapshotState(const FAetherPrototypeCheckpointSnapshotState& SnapshotState)
{
	bHasActivePrototypeCheckpoint = SnapshotState.bHasActiveCheckpoint;
	ActivePrototypeCheckpointTransform = SnapshotState.ActiveCheckpointTransform;
	ActivePrototypeCheckpointLabel = SnapshotState.ActiveCheckpointLabel;
	ActivePrototypeCheckpointProgressRank = SnapshotState.ActiveCheckpointProgressRank;
	LoadedPrototypePlayerCurrentHealth = SnapshotState.PlayerCurrentHealth;
	LoadedPrototypeHealingItemCount = SnapshotState.PrototypeHealingItemCount;
	ActivePrototypeEncounterLabel = SnapshotState.ActivePrototypeEncounterLabel;
	LastCompletedPrototypeEncounterLabel = SnapshotState.LastCompletedPrototypeEncounterLabel;
	CompletedPrototypeEncounterLabels = SnapshotState.CompletedPrototypeEncounterLabels;
	bPrototypeCombatRoundComplete = SnapshotState.bPrototypeCombatRoundComplete;
	PrototypeRoundDefeatCount = SnapshotState.PrototypeRoundDefeatCount;
	PrototypeRoundKillGoal = SnapshotState.PrototypeRoundKillGoal;
	bPrototypeRoundGoalMetCuePlayed = SnapshotState.bPrototypeRoundGoalMetCuePlayed;
	PrototypeRoundClearBannerExpireTime = -1.0;
	PrototypeLevelCompleteBannerExpireTime = -1.0;
	bPrototypeLevelComplete = SnapshotState.bPrototypeLevelComplete;
	CompletedPrototypeLevelGoalLabel = SnapshotState.CompletedPrototypeLevelGoalLabel;
	bCathedralEndingComplete = SnapshotState.bCathedralEndingComplete;
	CollectedPrototypeKeyLabels = SnapshotState.CollectedPrototypeKeyLabels;
	CollectedPrototypeRewardLabels = SnapshotState.CollectedPrototypeRewardLabels;
	CollectedPrototypeLoreLabels = SnapshotState.CollectedPrototypeLoreLabels;
	ActivatedPrototypeLeverLabels = SnapshotState.ActivatedPrototypeLeverLabels;
	UnlockedPrototypeProgressGateLabels = SnapshotState.UnlockedPrototypeProgressGateLabels;
	UnlockedPrototypeKeyGateLabels = SnapshotState.UnlockedPrototypeKeyGateLabels;
	OpenedPrototypeChestLabels = SnapshotState.OpenedPrototypeChestLabels;
	if (DialogueComponent)
	{
		DialogueComponent->SetPlayedDialogueLabels(SnapshotState.PlayedPrototypeDialogueLabels);
	}
}

void AAetherGameModeBase::LogPrototypeCheckpointSnapshotQAState(const FString& Context) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherGameMode] %s"), *FAetherPrototypeCheckpointSnapshot::BuildQASummary(Context, BuildPrototypeCheckpointSnapshotState(nullptr)));
}

void AAetherGameModeBase::LoadPrototypeCheckpointSnapshot()
{
	UAetherSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	if (!SaveSubsystem || !SaveSubsystem->HasPrototypeCheckpointSnapshot())
	{
		return;
	}

	UAetherPrototypeSaveGame* SaveGameObject = SaveSubsystem->LoadPrototypeCheckpointSnapshot();
	if (!SaveGameObject)
	{
		return;
	}

	if (!PopulatePrototypeCheckpointStateFromSaveGame(SaveGameObject))
	{
		return;
	}

	bHasLoadedPrototypeCheckpointSnapshot = true;
	bDeferPrototypeCheckpointSaveUntilLoadedStateApplied = true;

	ShowPrototypeMessage(
		FString::Printf(TEXT("Prototype checkpoint snapshot loaded (%s)"), *ActivePrototypeCheckpointLabel.ToString()),
		FColor::Cyan);
	LogPrototypeCheckpointSnapshotQAState(TEXT("loaded"));
}

void AAetherGameModeBase::SavePrototypeCheckpointSnapshot()
{
	if (!bHasActivePrototypeCheckpoint)
	{
		return;
	}

	if (bDeferPrototypeCheckpointSaveUntilLoadedStateApplied)
	{
		ShowPrototypeMessage(
			FString::Printf(
				TEXT("Prototype checkpoint save deferred during snapshot restore (%s)"),
				*ActivePrototypeCheckpointLabel.ToString()),
			FColor::Cyan);
		return;
	}

	UAetherSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	AAetherfallCharacter* PlayerCharacter = Cast<AAetherfallCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!SaveSubsystem || !PlayerCharacter)
	{
		return;
	}

	UAetherPrototypeSaveGame* SaveGameObject = Cast<UAetherPrototypeSaveGame>(UGameplayStatics::CreateSaveGameObject(UAetherPrototypeSaveGame::StaticClass()));
	if (!SaveGameObject)
	{
		return;
	}

	FAetherPrototypeCheckpointSnapshot::WriteSaveGame(*SaveGameObject, BuildPrototypeCheckpointSnapshotState(PlayerCharacter));

	if (SaveSubsystem->SavePrototypeCheckpointSnapshot(SaveGameObject))
	{
		ShowPrototypeMessage(
			FString::Printf(TEXT("Prototype checkpoint snapshot saved (%s)"), *ActivePrototypeCheckpointLabel.ToString()),
			FColor::Cyan);
		RegisterPrototypeCheckpointFeedback(
			FString::Printf(TEXT("CHECKPOINT SAVED - %s"), *ActivePrototypeCheckpointLabel.ToString().ToUpper()),
			FLinearColor(0.42f, 0.86f, 1.0f, 1.0f));
		LogPrototypeCheckpointSnapshotQAState(TEXT("saved"));
	}
}

void AAetherGameModeBase::ApplyLoadedPrototypePlayerState()
{
	AAetherfallCharacter* PlayerCharacter = Cast<AAetherfallCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!PlayerCharacter)
	{
		return;
	}

	if (bHasActivePrototypeCheckpoint)
	{
		const FVector RespawnLocation = ActivePrototypeCheckpointTransform.GetLocation();
		const FRotator RespawnRotation = ActivePrototypeCheckpointTransform.Rotator();
		PlayerCharacter->SetActorLocationAndRotation(RespawnLocation, RespawnRotation, false, nullptr, ETeleportType::TeleportPhysics);

		if (AController* PlayerController = PlayerCharacter->GetController())
		{
			PlayerController->SetControlRotation(RespawnRotation);
		}
	}

	if (UAetherHealthComponent* HealthComponent = PlayerCharacter->GetHealthComponent())
	{
		HealthComponent->SetCurrentHealth(LoadedPrototypePlayerCurrentHealth);
	}

	if (UAetherInventoryComponent* InventoryComponent = PlayerCharacter->GetInventoryComponent())
	{
		InventoryComponent->SetPrototypeHealingItemCount(LoadedPrototypeHealingItemCount);
	}

	bDeferPrototypeCheckpointSaveUntilLoadedStateApplied = false;

	ShowPrototypeMessage(
		FString::Printf(TEXT("Prototype checkpoint snapshot applied (%s)"), *ActivePrototypeCheckpointLabel.ToString()),
		FColor::Cyan);
	RegisterPrototypeCheckpointFeedback(
		FString::Printf(TEXT("CHECKPOINT RESTORED - %s"), *ActivePrototypeCheckpointLabel.ToString().ToUpper()),
		FLinearColor(0.36f, 0.95f, 0.78f, 1.0f));
	LogPrototypeCheckpointSnapshotQAState(TEXT("applied"));
}

bool AAetherGameModeBase::PopulatePrototypeCheckpointStateFromSaveGame(const UAetherPrototypeSaveGame* SaveGameObject)
{
	if (!SaveGameObject)
	{
		return false;
	}

	const FAetherPrototypeSaveSchemaLoadPlan LoadPlan = FAetherPrototypeSaveSchemaPolicy::BuildLoadPlan(*SaveGameObject);
	if (!LoadPlan.bCanLoad)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AetherGameMode] %s"), *LoadPlan.SummaryMessage);
		ShowPrototypeMessage(LoadPlan.SummaryMessage, FColor::Red);
		return false;
	}

	if (LoadPlan.bNeedsMigration)
	{
		UE_LOG(LogTemp, Log, TEXT("[AetherGameMode] %s"), *LoadPlan.SummaryMessage);
	}

	ApplyPrototypeCheckpointSnapshotState(FAetherPrototypeCheckpointSnapshot::ReadSaveGame(*SaveGameObject, GetPrototypeCathedralEndingGoalLabel()));
	return true;
}

bool AAetherGameModeBase::PreparePrototypeCheckpointRetryRestore()
{
	UAetherSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	if (!SaveSubsystem || !SaveSubsystem->HasPrototypeCheckpointSnapshot())
	{
		return false;
	}

	UAetherPrototypeSaveGame* SaveGameObject = SaveSubsystem->LoadPrototypeCheckpointSnapshot();
	if (!PopulatePrototypeCheckpointStateFromSaveGame(SaveGameObject))
	{
		return false;
	}

	bHasLoadedPrototypeCheckpointSnapshot = true;
	bDeferPrototypeCheckpointSaveUntilLoadedStateApplied = true;
	LogPrototypeCheckpointSnapshotQAState(TEXT("retry prepared"));
	return true;
}

void AAetherGameModeBase::RefreshPrototypeCheckpointWorldState()
{
	FAetherPrototypeCheckpointWorldRestorer::RestoreWorldState(this, BuildPrototypeCheckpointSnapshotState(nullptr));
}

void AAetherGameModeBase::RegisterPrototypeCheckpointFeedback(const FString& FeedbackLabel, const FLinearColor& FeedbackColor)
{
	PrototypeCheckpointFeedbackLabel = FeedbackLabel.ToUpper();
	PrototypeCheckpointFeedbackColor = FeedbackColor;
	constexpr float MinimumReadableFeedbackDuration = 2.75f;
	const float FeedbackDuration = FMath::Max(MinimumReadableFeedbackDuration, PrototypeCheckpointFeedbackDuration);

	if (const UWorld* World = GetWorld())
	{
		PrototypeCheckpointFeedbackExpireTime = World->GetTimeSeconds() + FeedbackDuration;
	}
	else
	{
		PrototypeCheckpointFeedbackExpireTime = FeedbackDuration;
	}
}

void AAetherGameModeBase::SpawnPrototypeDamageTarget()
{
	UWorld* World = GetWorld();
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!World || !PlayerPawn || !PrototypeDamageTargetClass)
	{
		return;
	}

	DestroyPrototypeDamageTarget();

	const FVector Forward = PlayerPawn->GetActorForwardVector();
	const FVector SpawnLocation = PlayerPawn->GetActorLocation() + Forward * PrototypeDamageTargetSpawnDistance;
	const FRotator SpawnRotation = (-Forward).Rotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CurrentPrototypeDamageTarget = World->SpawnActor<AAetherPrototypeDamageTarget>(PrototypeDamageTargetClass, SpawnLocation, SpawnRotation, SpawnParameters);
}

void AAetherGameModeBase::DestroyPrototypeDamageTarget()
{
	if (AAetherPrototypeDamageTarget* DamageTarget = CurrentPrototypeDamageTarget.Get())
	{
		DamageTarget->Destroy();
	}

	CurrentPrototypeDamageTarget.Reset();
}

void AAetherGameModeBase::SpawnPrototypeEnemy()
{
	UWorld* World = GetWorld();
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!World || !PlayerPawn || !PrototypeEnemyClass || bPrototypeCombatRoundComplete)
	{
		return;
	}

	CleanPrototypeEnemyReferences();

	const FVector Forward = PlayerPawn->GetActorForwardVector();
	const FVector Right = PlayerPawn->GetActorRightVector();
	const int32 TargetSpawnCount = FMath::Max(1, PrototypeEnemySpawnCount);
	CurrentPrototypeEnemies.Reserve(TargetSpawnCount);
	int32 SlotIndex = CurrentPrototypeEnemies.Num();

	while (GetLivingPrototypeEnemyCount() < TargetSpawnCount)
	{
		FAetherPrototypeEnemySpawnRequest SpawnRequest;
		SpawnRequest.PlayerLocation = PlayerPawn->GetActorLocation();
		SpawnRequest.PlayerForward = Forward;
		SpawnRequest.PlayerRight = Right;
		SpawnRequest.SlotIndex = SlotIndex;
		SpawnRequest.TargetSpawnCount = TargetSpawnCount;
		SpawnRequest.SpawnDistance = PrototypeEnemySpawnDistance;
		SpawnRequest.SpawnRightOffset = PrototypeEnemySpawnRightOffset;
		SpawnRequest.SpawnSpacing = PrototypeEnemySpawnSpacing;

		const FAetherPrototypeEnemySpawnPlan SpawnPlan =
			FAetherPrototypeEnemySpawnPolicy::BuildSpawnPlan(SpawnRequest, PrototypeEnemyArchetypeSequence);
		AAetherEnemyBase* SpawnedEnemy = World->SpawnActorDeferred<AAetherEnemyBase>(
			PrototypeEnemyClass,
			SpawnPlan.SpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (SpawnedEnemy)
		{
			SpawnedEnemy->ApplyEnemyArchetype(SpawnPlan.EnemyArchetype);
			UGameplayStatics::FinishSpawningActor(SpawnedEnemy, SpawnPlan.SpawnTransform);
			CurrentPrototypeEnemies.Add(SpawnedEnemy);
			if (SpawnedEnemy->GetEnemyArchetype() == EAetherEnemyArchetype::Aurel)
			{
				UE_LOG(LogTemp, Log, TEXT("[AetherBoss] Aurel spawned fresh"));
			}
			if (SpawnedEnemy->GetHealthComponent())
			{
				SpawnedEnemy->GetHealthComponent()->OnDeath.AddDynamic(this, &AAetherGameModeBase::HandlePrototypeEnemyDeath);
			}
		}

		++SlotIndex;
		if (SlotIndex > TargetSpawnCount + 8)
		{
			break;
		}
	}
}

void AAetherGameModeBase::DestroyPrototypeEnemies()
{
	PrototypeEnemyAttackSlots.Reset();

	for (TWeakObjectPtr<AAetherEnemyBase>& EnemyPtr : CurrentPrototypeEnemies)
	{
		if (AAetherEnemyBase* Enemy = EnemyPtr.Get())
		{
			Enemy->Destroy();
		}
	}

	CurrentPrototypeEnemies.Reset();
}

void AAetherGameModeBase::ResetPrototypeBossEncounterRuntimeForCheckpointRestore(FName RuntimeEncounterLabel)
{
	if (RuntimeEncounterLabel != GetPrototypeBossEncounterLabel())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(PrototypeEnemyRespawnTimerHandle);
	PrototypeEnemyAttackSlots.Reset();

	if (AAetherfallCharacter* PlayerCharacter = Cast<AAetherfallCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		if (UAetherLockOnComponent* LockOnComponent = PlayerCharacter->GetLockOnComponent())
		{
			LockOnComponent->ClearLockOn();
		}
	}

	const int32 RemovedAurelCount = DestroyPrototypeEnemiesByArchetype(EAetherEnemyArchetype::Aurel);
	UE_LOG(LogTemp, Log, TEXT("[AetherBoss] Aurel encounter reset"));
	if (RemovedAurelCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[AetherBoss] Aurel stale instance removed / count %d"), RemovedAurelCount);
	}
}

int32 AAetherGameModeBase::DestroyPrototypeEnemiesByArchetype(EAetherEnemyArchetype EnemyArchetype)
{
	int32 DestroyedCount = 0;
	UWorld* World = GetWorld();
	if (!World)
	{
		return DestroyedCount;
	}

	for (TActorIterator<AAetherEnemyBase> It(World); It; ++It)
	{
		AAetherEnemyBase* Enemy = *It;
		if (!IsValid(Enemy) || Enemy->GetEnemyArchetype() != EnemyArchetype)
		{
			continue;
		}

		ReleasePrototypeEnemyAttackSlot(Enemy);
		Enemy->Destroy();
		++DestroyedCount;
	}

	CurrentPrototypeEnemies.RemoveAll([EnemyArchetype](const TWeakObjectPtr<AAetherEnemyBase>& EnemyPtr)
	{
		const AAetherEnemyBase* Enemy = EnemyPtr.Get();
		return !Enemy || Enemy->GetEnemyArchetype() == EnemyArchetype;
	});

	return DestroyedCount;
}

void AAetherGameModeBase::CleanPrototypeEnemyReferences()
{
	CurrentPrototypeEnemies.RemoveAll([](const TWeakObjectPtr<AAetherEnemyBase>& EnemyPtr)
	{
		const AAetherEnemyBase* Enemy = EnemyPtr.Get();
		const UAetherHealthComponent* HealthComponent = Enemy ? Enemy->GetHealthComponent() : nullptr;
		return !Enemy || !HealthComponent || HealthComponent->IsDead();
	});
}

int32 AAetherGameModeBase::GetLivingPrototypeEnemyCount() const
{
	int32 LivingCount = 0;
	for (const TWeakObjectPtr<AAetherEnemyBase>& EnemyPtr : CurrentPrototypeEnemies)
	{
		const AAetherEnemyBase* Enemy = EnemyPtr.Get();
		const UAetherHealthComponent* HealthComponent = Enemy ? Enemy->GetHealthComponent() : nullptr;
		if (HealthComponent && !HealthComponent->IsDead())
		{
			++LivingCount;
		}
	}

	return LivingCount;
}

FAetherPrototypeEncounterRuntimeConfig AAetherGameModeBase::BuildActivePrototypeEncounterRuntimeConfig() const
{
	FAetherPrototypeEncounterRuntimeConfig RuntimeConfig;
	RuntimeConfig.EnemySpawnCount = PrototypeEnemySpawnCount;
	RuntimeConfig.RoundKillGoal = PrototypeRoundKillGoal;
	RuntimeConfig.EnemyArchetypeSequence = PrototypeEnemyArchetypeSequence;
	RuntimeConfig.CompletionRewardLabel = ActivePrototypeEncounterCompletionRewardLabel;
	RuntimeConfig.CompletionAetherGaugeAmount = ActivePrototypeEncounterCompletionAetherReward;
	RuntimeConfig.CompletionFeedbackLabel = ActivePrototypeEncounterCompletionFeedbackLabel;
	return RuntimeConfig;
}

void AAetherGameModeBase::CommitPrototypeEncounterRuntimeConfig(const FAetherPrototypeEncounterRuntimeConfig& RuntimeConfig)
{
	PrototypeEnemySpawnCount = RuntimeConfig.EnemySpawnCount;
	PrototypeRoundKillGoal = RuntimeConfig.RoundKillGoal;
	PrototypeEnemyArchetypeSequence = RuntimeConfig.EnemyArchetypeSequence;
	ActivePrototypeEncounterCompletionRewardLabel = RuntimeConfig.CompletionRewardLabel;
	ActivePrototypeEncounterCompletionAetherReward = RuntimeConfig.CompletionAetherGaugeAmount;
	ActivePrototypeEncounterCompletionFeedbackLabel = RuntimeConfig.CompletionFeedbackLabel;
}

void AAetherGameModeBase::SchedulePrototypeEnemyRespawn()
{
	if (!bRespawnPrototypeEnemiesDuringRound || bPrototypeCombatRoundComplete)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(PrototypeEnemyRespawnTimerHandle);
	GetWorldTimerManager().SetTimer(PrototypeEnemyRespawnTimerHandle, this, &AAetherGameModeBase::SpawnPrototypeEnemy, PrototypeEnemyRespawnDelay, false);
	ShowPrototypeMessage(FString::Printf(TEXT("Enemy refill in %.1f sec"), PrototypeEnemyRespawnDelay), FColor::Yellow);
}

void AAetherGameModeBase::HandlePrototypeEnemyDeath(UAetherHealthComponent* DeadHealthComponent, AActor* DamageCauser)
{
	for (const TWeakObjectPtr<AAetherEnemyBase>& EnemyPtr : CurrentPrototypeEnemies)
	{
		AAetherEnemyBase* Enemy = EnemyPtr.Get();
		if (Enemy && Enemy->GetHealthComponent() == DeadHealthComponent)
		{
			ReleasePrototypeEnemyAttackSlot(Enemy);
			break;
		}
	}

	FAetherPrototypeRoundDeathInput RoundDeathInput;
	RoundDeathInput.bUseRoundGoal = bUsePrototypeCombatRoundGoal;
	RoundDeathInput.bRoundComplete = bPrototypeCombatRoundComplete;
	RoundDeathInput.CurrentDefeatCount = PrototypeRoundDefeatCount;
	RoundDeathInput.KillGoal = PrototypeRoundKillGoal;
	RoundDeathInput.LivingEnemiesAfterDeath = GetLivingPrototypeEnemyCount();
	RoundDeathInput.bGoalMetCuePlayed = bPrototypeRoundGoalMetCuePlayed;

	const FAetherPrototypeRoundDeathResult RoundDeathResult = FAetherPrototypeRoundPolicy::EvaluateEnemyDeath(RoundDeathInput);
	PrototypeRoundDefeatCount = RoundDeathResult.NewDefeatCount;
	bPrototypeRoundGoalMetCuePlayed = RoundDeathResult.bGoalMetCuePlayed;

	if (RoundDeathResult.bBroadcastProgress)
	{
		OnPrototypeRoundProgressChanged.Broadcast(PrototypeRoundDefeatCount, PrototypeRoundKillGoal);
	}

	for (const FAetherPrototypeRoundFeedback& Feedback : RoundDeathResult.FeedbackMessages)
	{
		ShowPrototypeMessage(Feedback.Message, Feedback.Color);
	}

	if (RoundDeathResult.bClearRespawnTimer)
	{
		GetWorldTimerManager().ClearTimer(PrototypeEnemyRespawnTimerHandle);
	}

	if (RoundDeathResult.bBroadcastGoalMet)
	{
		OnPrototypeRoundGoalMet.Broadcast();
	}

	if (RoundDeathResult.bPlayGoalMetCue)
	{
		PlayPrototypeRoundCueSound(PrototypeRoundGoalMetSound, TEXT("Goal met"));
	}

	if (RoundDeathResult.bCompleteRound)
	{
		CompletePrototypeCombatRound();
		return;
	}

	if (RoundDeathResult.bScheduleRespawn)
	{
		SchedulePrototypeEnemyRespawn();
	}
}

void AAetherGameModeBase::CompletePrototypeCombatRound()
{
	if (bPrototypeCombatRoundComplete)
	{
		return;
	}

	bPrototypeCombatRoundComplete = true;
	bPrototypeRoundGoalMetCuePlayed = true;
	if (UWorld* World = GetWorld())
	{
		PrototypeRoundClearBannerExpireTime = World->GetTimeSeconds() + FMath::Max(0.1f, PrototypeRoundClearBannerDuration);
	}
	PrototypeEnemyAttackSlots.Reset();
	GetWorldTimerManager().ClearTimer(PrototypeEnemyRespawnTimerHandle);
	PlayPrototypeRoundCueSound(PrototypeRoundClearSound, TEXT("Clear"));
	OnPrototypeRoundGoalMet.Broadcast();
	OnPrototypeRoundCompleted.Broadcast();
	LastCompletedPrototypeEncounterLabel = ActivePrototypeEncounterLabel;
	if (!ActivePrototypeEncounterLabel.IsNone())
	{
		CompletedPrototypeEncounterLabels.Add(ActivePrototypeEncounterLabel);
		if (ActivePrototypeEncounterLabel == GetPrototypeBossEncounterLabel())
		{
			UE_LOG(LogTemp, Log, TEXT("[AetherBoss] Aurel defeated persisted"));
		}
	}
	TryStartPrototypeDialogueForLabeledEvent(TEXT("EncounterComplete"), ActivePrototypeEncounterLabel);
	if (!ActivePrototypeEncounterCompletionRewardLabel.IsNone())
	{
		const bool bRewardAlreadyCollected = HasCollectedPrototypeReward(ActivePrototypeEncounterCompletionRewardLabel);
		CollectPrototypeReward(ActivePrototypeEncounterCompletionRewardLabel);
		if (!bRewardAlreadyCollected && ActivePrototypeEncounterCompletionAetherReward > 0.0f)
		{
			if (AAetherfallCharacter* PlayerCharacter = Cast<AAetherfallCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
			{
				if (UAetherCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
				{
					CombatComponent->GrantPrototypeAetherGauge(
						ActivePrototypeEncounterCompletionAetherReward,
						FString::Printf(TEXT("Encounter %s"), *ActivePrototypeEncounterLabel.ToString()));
				}
			}
		}
		if (!ActivePrototypeEncounterCompletionFeedbackLabel.IsEmpty())
		{
			RegisterPrototypeProgressFeedback(ActivePrototypeEncounterCompletionFeedbackLabel, FLinearColor(0.62f, 0.36f, 1.0f, 1.0f));
		}
		FAetherPrototypeEncounterRuntimeConfig ClearedRewardConfig = BuildActivePrototypeEncounterRuntimeConfig();
		FAetherPrototypeEncounterConfigPolicy::ResetCompletionReward(ClearedRewardConfig);
		CommitPrototypeEncounterRuntimeConfig(ClearedRewardConfig);
	}
	OnPrototypeEncounterCompleted.Broadcast(ActivePrototypeEncounterLabel);
	ShowPrototypeMessage(
		FString::Printf(TEXT("Combat round complete / defeated %d / %d"), PrototypeRoundDefeatCount, PrototypeRoundKillGoal),
		FColor::Green);
	SavePrototypeCheckpointSnapshot();

	if (LastCompletedPrototypeEncounterLabel == GetPrototypeBossEncounterLabel())
	{
		TryRequestPrototypeCinematic(EAetherCinematicTrigger::BossDefeated, LastCompletedPrototypeEncounterLabel);
	}
}

void AAetherGameModeBase::TryStartPrototypeDialogueForLabeledEvent(const TCHAR* EventPrefix, FName EventLabel, bool bForceReplay)
{
	TryStartPrototypeDialogue(BuildPrototypeDialogueEventLabel(EventPrefix, EventLabel), bForceReplay);
}

void AAetherGameModeBase::PlayPrototypeRoundCueSound(USoundBase* CueSound, FName CueName) const
{
	if (!bEnablePrototypeRoundCueSounds || !CueSound)
	{
		return;
	}

	const float VolumeVariance = FMath::Max(0.0f, PrototypeRoundCueSoundVolumeVariance);
	const float Volume = FMath::Max(0.0f, PrototypeRoundCueSoundVolume + FMath::FRandRange(-VolumeVariance, VolumeVariance));
	const float PitchMin = FMath::Max(0.1f, FMath::Min(PrototypeRoundCueSoundPitchMin, PrototypeRoundCueSoundPitchMax));
	const float PitchMax = FMath::Max(PitchMin, PrototypeRoundCueSoundPitchMax);
	const float Pitch = FMath::FRandRange(PitchMin, PitchMax);

	UAetherAudioSettingsLibrary::SpawnSound2DForCategory(this, CueSound, EAetherAudioCategory::Sfx, Volume, Pitch);
	UE_LOG(LogTemp, Log, TEXT("[AetherGameMode] Round cue sound played (%s)"), *CueName.ToString());
}

void AAetherGameModeBase::ShowPrototypeMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherGameMode] %s"), *Message);

	if (bShowPrototypeScreenDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
	}
}
