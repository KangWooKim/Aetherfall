#pragma once

#include "CoreMinimal.h"

struct FAetherCombatActionStateSnapshot
{
	bool bOwnerDead = false;
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

struct FAetherCombatActionGateResult
{
	bool bCanStartAction = true;
	bool bShouldQueueLightAttack = false;
	FString FailureMessage;
	FColor MessageColor = FColor::Yellow;
};

class AETHERFALL_API FAetherCombatActionGatePolicy
{
public:
	static FAetherCombatActionGateResult EvaluateLightAttack(const FAetherCombatActionStateSnapshot& State);
	static FAetherCombatActionGateResult EvaluateHeavyAttack(const FAetherCombatActionStateSnapshot& State);
	static FAetherCombatActionGateResult EvaluateAetherSlash(const FAetherCombatActionStateSnapshot& State, double RemainingCooldown);
	static FAetherCombatActionGateResult EvaluateDodge(
		const FAetherCombatActionStateSnapshot& State,
		double CurrentTimeSeconds,
		double LastDodgeTimeSeconds,
		float DodgeCooldown,
		float CurrentStamina,
		float DodgeStaminaCost);
	static FAetherCombatActionGateResult EvaluateGuard(const FAetherCombatActionStateSnapshot& State, float CurrentStamina);
	static FAetherCombatActionGateResult EvaluateParry(const FAetherCombatActionStateSnapshot& State, float CurrentStamina, float ParryStaminaCost);
	static bool ShouldBlockMovementInput(const FAetherCombatActionStateSnapshot& State);
	static bool CanRegenerateStamina(
		const FAetherCombatActionStateSnapshot& State,
		float CurrentStamina,
		float MaxStamina,
		double CurrentTimeSeconds,
		double LastStaminaSpendTimeSeconds,
		float StaminaRegenDelay);

private:
	static FAetherCombatActionGateResult Blocked(const FString& Message, const FColor& Color = FColor::Yellow);
};
