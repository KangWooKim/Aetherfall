#pragma once

#include "CoreMinimal.h"

/** 행동 종료·중단·초기화 상황에 따라 해제할 타이머를 선택하는 이유다. */
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

/** 상황별로 유지하거나 해제해야 할 지연 작업을 명시한다. */
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

/** 행동 시작·중단·재시작 이유를 타이머 해제 계획으로 변환한다. 실제 TimerManager 접근은 전투 컴포넌트가 담당한다. */
class AETHERFALL_API FAetherCombatActionTimerPolicy
{
public:
	/** 전이 이유별로 해제할 타이머만 지정한다. 피격 중단은 기존 무적 종료 타이머를 유지하고 전체 초기화는 모든 관련 타이머를 지운다. */
	static FAetherCombatActionTimerClearPlan BuildClearPlan(EAetherCombatActionTimerClearReason Reason);

private:
	static FAetherCombatActionTimerClearPlan BuildActionStartPlan();
	/** 실행 중 행동과 지연 판정이 다음 상태에 남지 않도록 전체 해제 플래그를 구성한다. */
	static FAetherCombatActionTimerClearPlan BuildFullInterruptPlan();
};
