#include "AetherCombatActionStatePolicy.h"

FAetherCombatActionRuntimeFlags FAetherCombatActionStatePolicy::BuildFlagsForMode(EAetherCombatActionMode Mode)
{
	FAetherCombatActionRuntimeFlags Flags;

	switch (Mode)
	{
	case EAetherCombatActionMode::LightAttack:
		Flags.bAttacking = true;
		break;
	case EAetherCombatActionMode::HeavyAttack:
		Flags.bAttacking = true;
		Flags.bHeavyAttacking = true;
		break;
	case EAetherCombatActionMode::AetherSlash:
		Flags.bAttacking = true;
		Flags.bAetherSlashing = true;
		break;
	case EAetherCombatActionMode::Execution:
		Flags.bExecuting = true;
		break;
	case EAetherCombatActionMode::Dodge:
		Flags.bDodging = true;
		break;
	case EAetherCombatActionMode::Guard:
		Flags.bGuarding = true;
		break;
	case EAetherCombatActionMode::ParryWindow:
		Flags.bParryWindowActive = true;
		break;
	case EAetherCombatActionMode::ParryRecovery:
		Flags.bParryRecovering = true;
		break;
	case EAetherCombatActionMode::ParryCounterWindow:
		Flags.bParryCounterWindowActive = true;
		break;
	case EAetherCombatActionMode::HitReaction:
		Flags.bHitReacting = true;
		break;
	case EAetherCombatActionMode::Idle:
	default:
		break;
	}

	return Flags;
}

FAetherCombatActionStateSnapshot FAetherCombatActionStatePolicy::BuildSnapshot(
	bool bOwnerDead,
	const FAetherCombatActionRuntimeFlags& RuntimeFlags)
{
	FAetherCombatActionStateSnapshot State;
	State.bOwnerDead = bOwnerDead;
	State.bAttacking = RuntimeFlags.bAttacking;
	State.bHeavyAttacking = RuntimeFlags.bHeavyAttacking;
	State.bExecuting = RuntimeFlags.bExecuting;
	State.bAetherSlashing = RuntimeFlags.bAetherSlashing;
	State.bDodging = RuntimeFlags.bDodging;
	State.bGuarding = RuntimeFlags.bGuarding;
	State.bParryWindowActive = RuntimeFlags.bParryWindowActive;
	State.bParryRecovering = RuntimeFlags.bParryRecovering;
	State.bParryCounterWindowActive = RuntimeFlags.bParryCounterWindowActive;
	State.bHitReacting = RuntimeFlags.bHitReacting;
	return State;
}

EAetherCombatActionMode FAetherCombatActionStatePolicy::ResolveDominantMode(const FAetherCombatActionStateSnapshot& State)
{
	if (State.bHitReacting)
	{
		return EAetherCombatActionMode::HitReaction;
	}
	if (State.bExecuting)
	{
		return EAetherCombatActionMode::Execution;
	}
	if (State.bAetherSlashing)
	{
		return EAetherCombatActionMode::AetherSlash;
	}
	if (State.bHeavyAttacking)
	{
		return EAetherCombatActionMode::HeavyAttack;
	}
	if (State.bAttacking)
	{
		return EAetherCombatActionMode::LightAttack;
	}
	if (State.bDodging)
	{
		return EAetherCombatActionMode::Dodge;
	}
	if (State.bParryCounterWindowActive)
	{
		return EAetherCombatActionMode::ParryCounterWindow;
	}
	if (State.bParryWindowActive)
	{
		return EAetherCombatActionMode::ParryWindow;
	}
	if (State.bParryRecovering)
	{
		return EAetherCombatActionMode::ParryRecovery;
	}
	if (State.bGuarding)
	{
		return EAetherCombatActionMode::Guard;
	}

	return EAetherCombatActionMode::Idle;
}

bool FAetherCombatActionStatePolicy::IsAttackMode(EAetherCombatActionMode Mode)
{
	return Mode == EAetherCombatActionMode::LightAttack
		|| Mode == EAetherCombatActionMode::HeavyAttack
		|| Mode == EAetherCombatActionMode::AetherSlash;
}

bool FAetherCombatActionStatePolicy::IsInterruptMode(EAetherCombatActionMode Mode)
{
	return Mode == EAetherCombatActionMode::Execution
		|| Mode == EAetherCombatActionMode::Dodge
		|| Mode == EAetherCombatActionMode::HitReaction;
}
