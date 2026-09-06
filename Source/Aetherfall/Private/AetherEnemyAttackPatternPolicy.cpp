/** 거리 조건과 선택 가중치를 만족하는 적 공격을 가중 무작위 또는 순차 방식으로 선택한다. */
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

/** 시작 거리와 양수 가중치를 통과한 후보만 추첨한다. 패턴 목록 자체가 비어 있을 때만 기본 공격을 사용한다. */
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

/** 다음 인덱스부터 순환 검색하여 가능한 패턴을 선택하고, 성공한 경우에만 다음 인덱스를 갱신한다. */
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

/** 공격 범위에서 시작 여유 거리를 뺀 범위 안에 있고 선택 가중치가 양수인 패턴만 허용한다. */
bool FAetherEnemyAttackPatternPolicy::CanStartAttackPattern(
	const FAetherEnemyAttackPatternData& AttackPattern,
	float DistanceToTarget,
	float AttackStartRangeBuffer)
{
	const float RequiredStartRange = FMath::Max(0.0f, AttackPattern.Range - AttackStartRangeBuffer);
	return DistanceToTarget <= RequiredStartRange && AttackPattern.SelectionWeight > 0.0f;
}
