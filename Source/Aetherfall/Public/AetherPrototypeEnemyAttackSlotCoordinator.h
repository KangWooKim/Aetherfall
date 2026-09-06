#pragma once

#include "CoreMinimal.h"

class AAetherEnemyBase;

/** 선택적 단일 공격 슬롯과 공격 지연 시각을 관리하여 여러 적의 공격 시작을 조정한다. */
class AETHERFALL_API FAetherPrototypeEnemyAttackSlotCoordinator
{
public:
	void Reset();
	/** 기존 공격자의 생존을 정리한 뒤 지연 시간과 슬롯 소유를 확인한다. 조정 비활성 시에는 요청을 통과시킨다. */
	bool TryAcquire(AAetherEnemyBase* RequestingEnemy, double CurrentTimeSeconds, bool bCoordinateAttacks);
	/** 현재 슬롯을 가진 적의 반환 요청만 반영하여 다른 적의 소유권을 지우지 않는다. */
	void Release(AAetherEnemyBase* ReleasingEnemy, bool bCoordinateAttacks);
	/** 기존 지연 종료 시각보다 짧아지지 않도록 새로운 공격 대기 시간을 연장한다. */
	bool DelayAttacks(double CurrentTimeSeconds, float DelayDuration, bool bCoordinateAttacks);

private:
	TWeakObjectPtr<AAetherEnemyBase> CurrentAttackingEnemy;
	double AttackDelayUntil = -100.0;
};
