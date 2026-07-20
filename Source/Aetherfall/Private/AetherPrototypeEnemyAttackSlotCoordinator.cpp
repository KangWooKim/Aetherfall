#include "AetherPrototypeEnemyAttackSlotCoordinator.h"

#include "AetherEnemyBase.h"
#include "AetherHealthComponent.h"

void FAetherPrototypeEnemyAttackSlotCoordinator::Reset()
{
	CurrentAttackingEnemy.Reset();
	AttackDelayUntil = -100.0;
}

bool FAetherPrototypeEnemyAttackSlotCoordinator::TryAcquire(
	AAetherEnemyBase* RequestingEnemy,
	double CurrentTimeSeconds,
	bool bCoordinateAttacks)
{
	if (!bCoordinateAttacks || !RequestingEnemy)
	{
		return true;
	}

	AAetherEnemyBase* CurrentAttacker = CurrentAttackingEnemy.Get();
	const UAetherHealthComponent* CurrentAttackerHealth = CurrentAttacker ? CurrentAttacker->GetHealthComponent() : nullptr;
	if (!CurrentAttacker || !CurrentAttackerHealth || CurrentAttackerHealth->IsDead())
	{
		CurrentAttackingEnemy.Reset();
	}

	if (CurrentTimeSeconds < AttackDelayUntil && CurrentAttackingEnemy.Get() != RequestingEnemy)
	{
		return false;
	}

	if (!CurrentAttackingEnemy.IsValid() || CurrentAttackingEnemy.Get() == RequestingEnemy)
	{
		CurrentAttackingEnemy = RequestingEnemy;
		return true;
	}

	return false;
}

void FAetherPrototypeEnemyAttackSlotCoordinator::Release(AAetherEnemyBase* ReleasingEnemy, bool bCoordinateAttacks)
{
	if (!bCoordinateAttacks || !ReleasingEnemy)
	{
		return;
	}

	if (CurrentAttackingEnemy.Get() == ReleasingEnemy)
	{
		CurrentAttackingEnemy.Reset();
	}
}

bool FAetherPrototypeEnemyAttackSlotCoordinator::DelayAttacks(
	double CurrentTimeSeconds,
	float DelayDuration,
	bool bCoordinateAttacks)
{
	if (!bCoordinateAttacks || DelayDuration <= 0.0f)
	{
		return false;
	}

	AttackDelayUntil = FMath::Max(AttackDelayUntil, CurrentTimeSeconds + DelayDuration);
	return true;
}
