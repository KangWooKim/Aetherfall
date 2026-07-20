#include "AetherPrototypeEncounterConfigPolicy.h"

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
