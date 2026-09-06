/** 스태미나와 에테르 게이지의 변경값 및 소비 시각을 계산하여, 컴포넌트가 적용할 변경 계획으로 반환한다. */
#include "AetherCombatResourcePolicy.h"

FAetherCombatResourceMutationPlan FAetherCombatResourcePolicy::BuildInitializePlan(float MaxStamina)
{
	FAetherCombatResourceMutationPlan Plan;
	Plan.bUpdateStamina = true;
	Plan.NewStamina = FMath::Max(0.0f, MaxStamina);
	Plan.StaminaDelta = Plan.NewStamina;
	Plan.bUpdateAetherGauge = true;
	return Plan;
}

/** 스태미나를 최대값으로, 게이지를 0으로 초기화하고 회복 지연의 기준 시각을 기록한다. */
FAetherCombatResourceMutationPlan FAetherCombatResourcePolicy::BuildResetPlan(float MaxStamina, double CurrentTime)
{
	FAetherCombatResourceMutationPlan Plan = BuildInitializePlan(MaxStamina);
	Plan.bRecordStaminaSpendTime = true;
	Plan.NewLastStaminaSpendTime = CurrentTime;
	return Plan;
}

/** 비용을 0 이상으로 제한해 차감값과 소비 시각을 계산한다. 잔량 부족 여부는 호출 전 행동 허용 판정에서 확인한다. */
FAetherCombatResourceMutationPlan FAetherCombatResourcePolicy::BuildStaminaSpendPlan(float CurrentStamina, float StaminaCost, double CurrentTime)
{
	FAetherCombatResourceMutationPlan Plan;
	const float ClampedCost = FMath::Max(0.0f, StaminaCost);
	Plan.bUpdateStamina = ClampedCost > 0.0f;
	Plan.PreviousStamina = CurrentStamina;
	Plan.NewStamina = FMath::Max(0.0f, CurrentStamina - ClampedCost);
	Plan.StaminaDelta = Plan.NewStamina - Plan.PreviousStamina;
	Plan.bRecordStaminaSpendTime = true;
	Plan.NewLastStaminaSpendTime = CurrentTime;
	return Plan;
}

FAetherCombatResourceMutationPlan FAetherCombatResourcePolicy::BuildStaminaTimestampPlan(double CurrentTime)
{
	FAetherCombatResourceMutationPlan Plan;
	Plan.bRecordStaminaSpendTime = true;
	Plan.NewLastStaminaSpendTime = CurrentTime;
	return Plan;
}

/** 음수 회복량과 경과 시간을 배제하고 최대 스태미나를 넘지 않도록 회복 계획을 만든다. */
FAetherCombatResourceMutationPlan FAetherCombatResourcePolicy::BuildStaminaRegenPlan(float CurrentStamina, float MaxStamina, float RegenPerSecond, float DeltaTime)
{
	FAetherCombatResourceMutationPlan Plan;
	Plan.PreviousStamina = CurrentStamina;
	Plan.NewStamina = FMath::Min(FMath::Max(0.0f, MaxStamina), CurrentStamina + FMath::Max(0.0f, RegenPerSecond) * FMath::Max(0.0f, DeltaTime));
	Plan.StaminaDelta = Plan.NewStamina - Plan.PreviousStamina;
	Plan.bUpdateStamina = !FMath::IsNearlyEqual(Plan.NewStamina, Plan.PreviousStamina);
	return Plan;
}

FAetherCombatResourceMutationPlan FAetherCombatResourcePolicy::BuildAetherGainPlan(float CurrentAetherGauge, float MaxAetherGauge, float Amount)
{
	FAetherCombatResourceMutationPlan Plan;
	if (Amount <= 0.0f || MaxAetherGauge <= 0.0f)
	{
		return Plan;
	}

	Plan.PreviousAetherGauge = CurrentAetherGauge;
	Plan.NewAetherGauge = FMath::Clamp(CurrentAetherGauge + Amount, 0.0f, MaxAetherGauge);
	Plan.AetherGaugeDelta = Plan.NewAetherGauge - Plan.PreviousAetherGauge;
	Plan.bUpdateAetherGauge = !FMath::IsNearlyEqual(Plan.NewAetherGauge, Plan.PreviousAetherGauge);
	return Plan;
}

/** 게이지가 비용보다 부족하면 적용 불가를 반환하여 자원 변경을 막는다. */
FAetherCombatResourceMutationPlan FAetherCombatResourcePolicy::BuildAetherSpendPlan(float CurrentAetherGauge, float Amount)
{
	FAetherCombatResourceMutationPlan Plan;
	if (Amount <= 0.0f)
	{
		return Plan;
	}

	if (CurrentAetherGauge < Amount)
	{
		Plan.bCanApply = false;
		return Plan;
	}

	Plan.PreviousAetherGauge = CurrentAetherGauge;
	Plan.NewAetherGauge = FMath::Max(0.0f, CurrentAetherGauge - Amount);
	Plan.AetherGaugeDelta = Plan.NewAetherGauge - Plan.PreviousAetherGauge;
	Plan.bUpdateAetherGauge = !FMath::IsNearlyEqual(Plan.NewAetherGauge, Plan.PreviousAetherGauge);
	return Plan;
}
