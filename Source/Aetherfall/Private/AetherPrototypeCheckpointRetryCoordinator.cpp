#include "AetherPrototypeCheckpointRetryCoordinator.h"

void FAetherPrototypeCheckpointRetryCoordinator::ClearScheduledRetry()
{
	bDefeatRetryScheduled = false;
}

FAetherPrototypeCheckpointRetrySchedulePlan FAetherPrototypeCheckpointRetryCoordinator::ScheduleAfterDefeat(
	bool bHasActiveCheckpoint,
	float RetryDelaySeconds)
{
	FAetherPrototypeCheckpointRetrySchedulePlan Plan;
	if (!bHasActiveCheckpoint)
	{
		bDefeatRetryScheduled = false;
		Plan.FeedbackMessage = TEXT("Player defeated without checkpoint / manual reset required");
		Plan.FeedbackColor = FColor::Red;
		return Plan;
	}

	bDefeatRetryScheduled = true;
	Plan.bShouldScheduleRetryTimer = true;
	Plan.RetryDelaySeconds = FMath::Max(0.0f, RetryDelaySeconds);
	Plan.FeedbackMessage = FString::Printf(TEXT("Checkpoint retry scheduled after defeat %.2f sec"), RetryDelaySeconds);
	Plan.FeedbackColor = FColor::Red;
	return Plan;
}

FAetherPrototypeCheckpointRetryResetPlan FAetherPrototypeCheckpointRetryCoordinator::BuildResetPlan(
	const FAetherPrototypeCheckpointRetryResetInput& Input) const
{
	FAetherPrototypeCheckpointRetryResetPlan Plan;

	Plan.bResetBossEncounterRuntime =
		Input.bRuntimeBossEncounterWasActive &&
		!Input.bRuntimeBossEncounterDefeatedAfterRestore;
	Plan.BossEncounterLabelToReset = Plan.bResetBossEncounterRuntime
		? Input.RuntimeEncounterLabelBeforeRestore
		: NAME_None;

	if (!Input.bHasActiveCheckpoint)
	{
		Plan.bStopAfterPlayerCleanupWithoutCheckpoint = true;
		Plan.NoCheckpointMessage = TEXT("Player reset without checkpoint");
		Plan.NoCheckpointColor = FColor::Green;
		return Plan;
	}

	const FName EffectiveActiveEncounterLabel = Plan.bResetBossEncounterRuntime
		? Input.RuntimeEncounterLabelBeforeRestore
		: Input.ActiveEncounterLabelAfterRestore;
	const bool bEffectiveRoundComplete = Plan.bResetBossEncounterRuntime
		? false
		: Input.bPrototypeCombatRoundCompleteAfterRestore;
	Plan.bRestartActiveEncounter =
		Input.bRestartActiveEncounterOnCheckpointReset &&
		!EffectiveActiveEncounterLabel.IsNone() &&
		!bEffectiveRoundComplete;
	Plan.EncounterLabelToRestart = Plan.bRestartActiveEncounter ? EffectiveActiveEncounterLabel : NAME_None;

	if (Input.bHasPreparedRetrySnapshot)
	{
		Plan.bRefreshCheckpointWorldState = true;
		Plan.bApplyLoadedPrototypePlayerState = true;
		Plan.SnapshotRestoredMessage = FString::Printf(
			TEXT("Prototype checkpoint retry snapshot restored (%s)"),
			*Input.ActiveCheckpointLabel.ToString());
		Plan.SnapshotRestoredColor = FColor::Cyan;
	}

	Plan.PlayerResetMessage = FString::Printf(
		TEXT("Player reset at checkpoint (%s)"),
		*Input.ActiveCheckpointLabel.ToString());
	Plan.PlayerResetColor = FColor::Green;
	return Plan;
}
