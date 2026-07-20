#include "AetherCombatActionTimerPolicy.h"

FAetherCombatActionTimerClearPlan FAetherCombatActionTimerPolicy::BuildClearPlan(EAetherCombatActionTimerClearReason Reason)
{
	switch (Reason)
	{
	case EAetherCombatActionTimerClearReason::StartLightAttack:
	{
		FAetherCombatActionTimerClearPlan Plan;
		Plan.bClearComboReset = true;
		return Plan;
	}
	case EAetherCombatActionTimerClearReason::StartExecution:
	{
		FAetherCombatActionTimerClearPlan Plan = BuildActionStartPlan();
		Plan.bClearExecutionImpact = true;
		Plan.bClearExecution = true;
		return Plan;
	}
	case EAetherCombatActionTimerClearReason::StartAetherSlash:
	{
		FAetherCombatActionTimerClearPlan Plan = BuildActionStartPlan();
		Plan.bClearAetherSlashImpact = true;
		return Plan;
	}
	case EAetherCombatActionTimerClearReason::StartHeavyAttack:
	{
		FAetherCombatActionTimerClearPlan Plan;
		Plan.bClearComboReset = true;
		Plan.bClearParryCounterWindow = true;
		Plan.bClearHeavyImpact = true;
		return Plan;
	}
	case EAetherCombatActionTimerClearReason::StartAutoParryCounter:
	{
		FAetherCombatActionTimerClearPlan Plan;
		Plan.bClearAttackEnd = true;
		Plan.bClearComboReset = true;
		Plan.bClearParryCounterWindow = true;
		Plan.bClearHeavyImpact = true;
		return Plan;
	}
	case EAetherCombatActionTimerClearReason::OpenParryCounterWindow:
	{
		FAetherCombatActionTimerClearPlan Plan;
		Plan.bClearParryCounterWindow = true;
		return Plan;
	}
	case EAetherCombatActionTimerClearReason::RefreshDamageInvulnerability:
	{
		FAetherCombatActionTimerClearPlan Plan;
		Plan.bClearDamageInvulnerability = true;
		return Plan;
	}
	case EAetherCombatActionTimerClearReason::ParrySuccess:
	{
		FAetherCombatActionTimerClearPlan Plan;
		Plan.bClearParryWindow = true;
		Plan.bClearParryRecovery = true;
		Plan.bClearParryCounterWindow = true;
		return Plan;
	}
	case EAetherCombatActionTimerClearReason::HitReaction:
	{
		FAetherCombatActionTimerClearPlan Plan = BuildFullInterruptPlan();
		Plan.bClearDamageInvulnerability = false;
		return Plan;
	}
	case EAetherCombatActionTimerClearReason::RuntimeReset:
		return BuildFullInterruptPlan();
	default:
		return FAetherCombatActionTimerClearPlan();
	}
}

FAetherCombatActionTimerClearPlan FAetherCombatActionTimerPolicy::BuildActionStartPlan()
{
	FAetherCombatActionTimerClearPlan Plan;
	Plan.bClearAttackEnd = true;
	Plan.bClearComboReset = true;
	Plan.bClearParryCounterWindow = true;
	Plan.bClearHeavyImpact = true;
	return Plan;
}

FAetherCombatActionTimerClearPlan FAetherCombatActionTimerPolicy::BuildFullInterruptPlan()
{
	FAetherCombatActionTimerClearPlan Plan;
	Plan.bClearAttackEnd = true;
	Plan.bClearComboReset = true;
	Plan.bClearDodgeEnd = true;
	Plan.bClearParryWindow = true;
	Plan.bClearParryRecovery = true;
	Plan.bClearParryCounterWindow = true;
	Plan.bClearDamageInvulnerability = true;
	Plan.bClearHitReaction = true;
	Plan.bClearHeavyImpact = true;
	Plan.bClearAetherSlashImpact = true;
	Plan.bClearExecutionImpact = true;
	Plan.bClearExecution = true;
	return Plan;
}
