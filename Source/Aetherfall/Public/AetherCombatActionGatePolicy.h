#pragma once

#include "CoreMinimal.h"

/** 행동 가능 여부를 판단하기 위해 현재 전투 플래그를 복사한 입력이다. */
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

/** 행동 허용 여부와 거절 사유를 함께 반환한다. */
struct FAetherCombatActionGateResult
{
	bool bCanStartAction = true;
	bool bShouldQueueLightAttack = false;
	FString FailureMessage;
	FColor MessageColor = FColor::Yellow;
};

/** 현재 상태 스냅샷으로 행동 허용 여부와 실패 안내를 계산한다. 자원 차감이나 타이머 변경은 수행하지 않는다. */
class AETHERFALL_API FAetherCombatActionGatePolicy
{
public:
	/** 충돌하는 상태를 먼저 거절한다. 약공격 도중 입력은 즉시 시작 대신 다음 공격 예약 플래그로 돌려준다. */
	static FAetherCombatActionGateResult EvaluateLightAttack(const FAetherCombatActionStateSnapshot& State);
	/** 현재 행동과 패링 상태를 확인한다. 반격 허용 시간이 열렸을 때에는 패링 중에도 강공격을 허용한다. */
	static FAetherCombatActionGateResult EvaluateHeavyAttack(const FAetherCombatActionStateSnapshot& State);
	/** 행동 충돌과 남은 재사용 대기 시간을 검사한다. 게이지 차감은 전투 컴포넌트의 별도 경로에서 처리한다. */
	static FAetherCombatActionGateResult EvaluateAetherSlash(const FAetherCombatActionStateSnapshot& State, double RemainingCooldown);
	/** 상태 충돌, 회피 재사용 대기, 스태미나 순으로 검사하고 처음 실패한 이유를 반환한다. */
	static FAetherCombatActionGateResult EvaluateDodge(
		const FAetherCombatActionStateSnapshot& State,
		double CurrentTimeSeconds,
		double LastDodgeTimeSeconds,
		float DodgeCooldown,
		float CurrentStamina,
		float DodgeStaminaCost);
	/** 상태 충돌과 남은 스태미나를 검사한다. 방어로 실제 소모할 자원은 이 단계에서 변경하지 않는다. */
	static FAetherCombatActionGateResult EvaluateGuard(const FAetherCombatActionStateSnapshot& State, float CurrentStamina);
	/** 패링의 상태 충돌과 비용 충족 여부를 검사한다. 성공해도 판정 시간과 자원은 호출자가 적용해야 한다. */
	static FAetherCombatActionGateResult EvaluateParry(const FAetherCombatActionStateSnapshot& State, float CurrentStamina, float ParryStaminaCost);
	static bool ShouldBlockMovementInput(const FAetherCombatActionStateSnapshot& State);
	/** 행동 제한과 최대치 도달 여부를 먼저 확인하고 마지막 소비 이후 회복 지연 시간이 지났는지 판단한다. */
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
