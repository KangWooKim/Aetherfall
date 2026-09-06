/** 행동 지속 시간과 타격 시점을 계산하는 월드 비의존 정책이다. 즉시 타격, 애니메이션 알림 대기, 타이머 예약 여부만 반환한다. */
#include "AetherCombatActionExecutionPolicy.h"

float FAetherCombatActionExecutionPolicy::ClampDuration(float Duration)
{
	return FMath::Max(Duration, 0.1f);
}

float FAetherCombatActionExecutionPolicy::ClampImpactDelay(float ImpactDelay, float Duration)
{
	return FMath::Clamp(FMath::Max(ImpactDelay, 0.0f), 0.0f, ClampDuration(Duration));
}

/** 약공격은 알림을 사용할 때 판정을 기다리고, 사용하지 않으면 즉시 판정하도록 계획한다. */
FAetherCombatActionExecutionPlan FAetherCombatActionExecutionPolicy::BuildLightAttackPlan(float Duration, bool bUseImpactNotify)
{
	FAetherCombatActionExecutionPlan Plan;
	Plan.Duration = ClampDuration(Duration);
	Plan.bWaitForImpactNotify = bUseImpactNotify;
	Plan.bApplyImpactImmediately = !Plan.bWaitForImpactNotify;
	return Plan;
}

/** 강공격의 판정 지연을 행동 시간 안으로 제한한다. 알림을 사용하지 않을 때만 판정 타이머를 예약한다. */
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

/** 처형은 설정 시간과 몽타주 길이를 함께 고려한다. 알림 경로에서도 정규화된 시점의 대체 판정을 준비하므로 호출자는 중복 타격을 막아야 한다. */
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
