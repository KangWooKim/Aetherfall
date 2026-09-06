/** Aurel의 페이즈 전환 조건, 일회성 안내와 페이즈별 순차 공격 인덱스를 관리한다. 실제 이동·보상 적용은 호출자가 담당한다. */
#include "AetherAurelBossPhasePolicy.h"

#include "AetherEnemyAttackPatternPolicy.h"
#include "AetherEnemyBase.h"

namespace
{
bool IsAurel(EAetherEnemyArchetype EnemyArchetype)
{
	return EnemyArchetype == EAetherEnemyArchetype::Aurel;
}
}

/** 재도전 시 안내 여부와 두 페이즈의 공격 순서를 초기화한다. */
void FAetherAurelBossPhasePolicy::Reset()
{
	bPhaseOneIntroAnnounced = false;
	bPhaseTwoActive = false;
	bPhaseShifting = false;
	bLowHealthWarningAnnounced = false;
	NextPhaseOneAttackIndex = 0;
	NextPhaseTwoAttackIndex = 0;
}

bool FAetherAurelBossPhasePolicy::IsPhaseOne(EAetherEnemyArchetype EnemyArchetype) const
{
	return IsAurel(EnemyArchetype) && !bPhaseTwoActive;
}

bool FAetherAurelBossPhasePolicy::IsPhaseTwo(EAetherEnemyArchetype EnemyArchetype) const
{
	return IsAurel(EnemyArchetype) && bPhaseTwoActive;
}

bool FAetherAurelBossPhasePolicy::IsPhaseShifting() const
{
	return bPhaseShifting;
}

FAetherAurelBossPhaseFeedback FAetherAurelBossPhasePolicy::EvaluatePhaseOneIntro(EAetherEnemyArchetype EnemyArchetype)
{
	FAetherAurelBossPhaseFeedback Feedback;
	if (!IsAurel(EnemyArchetype) || bPhaseOneIntroAnnounced)
	{
		return Feedback;
	}

	bPhaseOneIntroAnnounced = true;
	Feedback.bShouldAnnounce = true;
	Feedback.DebugMessage = TEXT("Aurel phase I begins / read the oath rhythm");
	Feedback.ProgressMessage = TEXT("AUREL PHASE I - READ THE OATH RHYTHM");
	Feedback.ProgressColor = FLinearColor(0.18f, 0.76f, 1.0f, 1.0f);
	return Feedback;
}

/** 살아 있는 Aurel이 체력 임계값에 도달했는지 판단하고 전환에 적용할 수치를 반환한다. 이 함수 자체는 페이즈를 변경하지 않는다. */
FAetherAurelBossPhaseTransitionPlan FAetherAurelBossPhasePolicy::EvaluatePhaseTwoTransition(
	EAetherEnemyArchetype EnemyArchetype,
	float CurrentHealth,
	float MaxHealth,
	const FAetherAurelBossPhaseTuning& Tuning) const
{
	FAetherAurelBossPhaseTransitionPlan Plan;
	if (!IsAurel(EnemyArchetype) || bPhaseTwoActive || CurrentHealth <= 0.0f || MaxHealth <= 0.0f)
	{
		return Plan;
	}

	const float HealthPercent = CurrentHealth / MaxHealth;
	if (HealthPercent > Tuning.PhaseTwoHealthThresholdPercent)
	{
		return Plan;
	}

	Plan.bShouldStartPhaseTwo = true;
	Plan.PhaseShiftDuration = FMath::Max(0.05f, Tuning.PhaseShiftDuration);
	Plan.AetherReward = FMath::Max(0.0f, Tuning.PhaseShiftAetherReward);
	Plan.AttackCooldownMultiplier = FMath::Max(0.1f, Tuning.PhaseTwoAttackCooldownMultiplier);
	Plan.PhaseTwoMovementSpeed = FMath::Max(0.0f, Tuning.PhaseTwoMovementSpeed);
	Plan.Feedback.bShouldAnnounce = true;
	Plan.Feedback.DebugMessage = TEXT("CS_Boss_AurelPhaseShift / Aurel phase II begins / rift answers");
	Plan.Feedback.ProgressMessage = TEXT("AUREL PHASE II - THE RIFT ANSWERS HIS OATH");
	Plan.Feedback.ProgressColor = FLinearColor(0.18f, 0.76f, 1.0f, 1.0f);
	return Plan;
}

/** 2페이즈의 낮은 체력 안내를 한 번만 반환하도록 안내 상태를 갱신한다. */
FAetherAurelBossPhaseFeedback FAetherAurelBossPhasePolicy::EvaluateLowHealth(
	EAetherEnemyArchetype EnemyArchetype,
	float CurrentHealth,
	float MaxHealth,
	const FAetherAurelBossPhaseTuning& Tuning)
{
	FAetherAurelBossPhaseFeedback Feedback;
	if (!IsAurel(EnemyArchetype) || !bPhaseTwoActive || bLowHealthWarningAnnounced || CurrentHealth <= 0.0f || MaxHealth <= 0.0f)
	{
		return Feedback;
	}

	const float HealthPercent = CurrentHealth / MaxHealth;
	if (HealthPercent > Tuning.LowHealthWarningPercent)
	{
		return Feedback;
	}

	bLowHealthWarningAnnounced = true;
	Feedback.bShouldAnnounce = true;
	Feedback.DebugMessage = TEXT("Aurel low HP / finish without hesitation");
	Feedback.ProgressMessage = TEXT("AUREL LOW HP - END IT WITHOUT HESITATION");
	Feedback.ProgressColor = FLinearColor(1.0f, 0.48f, 0.18f, 1.0f);
	return Feedback;
}

/** 2페이즈를 활성화하고 전환 연출 중 상태와 새 공격 순서를 시작한다. */
void FAetherAurelBossPhasePolicy::BeginPhaseTwoTransition()
{
	bPhaseTwoActive = true;
	bPhaseShifting = true;
	NextPhaseTwoAttackIndex = 0;
}

void FAetherAurelBossPhasePolicy::EndPhaseShift()
{
	bPhaseShifting = false;
}

/** 활성 페이즈의 순차 인덱스를 사용해 사거리 안의 공격을 선택한다. 선택 정책의 대체 공격 규칙도 그대로 적용한다. */
const FAetherEnemyAttackPatternData* FAetherAurelBossPhasePolicy::SelectAurelAttackPattern(
	const TArray<FAetherEnemyAttackPatternData>& AttackPatterns,
	float DistanceToTarget,
	float FallbackAttackRange,
	float AttackStartRangeBuffer,
	const FAetherEnemyAttackPatternData& FallbackAttackPattern)
{
	return FAetherEnemyAttackPatternPolicy::SelectSequentialPattern(
		AttackPatterns,
		DistanceToTarget,
		FallbackAttackRange,
		AttackStartRangeBuffer,
		FallbackAttackPattern,
		bPhaseTwoActive ? NextPhaseTwoAttackIndex : NextPhaseOneAttackIndex);
}
