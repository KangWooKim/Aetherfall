#pragma once

#include "CoreMinimal.h"

class UAnimationAsset;
class UAnimMontage;
class USoundBase;
struct FAetherEnemyAttackPatternData;

/** 적 행동별 기본 애니메이션·몽타주 자산을 선택 정책에 전달한다. */
struct FAetherEnemyActionVisualAssets
{
	UAnimMontage* QuickAttackMontage = nullptr;
	UAnimMontage* StandardAttackMontage = nullptr;
	UAnimMontage* HeavyAttackMontage = nullptr;
	UAnimMontage* HitReactionMontage = nullptr;
	UAnimMontage* ParryStaggerMontage = nullptr;
	UAnimMontage* DeathMontage = nullptr;
	UAnimationAsset* QuickAttackAnimation = nullptr;
	UAnimationAsset* StandardAttackAnimation = nullptr;
	UAnimationAsset* HeavyAttackAnimation = nullptr;
	UAnimationAsset* HitReactionAnimation = nullptr;
	UAnimationAsset* ParryStaggerAnimation = nullptr;
	UAnimationAsset* DeathAnimation = nullptr;
};

/** 적 애니메이션의 재생 속도 등 표현 조정값을 묶는다. */
struct FAetherEnemyActionVisualTuning
{
	bool bUsePrototypeEnemyAnimationDriver = true;
	bool bPreferPrototypeFallbackActionAnimations = true;
	float MoveAnimationReferenceSpeed = 600.0f;
	float MoveAnimationMinPlayRate = 0.35f;
	float MoveAnimationMaxPlayRate = 0.80f;
	float IdleAnimationPlayRate = 1.0f;
};

/** 적 행동에 대해 선택된 표현 자산과 재생 정보를 반환한다. */
struct FAetherEnemyActionVisualSelection
{
	UAnimMontage* Montage = nullptr;
	UAnimationAsset* FallbackAnimation = nullptr;
	bool bPreferFallbackAnimation = false;
};

/** 적 행동 종류에 대응하는 효과음 자산을 묶는다. */
struct FAetherEnemyActionSoundSet
{
	USoundBase* QuickAttackWindupSound = nullptr;
	USoundBase* StandardAttackWindupSound = nullptr;
	USoundBase* HeavyAttackWindupSound = nullptr;
	USoundBase* AttackHitSound = nullptr;
	USoundBase* AttackMissSound = nullptr;
	USoundBase* HitReactionSound = nullptr;
	USoundBase* ParryStaggerSound = nullptr;
	USoundBase* DeathSound = nullptr;
	float EnemySoundVolume = 0.75f;
	float EnemySoundVolumeVariance = 0.08f;
	float EnemySoundPitchMin = 0.94f;
	float EnemySoundPitchMax = 1.06f;
};

/** 적 효과음의 재생 여부와 볼륨·피치를 호출자에게 전달한다. */
struct FAetherEnemySoundPlayback
{
	bool bShouldPlay = false;
	float Volume = 0.0f;
	float Pitch = 1.0f;
};

/** 적의 공격 패턴과 상태를 몽타주·대체 애니메이션·효과음 선택 및 재생 속도 설정으로 변환한다. */
class AETHERFALL_API FAetherEnemyActionPresentationPolicy
{
public:
	/** 공격 이름에 대응하는 몽타주와 대체 애니메이션을 함께 선택해 실행 측에 전달한다. */
	static FAetherEnemyActionVisualSelection BuildAttackVisual(
		const FAetherEnemyAttackPatternData& AttackPattern,
		const FAetherEnemyActionVisualAssets& VisualAssets,
		const FAetherEnemyActionVisualTuning& VisualTuning);

	static FAetherEnemyActionVisualSelection BuildHitReactionVisual(
		const FAetherEnemyActionVisualAssets& VisualAssets,
		const FAetherEnemyActionVisualTuning& VisualTuning);

	static FAetherEnemyActionVisualSelection BuildParryStaggerVisual(
		const FAetherEnemyActionVisualAssets& VisualAssets,
		const FAetherEnemyActionVisualTuning& VisualTuning);

	static FAetherEnemyActionVisualSelection BuildDeathVisual(
		const FAetherEnemyActionVisualAssets& VisualAssets,
		const FAetherEnemyActionVisualTuning& VisualTuning);

	static UAnimMontage* SelectAttackMontage(
		const FAetherEnemyAttackPatternData& AttackPattern,
		const FAetherEnemyActionVisualAssets& VisualAssets);

	static UAnimationAsset* SelectAttackAnimation(
		const FAetherEnemyAttackPatternData& AttackPattern,
		const FAetherEnemyActionVisualAssets& VisualAssets);

	/** 프로토타입 애니메이션 구동과 대체 연출 우선 설정이 모두 켜진 경우에만 대체 애니메이션을 우선한다. */
	static bool ShouldPreferFallbackActionAnimation(
		const FAetherEnemyActionVisualTuning& VisualTuning,
		const UAnimationAsset* FallbackAnimation);

	/** 이동 속도에 비례한 재생 속도를 설정 범위로 제한하고 0.05 단위로 양자화한다. */
	static float ResolveMoveAnimationPlayRate(
		float Speed2D,
		const FAetherEnemyActionVisualTuning& VisualTuning);

	static USoundBase* SelectAttackWindupSound(
		const FAetherEnemyAttackPatternData& AttackPattern,
		const FAetherEnemyActionSoundSet& SoundSet);

	/** 유효한 효과음과 기본 음량을 확인한 뒤 음량·피치 변화를 적용할 재생 정보를 만든다. */
	static FAetherEnemySoundPlayback BuildSoundPlayback(
		const USoundBase* Sound,
		float VolumeMultiplier,
		const FAetherEnemyActionSoundSet& SoundSet);

	static float GetRandomizedSoundVolume(float BaseVolume, float Variance);
	static float GetRandomizedSoundPitch(float MinPitch, float MaxPitch);
};
