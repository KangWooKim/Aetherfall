#pragma once

#include "CoreMinimal.h"

enum class EAetherEnemyArchetype : uint8;
struct FAetherEnemyAttackPatternData;

struct FAetherAurelBossPhaseTuning
{
	float PhaseTwoHealthThresholdPercent = 0.5f;
	float PhaseShiftDuration = 1.2f;
	float PhaseShiftAetherReward = 20.0f;
	float PhaseTwoAttackCooldownMultiplier = 0.78f;
	float PhaseTwoMovementSpeed = 255.0f;
	float LowHealthWarningPercent = 0.15f;
};

struct FAetherAurelBossPhaseFeedback
{
	bool bShouldAnnounce = false;
	FString DebugMessage;
	FString ProgressMessage;
	FLinearColor ProgressColor = FLinearColor::White;
};

struct FAetherAurelBossPhaseTransitionPlan
{
	bool bShouldStartPhaseTwo = false;
	float PhaseShiftDuration = 0.0f;
	float AetherReward = 0.0f;
	float AttackCooldownMultiplier = 1.0f;
	float PhaseTwoMovementSpeed = 0.0f;
	FAetherAurelBossPhaseFeedback Feedback;
};

class AETHERFALL_API FAetherAurelBossPhasePolicy
{
public:
	void Reset();

	bool IsPhaseOne(EAetherEnemyArchetype EnemyArchetype) const;
	bool IsPhaseTwo(EAetherEnemyArchetype EnemyArchetype) const;
	bool IsPhaseShifting() const;

	FAetherAurelBossPhaseFeedback EvaluatePhaseOneIntro(EAetherEnemyArchetype EnemyArchetype);
	FAetherAurelBossPhaseTransitionPlan EvaluatePhaseTwoTransition(
		EAetherEnemyArchetype EnemyArchetype,
		float CurrentHealth,
		float MaxHealth,
		const FAetherAurelBossPhaseTuning& Tuning) const;
	FAetherAurelBossPhaseFeedback EvaluateLowHealth(
		EAetherEnemyArchetype EnemyArchetype,
		float CurrentHealth,
		float MaxHealth,
		const FAetherAurelBossPhaseTuning& Tuning);

	void BeginPhaseTwoTransition();
	void EndPhaseShift();

	const FAetherEnemyAttackPatternData* SelectAurelAttackPattern(
		const TArray<FAetherEnemyAttackPatternData>& AttackPatterns,
		float DistanceToTarget,
		float FallbackAttackRange,
		float AttackStartRangeBuffer,
		const FAetherEnemyAttackPatternData& FallbackAttackPattern);

private:
	bool bPhaseOneIntroAnnounced = false;
	bool bPhaseTwoActive = false;
	bool bPhaseShifting = false;
	bool bLowHealthWarningAnnounced = false;
	int32 NextPhaseOneAttackIndex = 0;
	int32 NextPhaseTwoAttackIndex = 0;
};
