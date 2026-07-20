#include "AetherCombatAudioCuePolicy.h"

#include "AetherHealthComponent.h"

namespace
{
float GetSafeHealthPercent(const UAetherHealthComponent* HealthComponent)
{
	if (!HealthComponent)
	{
		return 1.0f;
	}

	const float SafeMaxHealth = FMath::Max(HealthComponent->GetMaxHealth(), 1.0f);
	return FMath::Clamp(HealthComponent->GetCurrentHealth() / SafeMaxHealth, 0.0f, 1.0f);
}

float GetLowHealthThreshold(const FAetherPlayerDangerCueConfig& Config)
{
	return FMath::Clamp(Config.LowHealthThresholdPercent, 0.0f, 1.0f);
}

float GetCriticalHealthThreshold(const FAetherPlayerDangerCueConfig& Config)
{
	return FMath::Clamp(Config.CriticalHealthThresholdPercent, 0.0f, GetLowHealthThreshold(Config));
}
}

void FAetherCombatAudioCuePolicy::RefreshPlayerDangerStateAfterHeal(
	const UAetherHealthComponent* HealthComponent,
	const FAetherPlayerDangerCueConfig& Config,
	FAetherPlayerDangerCueState& State)
{
	if (!Config.bEnabled || !HealthComponent)
	{
		return;
	}

	const float HealthPercent = GetSafeHealthPercent(HealthComponent);
	if (HealthPercent > GetLowHealthThreshold(Config))
	{
		State.bLowHealthWarningPlayed = false;
	}

	if (HealthPercent > GetCriticalHealthThreshold(Config))
	{
		State.bCriticalHealthWarningPlayed = false;
	}
}

EAetherPlayerDangerCue FAetherCombatAudioCuePolicy::EvaluatePlayerDangerCue(
	const UAetherHealthComponent* HealthComponent,
	const FAetherPlayerDangerCueConfig& Config,
	FAetherPlayerDangerCueState& State)
{
	if (!Config.bEnabled || !HealthComponent)
	{
		return EAetherPlayerDangerCue::None;
	}

	if (HealthComponent->IsDead())
	{
		if (!State.bDefeatedWarningPlayed)
		{
			State.bDefeatedWarningPlayed = true;
			State.bCriticalHealthWarningPlayed = true;
			State.bLowHealthWarningPlayed = true;
			return EAetherPlayerDangerCue::Defeated;
		}

		return EAetherPlayerDangerCue::None;
	}

	const float HealthPercent = GetSafeHealthPercent(HealthComponent);
	if (HealthPercent <= GetCriticalHealthThreshold(Config))
	{
		if (!State.bCriticalHealthWarningPlayed)
		{
			State.bCriticalHealthWarningPlayed = true;
			State.bLowHealthWarningPlayed = true;
			return EAetherPlayerDangerCue::CriticalHealth;
		}

		return EAetherPlayerDangerCue::None;
	}

	if (HealthPercent <= GetLowHealthThreshold(Config) && !State.bLowHealthWarningPlayed)
	{
		State.bLowHealthWarningPlayed = true;
		return EAetherPlayerDangerCue::LowHealth;
	}

	return EAetherPlayerDangerCue::None;
}

void FAetherCombatAudioCuePolicy::ResetPlayerDangerState(FAetherPlayerDangerCueState& State)
{
	State = FAetherPlayerDangerCueState();
}

EAetherResourceCue FAetherCombatAudioCuePolicy::EvaluateResourceCue(
	float PreviousAetherGauge,
	float CurrentAetherGauge,
	const FAetherResourceCueConfig& Config,
	FAetherResourceCueState& State)
{
	if (!Config.bEnabled || Config.MaxAetherGauge <= 0.0f)
	{
		return EAetherResourceCue::None;
	}

	const bool bReachedFull = CurrentAetherGauge >= Config.MaxAetherGauge && PreviousAetherGauge < Config.MaxAetherGauge;
	const bool bReachedSlashReady = CurrentAetherGauge >= Config.AetherSlashCost && PreviousAetherGauge < Config.AetherSlashCost;

	if (bReachedFull && !State.bAetherGaugeFullSoundPlayed)
	{
		State.bAetherGaugeFullSoundPlayed = true;
		State.bAetherSlashReadySoundPlayed = true;
		return EAetherResourceCue::AetherGaugeFull;
	}

	if (bReachedSlashReady && !State.bAetherSlashReadySoundPlayed)
	{
		State.bAetherSlashReadySoundPlayed = true;
		return EAetherResourceCue::AetherSlashReady;
	}

	return EAetherResourceCue::None;
}

void FAetherCombatAudioCuePolicy::RefreshResourceStateAfterSpend(
	float CurrentAetherGauge,
	const FAetherResourceCueConfig& Config,
	FAetherResourceCueState& State)
{
	if (CurrentAetherGauge < Config.AetherSlashCost)
	{
		State.bAetherSlashReadySoundPlayed = false;
	}

	if (CurrentAetherGauge < Config.MaxAetherGauge)
	{
		State.bAetherGaugeFullSoundPlayed = false;
	}
}

void FAetherCombatAudioCuePolicy::ResetResourceState(FAetherResourceCueState& State)
{
	State = FAetherResourceCueState();
}

float FAetherCombatAudioCuePolicy::GetRandomizedVolume(float BaseVolume, float Variance)
{
	const float ClampedBaseVolume = FMath::Max(0.0f, BaseVolume);
	const float ClampedVariance = FMath::Clamp(Variance, 0.0f, 1.0f);
	if (ClampedBaseVolume <= 0.0f || ClampedVariance <= 0.0f)
	{
		return ClampedBaseVolume;
	}

	return ClampedBaseVolume * FMath::FRandRange(1.0f - ClampedVariance, 1.0f + ClampedVariance);
}

float FAetherCombatAudioCuePolicy::GetRandomizedPitch(float MinPitch, float MaxPitch)
{
	const float ClampedMinPitch = FMath::Max(0.1f, FMath::Min(MinPitch, MaxPitch));
	const float ClampedMaxPitch = FMath::Max(0.1f, FMath::Max(MinPitch, MaxPitch));
	return FMath::FRandRange(ClampedMinPitch, ClampedMaxPitch);
}
