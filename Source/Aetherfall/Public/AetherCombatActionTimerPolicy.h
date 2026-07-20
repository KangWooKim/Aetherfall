#pragma once

#include "CoreMinimal.h"

enum class EAetherCombatActionTimerClearReason : uint8
{
	StartLightAttack,
	StartExecution,
	StartAetherSlash,
	StartHeavyAttack,
	StartAutoParryCounter,
	OpenParryCounterWindow,
	RefreshDamageInvulnerability,
	ParrySuccess,
	HitReaction,
	RuntimeReset
};

struct FAetherCombatActionTimerClearPlan
{
	bool bClearAttackEnd = false;
	bool bClearComboReset = false;
	bool bClearDodgeEnd = false;
	bool bClearParryWindow = false;
	bool bClearParryRecovery = false;
	bool bClearParryCounterWindow = false;
	bool bClearDamageInvulnerability = false;
	bool bClearHitReaction = false;
	bool bClearHeavyImpact = false;
	bool bClearAetherSlashImpact = false;
	bool bClearExecutionImpact = false;
	bool bClearExecution = false;
};

class AETHERFALL_API FAetherCombatActionTimerPolicy
{
public:
	static FAetherCombatActionTimerClearPlan BuildClearPlan(EAetherCombatActionTimerClearReason Reason);

private:
	static FAetherCombatActionTimerClearPlan BuildActionStartPlan();
	static FAetherCombatActionTimerClearPlan BuildFullInterruptPlan();
};
