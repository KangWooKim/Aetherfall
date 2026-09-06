/** 선택적 단일 공격 슬롯과 공격 지연 시각을 관리하여 여러 적의 공격 시작을 조정한다. */
#include "AetherPrototypeEnemyAttackSlotCoordinator.h"

#include "AetherEnemyBase.h"
#include "AetherHealthComponent.h"

void FAetherPrototypeEnemyAttackSlotCoordinator::Reset()
{
	CurrentAttackingEnemy.Reset();
	AttackDelayUntil = -100.0;
}

/** 기존 공격자의 생존을 정리한 뒤 지연 시간과 슬롯 소유를 확인한다. 조정 비활성 시에는 요청을 통과시킨다. */
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

/** 현재 슬롯을 가진 적의 반환 요청만 반영하여 다른 적의 소유권을 지우지 않는다. */
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

/** 기존 지연 종료 시각보다 짧아지지 않도록 새로운 공격 대기 시간을 연장한다. */
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
