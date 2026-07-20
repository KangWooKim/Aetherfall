#pragma once

#include "CoreMinimal.h"

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

class AETHERFALL_API FAetherCombatResourcePolicy
{
public:
	static FAetherCombatResourceMutationPlan BuildInitializePlan(float MaxStamina);
	static FAetherCombatResourceMutationPlan BuildResetPlan(float MaxStamina, double CurrentTime);
	static FAetherCombatResourceMutationPlan BuildStaminaSpendPlan(float CurrentStamina, float StaminaCost, double CurrentTime);
	static FAetherCombatResourceMutationPlan BuildStaminaTimestampPlan(double CurrentTime);
	static FAetherCombatResourceMutationPlan BuildStaminaRegenPlan(float CurrentStamina, float MaxStamina, float RegenPerSecond, float DeltaTime);
	static FAetherCombatResourceMutationPlan BuildAetherGainPlan(float CurrentAetherGauge, float MaxAetherGauge, float Amount);
	static FAetherCombatResourceMutationPlan BuildAetherSpendPlan(float CurrentAetherGauge, float Amount);
};
