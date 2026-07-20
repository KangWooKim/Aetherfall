#include "AetherCombatDamagePolicy.h"

#include "AetherHealthComponent.h"
#include "GameFramework/Actor.h"

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
