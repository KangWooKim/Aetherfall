#include "AetherCombatHudPresenter.h"

#include "AetherCombatComponent.h"
#include "AetherGameModeBase.h"
#include "AetherHealthComponent.h"

FString FAetherCombatHudPresenter::BuildCombatStateLabel(
	const UAetherCombatComponent* CombatComponent,
	const UAetherHealthComponent* HealthComponent,
	const AAetherGameModeBase* AetherGameMode)
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		if (AetherGameMode && AetherGameMode->IsPrototypeDefeatRetryScheduled())
		{
			return FString::Printf(
				TEXT("STATE DEFEATED / AUTO RETRY %.1f"),
				AetherGameMode->GetPrototypeDefeatRetryRemainingTime());
		}

		return TEXT("STATE DEFEATED / PRESS Y RESET");
	}

	if (!CombatComponent)
	{
		return TEXT("STATE UNKNOWN");
	}

	if (CombatComponent->IsHitReacting())
	{
		return TEXT("STATE HIT REACTION");
	}

	if (CombatComponent->IsDamageInvulnerable())
	{
		return TEXT("STATE HIT RECOVERY");
	}

	if (CombatComponent->IsExecuting())
	{
		return TEXT("STATE EXECUTION");
	}

	if (CombatComponent->IsAetherSlashing())
	{
		return TEXT("STATE AETHER SLASH");
	}

	if (CombatComponent->IsGuarding())
	{
		return TEXT("STATE GUARD");
	}

	if (CombatComponent->IsParryCounterWindowActive())
	{
		return TEXT("STATE PARRY COUNTER");
	}

	if (CombatComponent->IsParryWindowActive())
	{
		return TEXT("STATE PARRY WINDOW");
	}

	if (CombatComponent->IsDodging())
	{
		return TEXT("STATE DODGE");
	}

	if (CombatComponent->IsHeavyAttacking())
	{
		return TEXT("STATE HEAVY ATTACK");
	}

	if (CombatComponent->IsAttacking())
	{
		return FString::Printf(TEXT("STATE ATTACK %d"), CombatComponent->GetCurrentComboStep());
	}

	return TEXT("STATE READY");
}

FString FAetherCombatHudPresenter::BuildAetherSlashStatusLabel(const UAetherCombatComponent* CombatComponent)
{
	if (!CombatComponent)
	{
		return TEXT("AE SLASH UNKNOWN");
	}

	const float CooldownRemaining = CombatComponent->GetAetherSlashCooldownRemaining();
	if (CooldownRemaining > 0.0f)
	{
		return FString::Printf(TEXT("AE SLASH CD %.1f"), CooldownRemaining);
	}

	const float RequiredAether = CombatComponent->GetAetherSlashCost();
	const float MissingAether = RequiredAether - CombatComponent->GetCurrentAetherGauge();
	if (MissingAether > 0.0f)
	{
		return FString::Printf(TEXT("AE SLASH NEED %.0f"), MissingAether);
	}

	return CombatComponent->IsAetherSlashReady() ? TEXT("AE SLASH READY [X]") : TEXT("AE SLASH READY");
}

FAetherPlayerDangerViewData FAetherCombatHudPresenter::BuildPlayerDangerViewData(
	const UAetherHealthComponent* HealthComponent,
	const AAetherGameModeBase* AetherGameMode)
{
	FAetherPlayerDangerViewData ViewData;
	if (!HealthComponent || (AetherGameMode && AetherGameMode->IsPrototypeCombatRoundComplete()))
	{
		return ViewData;
	}

	const float SafeMaxHealth = FMath::Max(HealthComponent->GetMaxHealth(), 1.0f);
	const float HealthPercent = FMath::Clamp(HealthComponent->GetCurrentHealth() / SafeMaxHealth, 0.0f, 1.0f);

	if (HealthComponent->IsDead())
	{
		ViewData.Label = AetherGameMode && AetherGameMode->IsPrototypeDefeatRetryScheduled()
			? FString::Printf(TEXT("DEFEATED - RETRY IN %.1f"), AetherGameMode->GetPrototypeDefeatRetryRemainingTime())
			: TEXT("DEFEATED - PRESS Y TO RESET");
		ViewData.Color = FLinearColor(0.86f, 0.02f, 0.01f, 1.0f);
		ViewData.bShouldDisplay = true;
		return ViewData;
	}

	if (HealthPercent <= 0.25f)
	{
		ViewData.Label = TEXT("CRITICAL HP");
		ViewData.Color = FLinearColor(1.0f, 0.06f, 0.02f, 1.0f);
		ViewData.bShouldDisplay = true;
		return ViewData;
	}

	if (HealthPercent <= 0.45f)
	{
		ViewData.Label = TEXT("LOW HP");
		ViewData.Color = FLinearColor(1.0f, 0.42f, 0.05f, 1.0f);
		ViewData.bShouldDisplay = true;
	}

	return ViewData;
}
