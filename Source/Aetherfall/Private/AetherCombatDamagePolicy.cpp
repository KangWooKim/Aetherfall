/** 대상 유효성, 자가 피해, 체력 컴포넌트와 사망 여부를 확인하고 피해 적용 결과와 거부 사유를 반환한다. */
#include "AetherCombatDamagePolicy.h"

#include "AetherHealthComponent.h"
#include "GameFramework/Actor.h"

/** 음수 피해를 0으로 제한하고 공통 사전 조건을 통과한 대상의 체력 컴포넌트에 피해를 위임한다. */
FAetherCombatDamageResult FAetherCombatDamagePolicy::TryApplyDamage(const FAetherCombatDamageRequest& Request)
{
	FAetherCombatDamageResult Result;
	Result.TargetActor = Request.TargetActor;
	Result.DamageAmount = FMath::Max(0.0f, Request.DamageAmount);

	if (!Request.TargetActor)
	{
		Result.FailureReason = EAetherCombatDamageFailureReason::InvalidTarget;
		return Result;
	}

	if (Request.TargetActor == Request.DamageCauser)
	{
		Result.FailureReason = EAetherCombatDamageFailureReason::SelfDamage;
		return Result;
	}

	UAetherHealthComponent* HealthComponent = Request.TargetActor->FindComponentByClass<UAetherHealthComponent>();
	Result.HealthComponent = HealthComponent;
	if (!HealthComponent)
	{
		Result.FailureReason = EAetherCombatDamageFailureReason::MissingHealthComponent;
		return Result;
	}

	if (HealthComponent->IsDead())
	{
		Result.FailureReason = EAetherCombatDamageFailureReason::TargetDead;
		return Result;
	}

	if (!HealthComponent->ApplyDamage(Result.DamageAmount, Request.DamageCauser))
	{
		Result.FailureReason = EAetherCombatDamageFailureReason::RejectedByHealthComponent;
		return Result;
	}

	Result.bApplied = true;
	Result.FailureReason = EAetherCombatDamageFailureReason::None;
	return Result;
}
