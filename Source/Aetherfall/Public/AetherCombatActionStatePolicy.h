#pragma once

#include "CoreMinimal.h"
#include "AetherCombatActionGatePolicy.h"

enum class EAetherCombatActionMode : uint8
{
	Idle,
	LightAttack,
	HeavyAttack,
	AetherSlash,
	Execution,
	Dodge,
	Guard,
	ParryWindow,
	ParryRecovery,
	ParryCounterWindow,
	HitReaction
};

struct FAetherCombatActionRuntimeFlags
{
	bool bAttacking = false;
	bool bHeavyAttacking = false;
	bool bExecuting = false;
	bool bAetherSlashing = false;
	bool bDodging = false;
	bool bGuarding = false;
	bool bParryWindowActive = false;
	bool bParryRecovering = false;
	bool bParryCounterWindowActive = false;
	bool bHitReacting = false;
};

class AETHERFALL_API FAetherCombatActionStatePolicy
{
public:
	static FAetherCombatActionRuntimeFlags BuildFlagsForMode(EAetherCombatActionMode Mode);
	static FAetherCombatActionStateSnapshot BuildSnapshot(bool bOwnerDead, const FAetherCombatActionRuntimeFlags& RuntimeFlags);
	static EAetherCombatActionMode ResolveDominantMode(const FAetherCombatActionStateSnapshot& State);
	static bool IsAttackMode(EAetherCombatActionMode Mode);
	static bool IsInterruptMode(EAetherCombatActionMode Mode);
};
