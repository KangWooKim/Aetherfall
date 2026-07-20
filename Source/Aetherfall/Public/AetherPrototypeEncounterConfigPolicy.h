#pragma once

#include "CoreMinimal.h"
#include "AetherPrototypeEncounterConfig.h"

struct FAetherPrototypeEncounterRuntimeConfig
{
	int32 EnemySpawnCount = 2;
	int32 RoundKillGoal = 6;
	TArray<EAetherEnemyArchetype> EnemyArchetypeSequence;
	FName CompletionRewardLabel = NAME_None;
	float CompletionAetherGaugeAmount = 0.0f;
	FString CompletionFeedbackLabel;
};

struct FAetherPrototypeEncounterConfigApplyResult
{
	FAetherPrototypeEncounterRuntimeConfig RuntimeConfig;
	FString SummaryMessage;
};

class AETHERFALL_API FAetherPrototypeEncounterConfigPolicy
{
public:
	static FAetherPrototypeEncounterConfigApplyResult BuildRuntimeConfig(
		const FAetherPrototypeEncounterConfig& EncounterConfig,
		const FAetherPrototypeEncounterRuntimeConfig& CurrentRuntimeConfig);

	static int32 ResolveEnemySpawnCount(
		bool bOverrideEnemySpawnCount,
		int32 OverrideEnemySpawnCount,
		int32 CurrentEnemySpawnCount);

	static int32 ResolveRoundKillGoal(
		bool bOverrideRoundKillGoal,
		int32 OverrideRoundKillGoal,
		int32 CurrentRoundKillGoal);

	static TArray<EAetherEnemyArchetype> ResolveEnemyArchetypeSequence(
		bool bOverrideEnemyArchetypeSequence,
		const TArray<EAetherEnemyArchetype>& OverrideEnemyArchetypeSequence,
		const TArray<EAetherEnemyArchetype>& CurrentEnemyArchetypeSequence);

	static void ResetCompletionReward(FAetherPrototypeEncounterRuntimeConfig& RuntimeConfig);
	static FString BuildSummaryMessage(const FAetherPrototypeEncounterRuntimeConfig& RuntimeConfig);
};
