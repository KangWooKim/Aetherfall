#pragma once

#include "CoreMinimal.h"

/** 행동 정책이 계산한 지속 시간과 판정 지연을 실제 타이머 설정에 전달한다. */
struct FAetherCombatActionExecutionPlan
{
	float Duration = 0.1f;
	float ImpactDelay = 0.0f;
	bool bWaitForImpactNotify = false;
	bool bApplyImpactImmediately = false;
	bool bScheduleImpactTimer = false;
};

/** 행동 지속 시간과 타격 시점을 계산하는 월드 비의존 정책이다. 즉시 타격, 애니메이션 알림 대기, 타이머 예약 여부만 반환한다. */
class AETHERFALL_API FAetherCombatActionExecutionPolicy
{
public:
	/** 약공격은 알림을 사용할 때 판정을 기다리고, 사용하지 않으면 즉시 판정하도록 계획한다. */
	static FAetherCombatActionExecutionPlan BuildLightAttackPlan(float Duration, bool bUseImpactNotify);
	/** 강공격의 판정 지연을 행동 시간 안으로 제한한다. 알림을 사용하지 않을 때만 판정 타이머를 예약한다. */
	static FAetherCombatActionExecutionPlan BuildHeavyAttackPlan(float Duration, float ImpactDelay, bool bUseImpactNotify);
	static FAetherCombatActionExecutionPlan BuildAetherSlashPlan(float Duration, float ImpactDelay, bool bUseImpactNotify);
	static FAetherCombatActionExecutionPlan BuildAutoParryCounterPlan(float Duration, float ImpactDelay, bool bUseImpactNotify);
	/** 처형은 설정 시간과 몽타주 길이를 함께 고려한다. 알림 경로에서도 정규화된 시점의 대체 판정을 준비하므로 호출자는 중복 타격을 막아야 한다. */
	static FAetherCombatActionExecutionPlan BuildExecutionPlan(
		float ConfiguredDuration,
		float SelectedMontageLength,
		bool bUseImpactNotify,
		float ImpactFallbackNormalizedTime);

private:
	static float ClampDuration(float Duration);
	static float ClampImpactDelay(float ImpactDelay, float Duration);
};
