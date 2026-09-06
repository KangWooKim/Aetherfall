/** 전투 행동 모드를 기존 실행 플래그와 판정용 스냅샷으로 변환한다. 여러 플래그가 함께 켜지면 정해진 우선순위로 대표 모드를 해석한다. */
#include "AetherCombatActionStatePolicy.h"

/** 새 플래그 묶음을 기본값에서 구성한다. 강공격과 참격은 공통 공격 플래그도 함께 켜진다. */
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

/** 실행 플래그를 복사하고 소유자의 사망 여부를 추가해 행동 판정의 입력으로 만든다. */
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

/** 피격·처형·참격 등의 우선순위로 현재 모드를 해석한다. 결과를 조회할 뿐 입력 플래그를 수정하지 않는다. */
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
