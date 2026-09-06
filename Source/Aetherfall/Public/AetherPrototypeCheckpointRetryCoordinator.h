#pragma once

#include "CoreMinimal.h"

/** 패배 후 자동 재시도 여부와 대기 시간을 결정한 결과다. */
struct FAetherPrototypeCheckpointRetrySchedulePlan
{
	bool bShouldScheduleRetryTimer = false;
	float RetryDelaySeconds = 0.0f;
	FString FeedbackMessage;
	FColor FeedbackColor = FColor::White;
};

/** 재시도 시 유지할 진행과 다시 시작할 전투를 판단하는 입력이다. */
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

/** 적 정리·플레이어 복원·전투 재시작의 실행 여부를 호출자에게 전달한다. */
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

/** 패배 후 재시도 예약 여부와 보스 정리·구간 재시작·저장 복원 절차를 실행 계획으로 계산한다. */
class AETHERFALL_API FAetherPrototypeCheckpointRetryCoordinator
{
public:
	bool IsRetryScheduled() const { return bDefeatRetryScheduled; }
	void ClearScheduledRetry();
	/** 체크포인트가 있으면 예약 플래그와 지연 시간을 반환한다. 실제 기존 타이머 교체는 게임 모드가 담당한다. */
	FAetherPrototypeCheckpointRetrySchedulePlan ScheduleAfterDefeat(bool bHasActiveCheckpoint, float RetryDelaySeconds);
	/** 복원 전 보스 상태와 복원 후 처치 이력을 비교하여 보스 정리 및 구간 재시작 여부를 결정한다. */
	FAetherPrototypeCheckpointRetryResetPlan BuildResetPlan(const FAetherPrototypeCheckpointRetryResetInput& Input) const;

private:
	bool bDefeatRetryScheduled = false;
};
