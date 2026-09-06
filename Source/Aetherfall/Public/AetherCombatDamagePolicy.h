#pragma once

#include "CoreMinimal.h"

class AActor;
class UAetherHealthComponent;

/** 피해 적용에 실패한 사전 조건을 호출자가 구분할 수 있게 한다. */
enum class EAetherCombatDamageFailureReason : uint8
{
	None,
	InvalidTarget,
	SelfDamage,
	MissingHealthComponent,
	TargetDead,
	RejectedByHealthComponent
};

/** 대상·피해 원인·피해량을 공통 피해 진입점에 전달한다. */
struct FAetherCombatDamageRequest
{
	AActor* TargetActor = nullptr;
	AActor* DamageCauser = nullptr;
	float DamageAmount = 0.0f;
};

/** 피해 적용 여부와 실패 사유를 반환해 보상 및 연출의 후속 실행을 판단한다. */
struct FAetherCombatDamageResult
{
	bool bApplied = false;
	AActor* TargetActor = nullptr;
	UAetherHealthComponent* HealthComponent = nullptr;
	float DamageAmount = 0.0f;
	EAetherCombatDamageFailureReason FailureReason = EAetherCombatDamageFailureReason::None;

	bool WasApplied() const { return bApplied; }
};

/** 대상 유효성, 자가 피해, 체력 컴포넌트와 사망 여부를 확인하고 피해 적용 결과와 거부 사유를 반환한다. */
class AETHERFALL_API FAetherCombatDamagePolicy
{
public:
	/** 음수 피해를 0으로 제한하고 공통 사전 조건을 통과한 대상의 체력 컴포넌트에 피해를 위임한다. */
	static FAetherCombatDamageResult TryApplyDamage(const FAetherCombatDamageRequest& Request);
};
