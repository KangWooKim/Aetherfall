/** 현재 상태 스냅샷으로 행동 허용 여부와 실패 안내를 계산한다. 자원 차감이나 타이머 변경은 수행하지 않는다. */
#include "AetherCombatActionGatePolicy.h"

FAetherCombatActionGateResult FAetherCombatActionGatePolicy::Blocked(const FString& Message, const FColor& Color)
{
	FAetherCombatActionGateResult Result;
	Result.bCanStartAction = false;
	Result.FailureMessage = Message;
	Result.MessageColor = Color;
	return Result;
}

/** 충돌하는 상태를 먼저 거절한다. 약공격 도중 입력은 즉시 시작 대신 다음 공격 예약 플래그로 돌려준다. */
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

/** 현재 행동과 패링 상태를 확인한다. 반격 허용 시간이 열렸을 때에는 패링 중에도 강공격을 허용한다. */
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

/** 행동 충돌과 남은 재사용 대기 시간을 검사한다. 게이지 차감은 전투 컴포넌트의 별도 경로에서 처리한다. */
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

/** 상태 충돌, 회피 재사용 대기, 스태미나 순으로 검사하고 처음 실패한 이유를 반환한다. */
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

/** 상태 충돌과 남은 스태미나를 검사한다. 방어로 실제 소모할 자원은 이 단계에서 변경하지 않는다. */
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

/** 패링의 상태 충돌과 비용 충족 여부를 검사한다. 성공해도 판정 시간과 자원은 호출자가 적용해야 한다. */
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

/** 행동 제한과 최대치 도달 여부를 먼저 확인하고 마지막 소비 이후 회복 지연 시간이 지났는지 판단한다. */
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
