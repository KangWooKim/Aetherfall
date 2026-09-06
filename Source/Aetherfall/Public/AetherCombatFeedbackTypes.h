/** 전투 판정과 연출 선택이 공유하는 타격·방어 사건의 종류를 정의한다. */
#pragma once

#include "CoreMinimal.h"
#include "AetherCombatFeedbackTypes.generated.h"

UENUM(BlueprintType)
enum class EAetherCombatFeedbackType : uint8
{
	LightHit,
	HeavyHit,
	HeavyCounterHit,
	Execution,
	ParrySuccess,
	PlayerHit,
	GuardBlock,
	AetherSlash
};
