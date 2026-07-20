#pragma once

#include "CoreMinimal.h"

class AActor;
class UAetherHealthComponent;

enum class EAetherCombatDamageFailureReason : uint8
{
	None,
	InvalidTarget,
	SelfDamage,
	MissingHealthComponent,
	TargetDead,
	RejectedByHealthComponent
};

struct FAetherCombatDamageRequest
{
	AActor* TargetActor = nullptr;
	AActor* DamageCauser = nullptr;
	float DamageAmount = 0.0f;
};

struct FAetherCombatDamageResult
{
	bool bApplied = false;
	AActor* TargetActor = nullptr;
	UAetherHealthComponent* HealthComponent = nullptr;
	float DamageAmount = 0.0f;
	EAetherCombatDamageFailureReason FailureReason = EAetherCombatDamageFailureReason::None;

	bool WasApplied() const { return bApplied; }
};

class AETHERFALL_API FAetherCombatDamagePolicy
{
public:
	static FAetherCombatDamageResult TryApplyDamage(const FAetherCombatDamageRequest& Request);
};
