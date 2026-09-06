/** 피격 목록에서 지정 대상, 락온 대상, 중복을 제거한 일반 대상 순으로 공격 대상을 결정한다. */
#include "AetherCombatTargetSelectionPolicy.h"

#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"

/** 우선 대상도 실제 피격 목록에 포함되어야 하며, 우선 대상이 정해지면 추가 대상은 반환하지 않는다. */
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

/** 공격자와 빈 대상을 제외하고 동일 액터가 여러 충돌 결과에 잡혀도 한 번만 추가한다. */
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
