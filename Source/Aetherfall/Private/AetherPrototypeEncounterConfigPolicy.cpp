/** 구간 설정의 덮어쓰기 플래그와 범위 제한을 적용해 게임 모드가 사용할 실행 설정을 계산한다. */
#include "AetherPrototypeEncounterConfigPolicy.h"

/** 현재값을 출발점으로 선택된 항목만 덮어쓰고, 보상 비활성 구간에서는 이전 완료 보상 정보를 비운다. */
FAetherPrototypeEncounterConfigApplyResult FAetherPrototypeEncounterConfigPolicy::BuildRuntimeConfig(
	const FAetherPrototypeEncounterConfig& EncounterConfig,
	const FAetherPrototypeEncounterRuntimeConfig& CurrentRuntimeConfig)
{
	FAetherPrototypeEncounterConfigApplyResult Result;
	Result.RuntimeConfig = CurrentRuntimeConfig;
	Result.RuntimeConfig.EnemySpawnCount = ResolveEnemySpawnCount(
		EncounterConfig.bOverrideEnemySpawnCount,
		EncounterConfig.EnemySpawnCount,
		CurrentRuntimeConfig.EnemySpawnCount);
	Result.RuntimeConfig.RoundKillGoal = ResolveRoundKillGoal(
		EncounterConfig.bOverrideRoundKillGoal,
		EncounterConfig.RoundKillGoal,
		CurrentRuntimeConfig.RoundKillGoal);
	Result.RuntimeConfig.EnemyArchetypeSequence = ResolveEnemyArchetypeSequence(
		EncounterConfig.bOverrideEnemyArchetypeSequence,
		EncounterConfig.EnemyArchetypeSequence,
		CurrentRuntimeConfig.EnemyArchetypeSequence);

	if (EncounterConfig.bGrantCompletionReward)
	{
		Result.RuntimeConfig.CompletionRewardLabel = EncounterConfig.CompletionRewardLabel;
		Result.RuntimeConfig.CompletionAetherGaugeAmount = FMath::Max(0.0f, EncounterConfig.CompletionAetherGaugeAmount);
		Result.RuntimeConfig.CompletionFeedbackLabel = EncounterConfig.CompletionFeedbackLabel;
	}
	else
	{
		ResetCompletionReward(Result.RuntimeConfig);
	}

	Result.SummaryMessage = BuildSummaryMessage(Result.RuntimeConfig);
	return Result;
}

int32 FAetherPrototypeEncounterConfigPolicy::ResolveEnemySpawnCount(
	bool bOverrideEnemySpawnCount,
	int32 OverrideEnemySpawnCount,
	int32 CurrentEnemySpawnCount)
{
	return bOverrideEnemySpawnCount ? FMath::Clamp(OverrideEnemySpawnCount, 1, 4) : CurrentEnemySpawnCount;
}

int32 FAetherPrototypeEncounterConfigPolicy::ResolveRoundKillGoal(
	bool bOverrideRoundKillGoal,
	int32 OverrideRoundKillGoal,
	int32 CurrentRoundKillGoal)
{
	return bOverrideRoundKillGoal ? FMath::Max(1, OverrideRoundKillGoal) : CurrentRoundKillGoal;
}

/** 덮어쓰기가 켜져 있고 목록이 비어 있지 않을 때만 새 적 원형 순서를 채택한다. */
TArray<EAetherEnemyArchetype> FAetherPrototypeEncounterConfigPolicy::ResolveEnemyArchetypeSequence(
	bool bOverrideEnemyArchetypeSequence,
	const TArray<EAetherEnemyArchetype>& OverrideEnemyArchetypeSequence,
	const TArray<EAetherEnemyArchetype>& CurrentEnemyArchetypeSequence)
{
	if (bOverrideEnemyArchetypeSequence && OverrideEnemyArchetypeSequence.Num() > 0)
	{
		return OverrideEnemyArchetypeSequence;
	}

	return CurrentEnemyArchetypeSequence;
}

void FAetherPrototypeEncounterConfigPolicy::ResetCompletionReward(FAetherPrototypeEncounterRuntimeConfig& RuntimeConfig)
{
	RuntimeConfig.CompletionRewardLabel = NAME_None;
	RuntimeConfig.CompletionAetherGaugeAmount = 0.0f;
	RuntimeConfig.CompletionFeedbackLabel.Empty();
}

FString FAetherPrototypeEncounterConfigPolicy::BuildSummaryMessage(const FAetherPrototypeEncounterRuntimeConfig& RuntimeConfig)
{
	return FString::Printf(TEXT("Encounter config applied / active %d / goal %d / archetypes %d / reward %s"),
		RuntimeConfig.EnemySpawnCount,
		RuntimeConfig.RoundKillGoal,
		RuntimeConfig.EnemyArchetypeSequence.Num(),
		*RuntimeConfig.CompletionRewardLabel.ToString());
}
