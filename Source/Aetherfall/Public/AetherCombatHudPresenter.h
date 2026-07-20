#pragma once

#include "CoreMinimal.h"

class AAetherGameModeBase;
class UAetherCombatComponent;
class UAetherHealthComponent;

struct FAetherPlayerDangerViewData
{
	bool bShouldDisplay = false;
	FString Label;
	FLinearColor Color = FLinearColor::White;
};

class AETHERFALL_API FAetherCombatHudPresenter
{
public:
	static FString BuildCombatStateLabel(
		const UAetherCombatComponent* CombatComponent,
		const UAetherHealthComponent* HealthComponent,
		const AAetherGameModeBase* AetherGameMode);

	static FString BuildAetherSlashStatusLabel(const UAetherCombatComponent* CombatComponent);

	static FAetherPlayerDangerViewData BuildPlayerDangerViewData(
		const UAetherHealthComponent* HealthComponent,
		const AAetherGameModeBase* AetherGameMode);
};
