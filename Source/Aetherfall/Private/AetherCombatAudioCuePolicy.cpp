/** 체력 위험과 게이지 임계값 통과를 한 번씩 알리기 위한 상태를 관리한다. 실제 사운드 재생은 호출자에게 맡긴다. */
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

/** 회복 후 임계값을 벗어난 위험 단계의 재생 기록을 해제해 다음 위험 상황에서 다시 알릴 수 있게 한다. */
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

/** 사망, 치명적 체력, 낮은 체력 순으로 필요한 알림을 고르고 중복 재생 방지 상태를 갱신한다. */
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

/** 이전·현재 게이지를 비교해 임계값을 새로 넘었을 때만 알린다. 최대치와 참격 준비가 겹치면 최대치 알림을 우선한다. */
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

/** 소비 후 해당 임계값 아래로 내려간 알림만 다시 재생 가능한 상태로 돌린다. */
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

/** 최소·최대 피치를 정렬하고 하한을 적용한 범위에서 무작위 값을 고른다. */
float FAetherCombatAudioCuePolicy::GetRandomizedPitch(float MinPitch, float MaxPitch)
{
	const float ClampedMinPitch = FMath::Max(0.1f, FMath::Min(MinPitch, MaxPitch));
	const float ClampedMaxPitch = FMath::Max(0.1f, FMath::Max(MinPitch, MaxPitch));
	return FMath::FRandRange(ClampedMinPitch, ClampedMaxPitch);
}
