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
