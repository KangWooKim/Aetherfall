#pragma once

#include "CoreMinimal.h"

class UAetherHealthComponent;

enum class EAetherPlayerDangerCue : uint8
{
	None,
	LowHealth,
	CriticalHealth,
	Defeated
};

enum class EAetherResourceCue : uint8
{
	None,
	AetherSlashReady,
	AetherGaugeFull
};

struct FAetherPlayerDangerCueConfig
{
	bool bEnabled = true;
	float LowHealthThresholdPercent = 0.45f;
	float CriticalHealthThresholdPercent = 0.25f;
};

struct FAetherPlayerDangerCueState
{
	bool bLowHealthWarningPlayed = false;
	bool bCriticalHealthWarningPlayed = false;
	bool bDefeatedWarningPlayed = false;
};

struct FAetherResourceCueConfig
{
	bool bEnabled = true;
	float MaxAetherGauge = 100.0f;
	float AetherSlashCost = 35.0f;
};

struct FAetherResourceCueState
{
	bool bAetherSlashReadySoundPlayed = false;
	bool bAetherGaugeFullSoundPlayed = false;
};

class AETHERFALL_API FAetherCombatAudioCuePolicy
{
public:
	static void RefreshPlayerDangerStateAfterHeal(
		const UAetherHealthComponent* HealthComponent,
		const FAetherPlayerDangerCueConfig& Config,
		FAetherPlayerDangerCueState& State);

	static EAetherPlayerDangerCue EvaluatePlayerDangerCue(
		const UAetherHealthComponent* HealthComponent,
		const FAetherPlayerDangerCueConfig& Config,
		FAetherPlayerDangerCueState& State);

	static void ResetPlayerDangerState(FAetherPlayerDangerCueState& State);

	static EAetherResourceCue EvaluateResourceCue(
		float PreviousAetherGauge,
		float CurrentAetherGauge,
		const FAetherResourceCueConfig& Config,
		FAetherResourceCueState& State);

	static void RefreshResourceStateAfterSpend(
		float CurrentAetherGauge,
		const FAetherResourceCueConfig& Config,
		FAetherResourceCueState& State);

	static void ResetResourceState(FAetherResourceCueState& State);

	static float GetRandomizedVolume(float BaseVolume, float Variance);
	static float GetRandomizedPitch(float MinPitch, float MaxPitch);
};
