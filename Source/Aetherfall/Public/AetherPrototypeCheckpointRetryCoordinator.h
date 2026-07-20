#pragma once

#include "CoreMinimal.h"

struct FAetherPrototypeCheckpointRetrySchedulePlan
{
	bool bShouldScheduleRetryTimer = false;
	float RetryDelaySeconds = 0.0f;
	FString FeedbackMessage;
	FColor FeedbackColor = FColor::White;
};

struct FAetherPrototypeCheckpointRetryResetInput
{
	bool bRuntimeBossEncounterWasActive = false;
	bool bRuntimeBossEncounterDefeatedAfterRestore = false;
	bool bRestartActiveEncounterOnCheckpointReset = false;
	bool bHasActiveCheckpoint = false;
	bool bHasPreparedRetrySnapshot = false;
	bool bPrototypeCombatRoundCompleteAfterRestore = false;
	FName RuntimeEncounterLabelBeforeRestore = NAME_None;
	FName ActiveEncounterLabelAfterRestore = NAME_None;
	FName ActiveCheckpointLabel = NAME_None;
};

struct FAetherPrototypeCheckpointRetryResetPlan
{
	bool bResetBossEncounterRuntime = false;
	FName BossEncounterLabelToReset = NAME_None;
	bool bStopAfterPlayerCleanupWithoutCheckpoint = false;
	FString NoCheckpointMessage;
	FColor NoCheckpointColor = FColor::Green;
	bool bRestartActiveEncounter = false;
	FName EncounterLabelToRestart = NAME_None;
	bool bRefreshCheckpointWorldState = false;
	bool bApplyLoadedPrototypePlayerState = false;
	FString SnapshotRestoredMessage;
	FColor SnapshotRestoredColor = FColor::Cyan;
	FString PlayerResetMessage;
	FColor PlayerResetColor = FColor::Green;
};

class AETHERFALL_API FAetherPrototypeCheckpointRetryCoordinator
{
public:
	bool IsRetryScheduled() const { return bDefeatRetryScheduled; }
	void ClearScheduledRetry();
	FAetherPrototypeCheckpointRetrySchedulePlan ScheduleAfterDefeat(bool bHasActiveCheckpoint, float RetryDelaySeconds);
	FAetherPrototypeCheckpointRetryResetPlan BuildResetPlan(const FAetherPrototypeCheckpointRetryResetInput& Input) const;

private:
	bool bDefeatRetryScheduled = false;
};
