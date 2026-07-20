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
