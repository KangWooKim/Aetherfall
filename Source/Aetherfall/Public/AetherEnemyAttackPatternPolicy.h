#pragma once

#include "CoreMinimal.h"

struct FAetherEnemyAttackPatternData;

class AETHERFALL_API FAetherEnemyAttackPatternPolicy
{
public:
	static const FAetherEnemyAttackPatternData* SelectWeightedPattern(
		const TArray<FAetherEnemyAttackPatternData>& AttackPatterns,
		float DistanceToTarget,
		float FallbackAttackRange,
		float AttackStartRangeBuffer,
		const FAetherEnemyAttackPatternData& FallbackAttackPattern);

	static const FAetherEnemyAttackPatternData* SelectSequentialPattern(
		const TArray<FAetherEnemyAttackPatternData>& AttackPatterns,
		float DistanceToTarget,
		float FallbackAttackRange,
		float AttackStartRangeBuffer,
		const FAetherEnemyAttackPatternData& FallbackAttackPattern,
		int32& NextAttackPatternIndex);

	static bool CanStartAttackPattern(
		const FAetherEnemyAttackPatternData& AttackPattern,
		float DistanceToTarget,
		float AttackStartRangeBuffer);
};
