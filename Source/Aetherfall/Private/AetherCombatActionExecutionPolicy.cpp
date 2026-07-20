#include "AetherCombatActionExecutionPolicy.h"

float FAetherCombatActionExecutionPolicy::ClampDuration(float Duration)
{
	return FMath::Max(Duration, 0.1f);
}

float FAetherCombatActionExecutionPolicy::ClampImpactDelay(float ImpactDelay, float Duration)
{
	return FMath::Clamp(FMath::Max(ImpactDelay, 0.0f), 0.0f, ClampDuration(Duration));
}

FAetherCombatActionExecutionPlan FAetherCombatActionExecutionPolicy::BuildLightAttackPlan(float Duration, bool bUseImpactNotify)
{
	FAetherCombatActionExecutionPlan Plan;
	Plan.Duration = ClampDuration(Duration);
	Plan.bWaitForImpactNotify = bUseImpactNotify;
	Plan.bApplyImpactImmediately = !Plan.bWaitForImpactNotify;
	return Plan;
}

FAetherCombatActionExecutionPlan FAetherCombatActionExecutionPolicy::BuildHeavyAttackPlan(
	float Duration,
	float ImpactDelay,
	bool bUseImpactNotify)
{
	FAetherCombatActionExecutionPlan Plan;
	Plan.Duration = ClampDuration(Duration);
	Plan.ImpactDelay = ClampImpactDelay(ImpactDelay, Plan.Duration);
	Plan.bWaitForImpactNotify = bUseImpactNotify;
	Plan.bScheduleImpactTimer = !Plan.bWaitForImpactNotify;
	return Plan;
}

FAetherCombatActionExecutionPlan FAetherCombatActionExecutionPolicy::BuildAetherSlashPlan(
	float Duration,
	float ImpactDelay,
	bool bUseImpactNotify)
{
	FAetherCombatActionExecutionPlan Plan = BuildHeavyAttackPlan(Duration, ImpactDelay, bUseImpactNotify);
	return Plan;
}

FAetherCombatActionExecutionPlan FAetherCombatActionExecutionPolicy::BuildAutoParryCounterPlan(
	float Duration,
	float ImpactDelay,
	bool bUseImpactNotify)
{
	FAetherCombatActionExecutionPlan Plan = BuildHeavyAttackPlan(Duration, ImpactDelay, bUseImpactNotify);
	return Plan;
}

FAetherCombatActionExecutionPlan FAetherCombatActionExecutionPolicy::BuildExecutionPlan(
	float ConfiguredDuration,
	float SelectedMontageLength,
	bool bUseImpactNotify,
	float ImpactFallbackNormalizedTime)
{
	FAetherCombatActionExecutionPlan Plan;
	const float FallbackDuration = ClampDuration(ConfiguredDuration);
	const float MontageDuration = FMath::Max(SelectedMontageLength, 0.0f);
	Plan.Duration = MontageDuration > KINDA_SMALL_NUMBER ? FMath::Max(FallbackDuration, MontageDuration) : FallbackDuration;
	Plan.ImpactDelay = ClampImpactDelay(Plan.Duration * FMath::Clamp(ImpactFallbackNormalizedTime, 0.0f, 1.0f), Plan.Duration);
	Plan.bWaitForImpactNotify = bUseImpactNotify && MontageDuration > KINDA_SMALL_NUMBER;
	Plan.bScheduleImpactTimer = Plan.bWaitForImpactNotify && Plan.ImpactDelay > KINDA_SMALL_NUMBER;
	Plan.bApplyImpactImmediately = !Plan.bWaitForImpactNotify || !Plan.bScheduleImpactTimer;
	return Plan;
}
