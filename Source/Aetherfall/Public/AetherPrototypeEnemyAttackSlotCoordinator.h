#pragma once

#include "CoreMinimal.h"

class AAetherEnemyBase;

class AETHERFALL_API FAetherPrototypeEnemyAttackSlotCoordinator
{
public:
	void Reset();
	bool TryAcquire(AAetherEnemyBase* RequestingEnemy, double CurrentTimeSeconds, bool bCoordinateAttacks);
	void Release(AAetherEnemyBase* ReleasingEnemy, bool bCoordinateAttacks);
	bool DelayAttacks(double CurrentTimeSeconds, float DelayDuration, bool bCoordinateAttacks);

private:
	TWeakObjectPtr<AAetherEnemyBase> CurrentAttackingEnemy;
	double AttackDelayUntil = -100.0;
};
