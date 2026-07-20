#include "AetherEnemyAttackPatternPolicy.h"

#include "AetherEnemyBase.h"

namespace
{
const FAetherEnemyAttackPatternData* SelectFallbackPattern(
	float DistanceToTarget,
	float FallbackAttackRange,
	const FAetherEnemyAttackPatternData& FallbackAttackPattern)
{
	return DistanceToTarget <= FallbackAttackRange ? &FallbackAttackPattern : nullptr;
}
}

const FAetherEnemyAttackPatternData* FAetherEnemyAttackPatternPolicy::SelectWeightedPattern(
	const TArray<FAetherEnemyAttackPatternData>& AttackPatterns,
	float DistanceToTarget,
	float FallbackAttackRange,
	float AttackStartRangeBuffer,
	const FAetherEnemyAttackPatternData& FallbackAttackPattern)
{
	if (AttackPatterns.Num() <= 0)
	{
		return SelectFallbackPattern(DistanceToTarget, FallbackAttackRange, FallbackAttackPattern);
	}

	TArray<const FAetherEnemyAttackPatternData*> CandidatePatterns;
	float TotalWeight = 0.0f;
	for (const FAetherEnemyAttackPatternData& AttackPattern : AttackPatterns)
	{
		if (!CanStartAttackPattern(AttackPattern, DistanceToTarget, AttackStartRangeBuffer))
		{
			continue;
		}

		CandidatePatterns.Add(&AttackPattern);
		TotalWeight += AttackPattern.SelectionWeight;
	}

	if (CandidatePatterns.Num() <= 0 || TotalWeight <= 0.0f)
	{
		return nullptr;
	}

	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	for (const FAetherEnemyAttackPatternData* CandidatePattern : CandidatePatterns)
	{
		Roll -= CandidatePattern->SelectionWeight;
		if (Roll <= 0.0f)
		{
			return CandidatePattern;
		}
	}

	return CandidatePatterns.Last();
}

const FAetherEnemyAttackPatternData* FAetherEnemyAttackPatternPolicy::SelectSequentialPattern(
	const TArray<FAetherEnemyAttackPatternData>& AttackPatterns,
	float DistanceToTarget,
	float FallbackAttackRange,
	float AttackStartRangeBuffer,
	const FAetherEnemyAttackPatternData& FallbackAttackPattern,
	int32& NextAttackPatternIndex)
{
	if (AttackPatterns.Num() <= 0)
	{
		return SelectFallbackPattern(DistanceToTarget, FallbackAttackRange, FallbackAttackPattern);
	}

	const int32 PatternCount = AttackPatterns.Num();
	const int32 StartIndex = FMath::Clamp(NextAttackPatternIndex, 0, PatternCount - 1);
	for (int32 Offset = 0; Offset < PatternCount; ++Offset)
	{
		const int32 PatternIndex = (StartIndex + Offset) % PatternCount;
		const FAetherEnemyAttackPatternData& AttackPattern = AttackPatterns[PatternIndex];
		if (!CanStartAttackPattern(AttackPattern, DistanceToTarget, AttackStartRangeBuffer))
		{
			continue;
		}

		NextAttackPatternIndex = (PatternIndex + 1) % PatternCount;
		return &AttackPattern;
	}

	return nullptr;
}

bool FAetherEnemyAttackPatternPolicy::CanStartAttackPattern(
	const FAetherEnemyAttackPatternData& AttackPattern,
	float DistanceToTarget,
	float AttackStartRangeBuffer)
{
	const float RequiredStartRange = FMath::Max(0.0f, AttackPattern.Range - AttackStartRangeBuffer);
	return DistanceToTarget <= RequiredStartRange && AttackPattern.SelectionWeight > 0.0f;
}
