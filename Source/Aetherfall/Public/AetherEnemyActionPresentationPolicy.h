#pragma once

#include "CoreMinimal.h"

class UAnimationAsset;
class UAnimMontage;
class USoundBase;
struct FAetherEnemyAttackPatternData;

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

struct FAetherEnemyActionVisualTuning
{
	bool bUsePrototypeEnemyAnimationDriver = true;
	bool bPreferPrototypeFallbackActionAnimations = true;
	float MoveAnimationReferenceSpeed = 600.0f;
	float MoveAnimationMinPlayRate = 0.35f;
	float MoveAnimationMaxPlayRate = 0.80f;
	float IdleAnimationPlayRate = 1.0f;
};

struct FAetherEnemyActionVisualSelection
{
	UAnimMontage* Montage = nullptr;
	UAnimationAsset* FallbackAnimation = nullptr;
	bool bPreferFallbackAnimation = false;
};

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

struct FAetherEnemySoundPlayback
{
	bool bShouldPlay = false;
	float Volume = 0.0f;
	float Pitch = 1.0f;
};

class AETHERFALL_API FAetherEnemyActionPresentationPolicy
{
public:
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

	static bool ShouldPreferFallbackActionAnimation(
		const FAetherEnemyActionVisualTuning& VisualTuning,
		const UAnimationAsset* FallbackAnimation);

	static float ResolveMoveAnimationPlayRate(
		float Speed2D,
		const FAetherEnemyActionVisualTuning& VisualTuning);

	static USoundBase* SelectAttackWindupSound(
		const FAetherEnemyAttackPatternData& AttackPattern,
		const FAetherEnemyActionSoundSet& SoundSet);

	static FAetherEnemySoundPlayback BuildSoundPlayback(
		const USoundBase* Sound,
		float VolumeMultiplier,
		const FAetherEnemyActionSoundSet& SoundSet);

	static float GetRandomizedSoundVolume(float BaseVolume, float Variance);
	static float GetRandomizedSoundPitch(float MinPitch, float MaxPitch);
};
