#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"

class AActor;

/** 실제 피격 목록 안에서 선호 대상과 잠금 대상의 우선순위를 구분한다. */
enum class EAetherCombatTargetPriority : uint8
{
	None,
	Preferred,
	Locked
};

/** 단일 우선 대상 또는 중복을 제거한 나머지 피해 후보를 전달한다. */
struct FAetherCombatTargetSelection
{
	AActor* PriorityTarget = nullptr;
	EAetherCombatTargetPriority Priority = EAetherCombatTargetPriority::None;
	TArray<AActor*> AdditionalTargets;

	bool HasPriorityTarget() const { return PriorityTarget != nullptr; }
	bool IsPreferredPriority() const { return Priority == EAetherCombatTargetPriority::Preferred; }
	bool IsLockedPriority() const { return Priority == EAetherCombatTargetPriority::Locked; }
};

/** 피격 목록에서 지정 대상, 락온 대상, 중복을 제거한 일반 대상 순으로 공격 대상을 결정한다. */
class AETHERFALL_API FAetherCombatTargetSelectionPolicy
{
public:
	/** 우선 대상도 실제 피격 목록에 포함되어야 하며, 우선 대상이 정해지면 추가 대상은 반환하지 않는다. */
	static FAetherCombatTargetSelection BuildTargetSelection(
		const TArray<FHitResult>& HitResults,
		AActor* DamageCauser,
		AActor* PreferredTarget,
		AActor* LockedTarget);

	static AActor* FindTargetInHits(const TArray<FHitResult>& HitResults, AActor* Target);

private:
	static bool IsValidDamageTarget(AActor* Candidate, AActor* DamageCauser);
	/** 공격자와 빈 대상을 제외하고 동일 액터가 여러 충돌 결과에 잡혀도 한 번만 추가한다. */
	static void AppendUniqueHitTargets(
		const TArray<FHitResult>& HitResults,
		AActor* DamageCauser,
		TArray<AActor*>& OutTargets);
};
