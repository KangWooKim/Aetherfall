#pragma once

#include "CoreMinimal.h"

struct FAetherEnemyAttackPatternData;

/** 거리 조건과 선택 가중치를 만족하는 적 공격을 가중 무작위 또는 순차 방식으로 선택한다. */
class AETHERFALL_API FAetherEnemyAttackPatternPolicy
{
public:
	/** 시작 거리와 양수 가중치를 통과한 후보만 추첨한다. 패턴 목록 자체가 비어 있을 때만 기본 공격을 사용한다. */
	static const FAetherEnemyAttackPatternData* SelectWeightedPattern(
		const TArray<FAetherEnemyAttackPatternData>& AttackPatterns,
		float DistanceToTarget,
		float FallbackAttackRange,
		float AttackStartRangeBuffer,
		const FAetherEnemyAttackPatternData& FallbackAttackPattern);

	/** 다음 인덱스부터 순환 검색하여 가능한 패턴을 선택하고, 성공한 경우에만 다음 인덱스를 갱신한다. */
	static const FAetherEnemyAttackPatternData* SelectSequentialPattern(
		const TArray<FAetherEnemyAttackPatternData>& AttackPatterns,
		float DistanceToTarget,
		float FallbackAttackRange,
		float AttackStartRangeBuffer,
		const FAetherEnemyAttackPatternData& FallbackAttackPattern,
		int32& NextAttackPatternIndex);

	/** 공격 범위에서 시작 여유 거리를 뺀 범위 안에 있고 선택 가중치가 양수인 패턴만 허용한다. */
	static bool CanStartAttackPattern(
		const FAetherEnemyAttackPatternData& AttackPattern,
		float DistanceToTarget,
		float AttackStartRangeBuffer);
};
