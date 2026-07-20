#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"

class AActor;

enum class EAetherCombatTargetPriority : uint8
{
	None,
	Preferred,
	Locked
};

struct FAetherCombatTargetSelection
{
	AActor* PriorityTarget = nullptr;
	EAetherCombatTargetPriority Priority = EAetherCombatTargetPriority::None;
	TArray<AActor*> AdditionalTargets;

	bool HasPriorityTarget() const { return PriorityTarget != nullptr; }
	bool IsPreferredPriority() const { return Priority == EAetherCombatTargetPriority::Preferred; }
	bool IsLockedPriority() const { return Priority == EAetherCombatTargetPriority::Locked; }
};

class AETHERFALL_API FAetherCombatTargetSelectionPolicy
{
public:
	static FAetherCombatTargetSelection BuildTargetSelection(
		const TArray<FHitResult>& HitResults,
		AActor* DamageCauser,
		AActor* PreferredTarget,
		AActor* LockedTarget);

	static AActor* FindTargetInHits(const TArray<FHitResult>& HitResults, AActor* Target);

private:
	static bool IsValidDamageTarget(AActor* Candidate, AActor* DamageCauser);
	static void AppendUniqueHitTargets(
		const TArray<FHitResult>& HitResults,
		AActor* DamageCauser,
		TArray<AActor*>& OutTargets);
};
