/** 행동 시작·중단·재시작 이유를 타이머 해제 계획으로 변환한다. 실제 TimerManager 접근은 전투 컴포넌트가 담당한다. */
#include "AetherCombatActionTimerPolicy.h"

/** 전이 이유별로 해제할 타이머만 지정한다. 피격 중단은 기존 무적 종료 타이머를 유지하고 전체 초기화는 모든 관련 타이머를 지운다. */
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

/** 실행 중 행동과 지연 판정이 다음 상태에 남지 않도록 전체 해제 플래그를 구성한다. */
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
