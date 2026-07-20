#pragma once

#include "CoreMinimal.h"

class AAetherGameModeBase;
class AAetherfallCharacter;

struct FAetherPrototypeRouteGuidanceViewData
{
	FString ObjectiveLabel;
	FString TutorialHintLabel;
	bool bCriticalObjective = false;

	bool HasGuidance() const
	{
		return !ObjectiveLabel.IsEmpty() || !TutorialHintLabel.IsEmpty();
	}
};

class AETHERFALL_API FAetherPrototypeRouteGuide
{
public:
	static FAetherPrototypeRouteGuidanceViewData BuildGuidance(
		const AAetherGameModeBase* AetherGameMode,
		const AAetherfallCharacter* PlayerCharacter);

	static FString BuildObjectiveLabel(const AAetherGameModeBase* AetherGameMode);
	static FString BuildTutorialHintLabel(
		const AAetherGameModeBase* AetherGameMode,
		const AAetherfallCharacter* PlayerCharacter);
	static bool IsCriticalObjective(const AAetherGameModeBase* AetherGameMode);
};
