#pragma once

#include "CoreMinimal.h"
#include "AetherPrototypeEncounterConfig.h"

/** 현재 전투에 적용할 적 구성과 처치 목표를 묶는다. */
struct FAetherPrototypeEncounterRuntimeConfig
{
	int32 EnemySpawnCount = 2;
	int32 RoundKillGoal = 6;
	TArray<EAetherEnemyArchetype> EnemyArchetypeSequence;
	FName CompletionRewardLabel = NAME_None;
	float CompletionAetherGaugeAmount = 0.0f;
	FString CompletionFeedbackLabel;
};

/** 전투 설정 적용 후 호출자가 사용할 선택 결과다. */
struct FAetherPrototypeEncounterConfigApplyResult
{
	FAetherPrototypeEncounterRuntimeConfig RuntimeConfig;
	FString SummaryMessage;
};

/** 구간 설정의 덮어쓰기 플래그와 범위 제한을 적용해 게임 모드가 사용할 실행 설정을 계산한다. */
class AETHERFALL_API FAetherPrototypeEncounterConfigPolicy
{
public:
	/** 현재값을 출발점으로 선택된 항목만 덮어쓰고, 보상 비활성 구간에서는 이전 완료 보상 정보를 비운다. */
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

	/** 덮어쓰기가 켜져 있고 목록이 비어 있지 않을 때만 새 적 원형 순서를 채택한다. */
	static TArray<EAetherEnemyArchetype> ResolveEnemyArchetypeSequence(
		bool bOverrideEnemyArchetypeSequence,
		const TArray<EAetherEnemyArchetype>& OverrideEnemyArchetypeSequence,
		const TArray<EAetherEnemyArchetype>& CurrentEnemyArchetypeSequence);

	static void ResetCompletionReward(FAetherPrototypeEncounterRuntimeConfig& RuntimeConfig);
	static FString BuildSummaryMessage(const FAetherPrototypeEncounterRuntimeConfig& RuntimeConfig);
};
