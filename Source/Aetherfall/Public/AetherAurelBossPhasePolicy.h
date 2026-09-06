#pragma once

#include "CoreMinimal.h"

enum class EAetherEnemyArchetype : uint8;
struct FAetherEnemyAttackPatternData;

/** 페이즈 전환 시 사용할 체력 임계값과 이동·공격 보정값을 묶는다. */
struct FAetherAurelBossPhaseTuning
{
	float PhaseTwoHealthThresholdPercent = 0.5f;
	float PhaseShiftDuration = 1.2f;
	float PhaseShiftAetherReward = 20.0f;
	float PhaseTwoAttackCooldownMultiplier = 0.78f;
	float PhaseTwoMovementSpeed = 255.0f;
	float LowHealthWarningPercent = 0.15f;
};

/** 페이즈 정책이 호출자에게 요청하는 일회성 안내 결과다. */
struct FAetherAurelBossPhaseFeedback
{
	bool bShouldAnnounce = false;
	FString DebugMessage;
	FString ProgressMessage;
	FLinearColor ProgressColor = FLinearColor::White;
};

/** 페이즈 전환을 실행하는 적 액터에 전달할 변경 계획이다. */
struct FAetherAurelBossPhaseTransitionPlan
{
	bool bShouldStartPhaseTwo = false;
	float PhaseShiftDuration = 0.0f;
	float AetherReward = 0.0f;
	float AttackCooldownMultiplier = 1.0f;
	float PhaseTwoMovementSpeed = 0.0f;
	FAetherAurelBossPhaseFeedback Feedback;
};

/** Aurel의 페이즈 전환 조건, 일회성 안내와 페이즈별 순차 공격 인덱스를 관리한다. 실제 이동·보상 적용은 호출자가 담당한다. */
class AETHERFALL_API FAetherAurelBossPhasePolicy
{
public:
	/** 재도전 시 안내 여부와 두 페이즈의 공격 순서를 초기화한다. */
	void Reset();

	bool IsPhaseOne(EAetherEnemyArchetype EnemyArchetype) const;
	bool IsPhaseTwo(EAetherEnemyArchetype EnemyArchetype) const;
	bool IsPhaseShifting() const;

	FAetherAurelBossPhaseFeedback EvaluatePhaseOneIntro(EAetherEnemyArchetype EnemyArchetype);
	/** 살아 있는 Aurel이 체력 임계값에 도달했는지 판단하고 전환에 적용할 수치를 반환한다. 이 함수 자체는 페이즈를 변경하지 않는다. */
	FAetherAurelBossPhaseTransitionPlan EvaluatePhaseTwoTransition(
		EAetherEnemyArchetype EnemyArchetype,
		float CurrentHealth,
		float MaxHealth,
		const FAetherAurelBossPhaseTuning& Tuning) const;
	/** 2페이즈의 낮은 체력 안내를 한 번만 반환하도록 안내 상태를 갱신한다. */
	FAetherAurelBossPhaseFeedback EvaluateLowHealth(
		EAetherEnemyArchetype EnemyArchetype,
		float CurrentHealth,
		float MaxHealth,
		const FAetherAurelBossPhaseTuning& Tuning);

	/** 2페이즈를 활성화하고 전환 연출 중 상태와 새 공격 순서를 시작한다. */
	void BeginPhaseTwoTransition();
	void EndPhaseShift();

	/** 활성 페이즈의 순차 인덱스를 사용해 사거리 안의 공격을 선택한다. 선택 정책의 대체 공격 규칙도 그대로 적용한다. */
	const FAetherEnemyAttackPatternData* SelectAurelAttackPattern(
		const TArray<FAetherEnemyAttackPatternData>& AttackPatterns,
		float DistanceToTarget,
		float FallbackAttackRange,
		float AttackStartRangeBuffer,
		const FAetherEnemyAttackPatternData& FallbackAttackPattern);

private:
	bool bPhaseOneIntroAnnounced = false;
	bool bPhaseTwoActive = false;
	bool bPhaseShifting = false;
	bool bLowHealthWarningAnnounced = false;
	int32 NextPhaseOneAttackIndex = 0;
	int32 NextPhaseTwoAttackIndex = 0;
};
