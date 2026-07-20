#include "AetherCombatActionTuningPolicy.h"

float FAetherCombatActionTuningPolicy::SelectLightAttackCost(int32 ComboStep, const TArray<float>& StaminaCosts)
{
	return SelectComboValue(ComboStep, StaminaCosts, 20.0f);
}

float FAetherCombatActionTuningPolicy::SelectLightAttackDamage(int32 ComboStep, const TArray<float>& DamageValues)
{
	return SelectComboValue(ComboStep, DamageValues, 20.0f);
}

float FAetherCombatActionTuningPolicy::CalculateHeavyAttackDamage(float BaseDamage, float StaggerDamageMultiplier, bool bStaggerBonus)
{
	const float ClampedBaseDamage = FMath::Max(0.0f, BaseDamage);
	if (!bStaggerBonus)
	{
		return ClampedBaseDamage;
	}

	return ClampedBaseDamage * FMath::Max(0.0f, StaggerDamageMultiplier);
}

float FAetherCombatActionTuningPolicy::CalculateGuardStaminaCost(float IncomingDamage, const FAetherGuardStaminaTuning& GuardTuning)
{
	const float BaseCost = FMath::Max(0.0f, GuardTuning.BaseCost);
	if (!GuardTuning.bScaleByIncomingDamage || GuardTuning.DamageReference <= 0.0f)
	{
		return BaseCost;
	}

	const float ScaledCost = BaseCost * FMath::Max(0.0f, IncomingDamage) / GuardTuning.DamageReference;
	const float MinCost = FMath::Min(GuardTuning.MinCost, GuardTuning.MaxCost);
	const float MaxCost = FMath::Max(GuardTuning.MinCost, GuardTuning.MaxCost);
	return FMath::Clamp(ScaledCost, FMath::Max(0.0f, MinCost), FMath::Max(0.0f, MaxCost));
}

float FAetherCombatActionTuningPolicy::SelectComboValue(int32 ComboStep, const TArray<float>& Values, float DefaultValue)
{
	const int32 ValueIndex = ComboStep - 1;
	if (Values.IsValidIndex(ValueIndex))
	{
		return Values[ValueIndex];
	}

	return Values.Num() > 0 ? Values.Last() : DefaultValue;
}
