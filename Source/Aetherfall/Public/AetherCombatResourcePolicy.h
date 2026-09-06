#pragma once

#include "CoreMinimal.h"

/** 자원 변화의 허용 여부와 변경 후 값을 계산한 결과다. 실제 컴포넌트 쓰기는 호출자가 수행한다. */
struct FAetherCombatResourceMutationPlan
{
	bool bCanApply = true;

	bool bUpdateStamina = false;
	float PreviousStamina = 0.0f;
	float NewStamina = 0.0f;
	float StaminaDelta = 0.0f;

	bool bRecordStaminaSpendTime = false;
	double NewLastStaminaSpendTime = 0.0;

	bool bUpdateAetherGauge = false;
	float PreviousAetherGauge = 0.0f;
	float NewAetherGauge = 0.0f;
	float AetherGaugeDelta = 0.0f;
};

/** 스태미나와 에테르 게이지의 변경값 및 소비 시각을 계산하여, 컴포넌트가 적용할 변경 계획으로 반환한다. */
class AETHERFALL_API FAetherCombatResourcePolicy
{
public:
	static FAetherCombatResourceMutationPlan BuildInitializePlan(float MaxStamina);
	/** 스태미나를 최대값으로, 게이지를 0으로 초기화하고 회복 지연의 기준 시각을 기록한다. */
	static FAetherCombatResourceMutationPlan BuildResetPlan(float MaxStamina, double CurrentTime);
	/** 비용을 0 이상으로 제한해 차감값과 소비 시각을 계산한다. 잔량 부족 여부는 호출 전 행동 허용 판정에서 확인한다. */
	static FAetherCombatResourceMutationPlan BuildStaminaSpendPlan(float CurrentStamina, float StaminaCost, double CurrentTime);
	static FAetherCombatResourceMutationPlan BuildStaminaTimestampPlan(double CurrentTime);
	/** 음수 회복량과 경과 시간을 배제하고 최대 스태미나를 넘지 않도록 회복 계획을 만든다. */
	static FAetherCombatResourceMutationPlan BuildStaminaRegenPlan(float CurrentStamina, float MaxStamina, float RegenPerSecond, float DeltaTime);
	static FAetherCombatResourceMutationPlan BuildAetherGainPlan(float CurrentAetherGauge, float MaxAetherGauge, float Amount);
	/** 게이지가 비용보다 부족하면 적용 불가를 반환하여 자원 변경을 막는다. */
	static FAetherCombatResourceMutationPlan BuildAetherSpendPlan(float CurrentAetherGauge, float Amount);
};
