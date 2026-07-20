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

FAetherCombatResourceMutationPlan FAetherCombatResourcePolicy::BuildResetPlan(float MaxStamina, double CurrentTime)
{
	FAetherCombatResourceMutationPlan Plan = BuildInitializePlan(MaxStamina);
	Plan.bRecordStaminaSpendTime = true;
	Plan.NewLastStaminaSpendTime = CurrentTime;
	return Plan;
}

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
