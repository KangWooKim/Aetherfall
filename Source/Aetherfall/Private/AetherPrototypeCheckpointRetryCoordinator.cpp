/** 패배 후 재시도 예약 여부와 보스 정리·구간 재시작·저장 복원 절차를 실행 계획으로 계산한다. */
#include "AetherPrototypeCheckpointRetryCoordinator.h"

void FAetherPrototypeCheckpointRetryCoordinator::ClearScheduledRetry()
{
	bDefeatRetryScheduled = false;
}

/** 체크포인트가 있으면 예약 플래그와 지연 시간을 반환한다. 실제 기존 타이머 교체는 게임 모드가 담당한다. */
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

/** 복원 전 보스 상태와 복원 후 처치 이력을 비교하여 보스 정리 및 구간 재시작 여부를 결정한다. */
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
