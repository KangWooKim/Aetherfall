#pragma once

#include "CoreMinimal.h"

struct FAetherCombatActionExecutionPlan
{
	float Duration = 0.1f;
	float ImpactDelay = 0.0f;
	bool bWaitForImpactNotify = false;
	bool bApplyImpactImmediately = false;
	bool bScheduleImpactTimer = false;
};

class AETHERFALL_API FAetherCombatActionExecutionPolicy
{
public:
	static FAetherCombatActionExecutionPlan BuildLightAttackPlan(float Duration, bool bUseImpactNotify);
	static FAetherCombatActionExecutionPlan BuildHeavyAttackPlan(float Duration, float ImpactDelay, bool bUseImpactNotify);
	static FAetherCombatActionExecutionPlan BuildAetherSlashPlan(float Duration, float ImpactDelay, bool bUseImpactNotify);
	static FAetherCombatActionExecutionPlan BuildAutoParryCounterPlan(float Duration, float ImpactDelay, bool bUseImpactNotify);
	static FAetherCombatActionExecutionPlan BuildExecutionPlan(
		float ConfiguredDuration,
		float SelectedMontageLength,
		bool bUseImpactNotify,
		float ImpactFallbackNormalizedTime);

private:
	static float ClampDuration(float Duration);
	static float ClampImpactDelay(float ImpactDelay, float Duration);
};
