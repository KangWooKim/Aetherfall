#include "AetherCombatTargetSelectionPolicy.h"

#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"

FAetherCombatTargetSelection FAetherCombatTargetSelectionPolicy::BuildTargetSelection(
	const TArray<FHitResult>& HitResults,
	AActor* DamageCauser,
	AActor* PreferredTarget,
	AActor* LockedTarget)
{
	FAetherCombatTargetSelection Selection;

	if (AActor* PreferredTargetInHits = FindTargetInHits(HitResults, PreferredTarget))
	{
		if (IsValidDamageTarget(PreferredTargetInHits, DamageCauser))
		{
			Selection.PriorityTarget = PreferredTargetInHits;
			Selection.Priority = EAetherCombatTargetPriority::Preferred;
			return Selection;
		}
	}

	if (AActor* LockedTargetInHits = FindTargetInHits(HitResults, LockedTarget))
	{
		if (IsValidDamageTarget(LockedTargetInHits, DamageCauser))
		{
			Selection.PriorityTarget = LockedTargetInHits;
			Selection.Priority = EAetherCombatTargetPriority::Locked;
			return Selection;
		}
	}

	AppendUniqueHitTargets(HitResults, DamageCauser, Selection.AdditionalTargets);
	return Selection;
}

AActor* FAetherCombatTargetSelectionPolicy::FindTargetInHits(const TArray<FHitResult>& HitResults, AActor* Target)
{
	if (!Target)
	{
		return nullptr;
	}

	for (const FHitResult& HitResult : HitResults)
	{
		if (HitResult.GetActor() == Target)
		{
			return Target;
		}
	}

	return nullptr;
}

bool FAetherCombatTargetSelectionPolicy::IsValidDamageTarget(AActor* Candidate, AActor* DamageCauser)
{
	return Candidate != nullptr && Candidate != DamageCauser;
}

void FAetherCombatTargetSelectionPolicy::AppendUniqueHitTargets(
	const TArray<FHitResult>& HitResults,
	AActor* DamageCauser,
	TArray<AActor*>& OutTargets)
{
	OutTargets.Reserve(HitResults.Num());
	for (const FHitResult& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!IsValidDamageTarget(HitActor, DamageCauser) || OutTargets.Contains(HitActor))
		{
			continue;
		}

		OutTargets.Add(HitActor);
	}
}
