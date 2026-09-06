#pragma once

#include "CoreMinimal.h"
#include "AetherCombatActionGatePolicy.h"

/** 서로 구분해야 하는 공격·방어·회복 상태의 전환 모드다. */
enum class EAetherCombatActionMode : uint8
{
	Idle,
	LightAttack,
	HeavyAttack,
	AetherSlash,
	Execution,
	Dodge,
	Guard,
	ParryWindow,
	ParryRecovery,
	ParryCounterWindow,
	HitReaction
};

/** 전환 모드에 대응하는 전투 플래그 묶음이다. 컴포넌트가 실제 상태에 적용한다. */
struct FAetherCombatActionRuntimeFlags
{
	bool bAttacking = false;
	bool bHeavyAttacking = false;
	bool bExecuting = false;
	bool bAetherSlashing = false;
	bool bDodging = false;
	bool bGuarding = false;
	bool bParryWindowActive = false;
	bool bParryRecovering = false;
	bool bParryCounterWindowActive = false;
	bool bHitReacting = false;
};

/** 전투 행동 모드를 기존 실행 플래그와 판정용 스냅샷으로 변환한다. 여러 플래그가 함께 켜지면 정해진 우선순위로 대표 모드를 해석한다. */
class AETHERFALL_API FAetherCombatActionStatePolicy
{
public:
	/** 새 플래그 묶음을 기본값에서 구성한다. 강공격과 참격은 공통 공격 플래그도 함께 켜진다. */
	static FAetherCombatActionRuntimeFlags BuildFlagsForMode(EAetherCombatActionMode Mode);
	/** 실행 플래그를 복사하고 소유자의 사망 여부를 추가해 행동 판정의 입력으로 만든다. */
	static FAetherCombatActionStateSnapshot BuildSnapshot(bool bOwnerDead, const FAetherCombatActionRuntimeFlags& RuntimeFlags);
	/** 피격·처형·참격 등의 우선순위로 현재 모드를 해석한다. 결과를 조회할 뿐 입력 플래그를 수정하지 않는다. */
	static EAetherCombatActionMode ResolveDominantMode(const FAetherCombatActionStateSnapshot& State);
	static bool IsAttackMode(EAetherCombatActionMode Mode);
	static bool IsInterruptMode(EAetherCombatActionMode Mode);
};
