#pragma once

#include "CoreMinimal.h"

struct FAetherGuardStaminaTuning
{
	float BaseCost = 12.0f;
	bool bScaleByIncomingDamage = true;
	float DamageReference = 24.0f;
	float MinCost = 8.0f;
	float MaxCost = 18.0f;
};

class AETHERFALL_API FAetherCombatActionTuningPolicy
{
public:
	static float SelectLightAttackCost(int32 ComboStep, const TArray<float>& StaminaCosts);
	static float SelectLightAttackDamage(int32 ComboStep, const TArray<float>& DamageValues);
	static float CalculateHeavyAttackDamage(float BaseDamage, float StaggerDamageMultiplier, bool bStaggerBonus);
	static float CalculateGuardStaminaCost(float IncomingDamage, const FAetherGuardStaminaTuning& GuardTuning);

private:
	static float SelectComboValue(int32 ComboStep, const TArray<float>& Values, float DefaultValue);
};
