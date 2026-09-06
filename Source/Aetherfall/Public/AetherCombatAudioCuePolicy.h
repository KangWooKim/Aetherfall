#pragma once

#include "CoreMinimal.h"

class UAetherHealthComponent;

/** 낮은 체력·임계 체력·패배에 대응하는 알림 종류다. */
enum class EAetherPlayerDangerCue : uint8
{
	None,
	LowHealth,
	CriticalHealth,
	Defeated
};

/** 검기 사용 가능 상태와 게이지 최대 충전을 알리는 소리 종류다. */
enum class EAetherResourceCue : uint8
{
	None,
	AetherSlashReady,
	AetherGaugeFull
};

/** 체력 위험 알림의 사용 여부와 단계별 임계값을 설정한다. */
struct FAetherPlayerDangerCueConfig
{
	bool bEnabled = true;
	float LowHealthThresholdPercent = 0.45f;
	float CriticalHealthThresholdPercent = 0.25f;
};

/** 이미 전달한 위험·패배 안내를 보관해 중복 재생을 제어한다. */
struct FAetherPlayerDangerCueState
{
	bool bLowHealthWarningPlayed = false;
	bool bCriticalHealthWarningPlayed = false;
	bool bDefeatedWarningPlayed = false;
};

/** 검기 사용 가능 상태와 최대 충전 알림에 사용할 게이지 기준을 묶는다. */
struct FAetherResourceCueConfig
{
	bool bEnabled = true;
	float MaxAetherGauge = 100.0f;
	float AetherSlashCost = 35.0f;
};

/** 자원 알림을 다시 재생할 수 있는 상태를 보관한다. */
struct FAetherResourceCueState
{
	bool bAetherSlashReadySoundPlayed = false;
	bool bAetherGaugeFullSoundPlayed = false;
};

/** 체력 위험과 게이지 임계값 통과를 한 번씩 알리기 위한 상태를 관리한다. 실제 사운드 재생은 호출자에게 맡긴다. */
class AETHERFALL_API FAetherCombatAudioCuePolicy
{
public:
	/** 회복 후 임계값을 벗어난 위험 단계의 재생 기록을 해제해 다음 위험 상황에서 다시 알릴 수 있게 한다. */
	static void RefreshPlayerDangerStateAfterHeal(
		const UAetherHealthComponent* HealthComponent,
		const FAetherPlayerDangerCueConfig& Config,
		FAetherPlayerDangerCueState& State);

	/** 사망, 치명적 체력, 낮은 체력 순으로 필요한 알림을 고르고 중복 재생 방지 상태를 갱신한다. */
	static EAetherPlayerDangerCue EvaluatePlayerDangerCue(
		const UAetherHealthComponent* HealthComponent,
		const FAetherPlayerDangerCueConfig& Config,
		FAetherPlayerDangerCueState& State);

	static void ResetPlayerDangerState(FAetherPlayerDangerCueState& State);

	/** 이전·현재 게이지를 비교해 임계값을 새로 넘었을 때만 알린다. 최대치와 참격 준비가 겹치면 최대치 알림을 우선한다. */
	static EAetherResourceCue EvaluateResourceCue(
		float PreviousAetherGauge,
		float CurrentAetherGauge,
		const FAetherResourceCueConfig& Config,
		FAetherResourceCueState& State);

	/** 소비 후 해당 임계값 아래로 내려간 알림만 다시 재생 가능한 상태로 돌린다. */
	static void RefreshResourceStateAfterSpend(
		float CurrentAetherGauge,
		const FAetherResourceCueConfig& Config,
		FAetherResourceCueState& State);

	static void ResetResourceState(FAetherResourceCueState& State);

	static float GetRandomizedVolume(float BaseVolume, float Variance);
	/** 최소·최대 피치를 정렬하고 하한을 적용한 범위에서 무작위 값을 고른다. */
	static float GetRandomizedPitch(float MinPitch, float MaxPitch);
};
