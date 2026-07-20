#include "AetherCombatActionGatePolicy.h"

FAetherCombatActionGateResult FAetherCombatActionGatePolicy::Blocked(const FString& Message, const FColor& Color)
{
	FAetherCombatActionGateResult Result;
	Result.bCanStartAction = false;
	Result.FailureMessage = Message;
	Result.MessageColor = Color;
	return Result;
}

FAetherCombatActionGateResult FAetherCombatActionGatePolicy::EvaluateLightAttack(const FAetherCombatActionStateSnapshot& State)
{
	if (State.bOwnerDead)
	{
		return Blocked(TEXT("Player is defeated. Press Y to reset."), FColor::Red);
	}

	if (State.bGuarding)
	{
		return Blocked(TEXT("Cannot attack while guarding"));
	}

	if (State.bHitReacting)
	{
		return Blocked(TEXT("Cannot attack while hit reacting"));
	}

	if (State.bExecuting)
	{
		return Blocked(TEXT("Cannot attack while executing"));
	}

	if (State.bHeavyAttacking)
	{
		return Blocked(TEXT("Cannot light attack while heavy attacking"));
	}

	if (State.bAetherSlashing)
	{
		return Blocked(TEXT("Cannot light attack while Aether Slashing"));
	}

	if (State.bDodging)
	{
		return Blocked(TEXT("Cannot attack while dodging"));
	}

	if (State.bAttacking)
	{
		FAetherCombatActionGateResult Result = Blocked(TEXT("Light attack buffered"));
		Result.bShouldQueueLightAttack = true;
		return Result;
	}

	return FAetherCombatActionGateResult();
}

FAetherCombatActionGateResult FAetherCombatActionGatePolicy::EvaluateHeavyAttack(const FAetherCombatActionStateSnapshot& State)
{
	if (State.bOwnerDead)
	{
		return Blocked(TEXT("Player is defeated. Press Y to reset."), FColor::Red);
	}

	if (State.bGuarding)
	{
		return Blocked(TEXT("Cannot heavy attack while guarding"));
	}

	if (State.bHitReacting)
	{
		return Blocked(TEXT("Cannot heavy attack while hit reacting"));
	}

	if (State.bExecuting)
	{
		return Blocked(TEXT("Cannot heavy attack while executing"));
	}

	if (State.bDodging)
	{
		return Blocked(TEXT("Cannot heavy attack while dodging"));
	}

	if ((State.bParryWindowActive || State.bParryRecovering) && !State.bParryCounterWindowActive)
	{
		return Blocked(TEXT("Cannot heavy attack while parrying"));
	}

	if (State.bAttacking)
	{
		return Blocked(TEXT("Cannot heavy attack while attacking"));
	}

	return FAetherCombatActionGateResult();
}

FAetherCombatActionGateResult FAetherCombatActionGatePolicy::EvaluateAetherSlash(
	const FAetherCombatActionStateSnapshot& State,
	double RemainingCooldown)
{
	if (State.bOwnerDead)
	{
		return Blocked(TEXT("Player is defeated. Press Y to reset."), FColor::Red);
	}

	if (State.bGuarding)
	{
		return Blocked(TEXT("Cannot Aether Slash while guarding"));
	}

	if (State.bHitReacting)
	{
		return Blocked(TEXT("Cannot Aether Slash while hit reacting"));
	}

	if (State.bExecuting)
	{
		return Blocked(TEXT("Cannot Aether Slash while executing"));
	}

	if (State.bDodging)
	{
		return Blocked(TEXT("Cannot Aether Slash while dodging"));
	}

	if (State.bParryWindowActive || State.bParryRecovering)
	{
		return Blocked(TEXT("Cannot Aether Slash while parrying"));
	}

	if (State.bAetherSlashing)
	{
		return Blocked(TEXT("Aether Slash active"));
	}

	if (State.bAttacking)
	{
		return Blocked(TEXT("Cannot Aether Slash while attacking"));
	}

	if (RemainingCooldown > 0.0)
	{
		return Blocked(FString::Printf(TEXT("Aether Slash cooldown: %.1f"), RemainingCooldown));
	}

	return FAetherCombatActionGateResult();
}

FAetherCombatActionGateResult FAetherCombatActionGatePolicy::EvaluateDodge(
	const FAetherCombatActionStateSnapshot& State,
	double CurrentTimeSeconds,
	double LastDodgeTimeSeconds,
	float DodgeCooldown,
	float CurrentStamina,
	float DodgeStaminaCost)
{
	if (State.bOwnerDead)
	{
		return Blocked(TEXT("Player is defeated. Press Y to reset."), FColor::Red);
	}

	if (State.bDodging)
	{
		return Blocked(TEXT("Dodge already active"));
	}

	if (State.bHitReacting)
	{
		return Blocked(TEXT("Cannot dodge while hit reacting"));
	}

	if (State.bExecuting)
	{
		return Blocked(TEXT("Cannot dodge while executing"));
	}

	if (State.bAttacking)
	{
		return Blocked(TEXT("Cannot dodge while attacking"));
	}

	if (State.bGuarding)
	{
		return Blocked(TEXT("Cannot dodge while guarding"));
	}

	const double RemainingCooldown = DodgeCooldown - (CurrentTimeSeconds - LastDodgeTimeSeconds);
	if (RemainingCooldown > 0.0)
	{
		return Blocked(FString::Printf(TEXT("Dodge cooldown: %.1f"), RemainingCooldown));
	}

	if (CurrentStamina < DodgeStaminaCost)
	{
		return Blocked(FString::Printf(TEXT("Not enough stamina for dodge: %.1f / %.1f"), CurrentStamina, DodgeStaminaCost), FColor::Red);
	}

	return FAetherCombatActionGateResult();
}

FAetherCombatActionGateResult FAetherCombatActionGatePolicy::EvaluateGuard(
	const FAetherCombatActionStateSnapshot& State,
	float CurrentStamina)
{
	if (State.bOwnerDead)
	{
		return Blocked(TEXT("Player is defeated. Press Y to reset."), FColor::Red);
	}

	if (State.bAttacking)
	{
		return Blocked(TEXT("Cannot guard while attacking"));
	}

	if (State.bHitReacting)
	{
		return Blocked(TEXT("Cannot guard while hit reacting"));
	}

	if (State.bExecuting)
	{
		return Blocked(TEXT("Cannot guard while executing"));
	}

	if (State.bDodging)
	{
		return Blocked(TEXT("Cannot guard while dodging"));
	}

	if (State.bParryWindowActive || State.bParryRecovering)
	{
		return Blocked(TEXT("Cannot guard while parrying"));
	}

	if (CurrentStamina <= 0.0f)
	{
		return Blocked(TEXT("Not enough stamina to guard"), FColor::Red);
	}

	return FAetherCombatActionGateResult();
}

FAetherCombatActionGateResult FAetherCombatActionGatePolicy::EvaluateParry(
	const FAetherCombatActionStateSnapshot& State,
	float CurrentStamina,
	float ParryStaminaCost)
{
	if (State.bOwnerDead)
	{
		return Blocked(TEXT("Player is defeated. Press Y to reset."), FColor::Red);
	}

	if (State.bAttacking)
	{
		return Blocked(TEXT("Cannot parry while attacking"));
	}

	if (State.bHitReacting)
	{
		return Blocked(TEXT("Cannot parry while hit reacting"));
	}

	if (State.bExecuting)
	{
		return Blocked(TEXT("Cannot parry while executing"));
	}

	if (State.bDodging)
	{
		return Blocked(TEXT("Cannot parry while dodging"));
	}

	if (State.bParryWindowActive || State.bParryRecovering)
	{
		return Blocked(TEXT("Parry recovering"));
	}

	if (CurrentStamina < ParryStaminaCost)
	{
		return Blocked(FString::Printf(TEXT("Not enough stamina for parry: %.1f / %.1f"), CurrentStamina, ParryStaminaCost), FColor::Red);
	}

	return FAetherCombatActionGateResult();
}

bool FAetherCombatActionGatePolicy::ShouldBlockMovementInput(const FAetherCombatActionStateSnapshot& State)
{
	return State.bAttacking || State.bDodging || State.bExecuting || State.bHitReacting || State.bParryWindowActive || State.bParryRecovering;
}

bool FAetherCombatActionGatePolicy::CanRegenerateStamina(
	const FAetherCombatActionStateSnapshot& State,
	float CurrentStamina,
	float MaxStamina,
	double CurrentTimeSeconds,
	double LastStaminaSpendTimeSeconds,
	float StaminaRegenDelay)
{
	if (State.bAttacking || State.bExecuting || State.bDodging || State.bGuarding || State.bParryWindowActive || State.bHitReacting || CurrentStamina >= MaxStamina)
	{
		return false;
	}

	return CurrentTimeSeconds - LastStaminaSpendTimeSeconds >= StaminaRegenDelay;
}
