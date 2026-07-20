#include "AetherEnemyActionPresentationPolicy.h"

#include "AetherEnemyBase.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundBase.h"

namespace
{
bool IsQuickPresentationPattern(const FAetherEnemyAttackPatternData& AttackPattern)
{
	const FString PatternText = AttackPattern.PatternName.ToString();
	return PatternText.Contains(TEXT("Quick")) || PatternText.Contains(TEXT("Rush")) || PatternText.Contains(TEXT("Feint"));
}

bool IsHeavyPresentationPattern(const FAetherEnemyAttackPatternData& AttackPattern)
{
	const FString PatternText = AttackPattern.PatternName.ToString();
	return PatternText.Contains(TEXT("Heavy")) ||
		PatternText.Contains(TEXT("Breaker")) ||
		PatternText.Contains(TEXT("Cleave")) ||
		PatternText.Contains(TEXT("Sweep"));
}

FAetherEnemyActionVisualSelection BuildVisualSelection(
	UAnimMontage* Montage,
	UAnimationAsset* FallbackAnimation,
	const FAetherEnemyActionVisualTuning& VisualTuning)
{
	FAetherEnemyActionVisualSelection Selection;
	Selection.Montage = Montage;
	Selection.FallbackAnimation = FallbackAnimation;
	Selection.bPreferFallbackAnimation =
		FAetherEnemyActionPresentationPolicy::ShouldPreferFallbackActionAnimation(VisualTuning, FallbackAnimation);
	return Selection;
}
}

FAetherEnemyActionVisualSelection FAetherEnemyActionPresentationPolicy::BuildAttackVisual(
	const FAetherEnemyAttackPatternData& AttackPattern,
	const FAetherEnemyActionVisualAssets& VisualAssets,
	const FAetherEnemyActionVisualTuning& VisualTuning)
{
	return BuildVisualSelection(
		SelectAttackMontage(AttackPattern, VisualAssets),
		SelectAttackAnimation(AttackPattern, VisualAssets),
		VisualTuning);
}

FAetherEnemyActionVisualSelection FAetherEnemyActionPresentationPolicy::BuildHitReactionVisual(
	const FAetherEnemyActionVisualAssets& VisualAssets,
	const FAetherEnemyActionVisualTuning& VisualTuning)
{
	return BuildVisualSelection(VisualAssets.HitReactionMontage, VisualAssets.HitReactionAnimation, VisualTuning);
}

FAetherEnemyActionVisualSelection FAetherEnemyActionPresentationPolicy::BuildParryStaggerVisual(
	const FAetherEnemyActionVisualAssets& VisualAssets,
	const FAetherEnemyActionVisualTuning& VisualTuning)
{
	return BuildVisualSelection(VisualAssets.ParryStaggerMontage, VisualAssets.ParryStaggerAnimation, VisualTuning);
}

FAetherEnemyActionVisualSelection FAetherEnemyActionPresentationPolicy::BuildDeathVisual(
	const FAetherEnemyActionVisualAssets& VisualAssets,
	const FAetherEnemyActionVisualTuning& VisualTuning)
{
	return BuildVisualSelection(VisualAssets.DeathMontage, VisualAssets.DeathAnimation, VisualTuning);
}

UAnimMontage* FAetherEnemyActionPresentationPolicy::SelectAttackMontage(
	const FAetherEnemyAttackPatternData& AttackPattern,
	const FAetherEnemyActionVisualAssets& VisualAssets)
{
	if (IsQuickPresentationPattern(AttackPattern))
	{
		return VisualAssets.QuickAttackMontage ? VisualAssets.QuickAttackMontage : VisualAssets.StandardAttackMontage;
	}

	if (IsHeavyPresentationPattern(AttackPattern))
	{
		return VisualAssets.HeavyAttackMontage ? VisualAssets.HeavyAttackMontage : VisualAssets.StandardAttackMontage;
	}

	return VisualAssets.StandardAttackMontage;
}

UAnimationAsset* FAetherEnemyActionPresentationPolicy::SelectAttackAnimation(
	const FAetherEnemyAttackPatternData& AttackPattern,
	const FAetherEnemyActionVisualAssets& VisualAssets)
{
	if (IsQuickPresentationPattern(AttackPattern))
	{
		return VisualAssets.QuickAttackAnimation ? VisualAssets.QuickAttackAnimation : VisualAssets.StandardAttackAnimation;
	}

	if (IsHeavyPresentationPattern(AttackPattern))
	{
		return VisualAssets.HeavyAttackAnimation ? VisualAssets.HeavyAttackAnimation : VisualAssets.StandardAttackAnimation;
	}

	return VisualAssets.StandardAttackAnimation;
}

bool FAetherEnemyActionPresentationPolicy::ShouldPreferFallbackActionAnimation(
	const FAetherEnemyActionVisualTuning& VisualTuning,
	const UAnimationAsset* FallbackAnimation)
{
	return VisualTuning.bUsePrototypeEnemyAnimationDriver &&
		VisualTuning.bPreferPrototypeFallbackActionAnimations &&
		FallbackAnimation != nullptr;
}

float FAetherEnemyActionPresentationPolicy::ResolveMoveAnimationPlayRate(
	float Speed2D,
	const FAetherEnemyActionVisualTuning& VisualTuning)
{
	if (VisualTuning.MoveAnimationReferenceSpeed <= KINDA_SMALL_NUMBER)
	{
		return FMath::Clamp(VisualTuning.MoveAnimationMaxPlayRate, 0.05f, 1.0f);
	}

	const float RawRate = Speed2D / VisualTuning.MoveAnimationReferenceSpeed;
	const float ClampedRate = FMath::Clamp(
		RawRate,
		FMath::Max(0.05f, VisualTuning.MoveAnimationMinPlayRate),
		FMath::Max(0.05f, VisualTuning.MoveAnimationMaxPlayRate));
	return FMath::RoundToFloat(ClampedRate * 20.0f) / 20.0f;
}

USoundBase* FAetherEnemyActionPresentationPolicy::SelectAttackWindupSound(
	const FAetherEnemyAttackPatternData& AttackPattern,
	const FAetherEnemyActionSoundSet& SoundSet)
{
	if (AttackPattern.PatternName == FName(TEXT("Quick")) && SoundSet.QuickAttackWindupSound)
	{
		return SoundSet.QuickAttackWindupSound;
	}

	if (AttackPattern.PatternName == FName(TEXT("Heavy")) && SoundSet.HeavyAttackWindupSound)
	{
		return SoundSet.HeavyAttackWindupSound;
	}

	if (SoundSet.StandardAttackWindupSound)
	{
		return SoundSet.StandardAttackWindupSound;
	}

	if (SoundSet.QuickAttackWindupSound)
	{
		return SoundSet.QuickAttackWindupSound;
	}

	return SoundSet.HeavyAttackWindupSound;
}

FAetherEnemySoundPlayback FAetherEnemyActionPresentationPolicy::BuildSoundPlayback(
	const USoundBase* Sound,
	float VolumeMultiplier,
	const FAetherEnemyActionSoundSet& SoundSet)
{
	FAetherEnemySoundPlayback Playback;
	if (!Sound || SoundSet.EnemySoundVolume <= 0.0f)
	{
		return Playback;
	}

	const float BaseVolume = SoundSet.EnemySoundVolume * FMath::Max(0.0f, VolumeMultiplier);
	Playback.Volume = GetRandomizedSoundVolume(BaseVolume, SoundSet.EnemySoundVolumeVariance);
	Playback.Pitch = GetRandomizedSoundPitch(SoundSet.EnemySoundPitchMin, SoundSet.EnemySoundPitchMax);
	Playback.bShouldPlay = Playback.Volume > 0.0f;
	return Playback;
}

float FAetherEnemyActionPresentationPolicy::GetRandomizedSoundVolume(float BaseVolume, float Variance)
{
	const float ClampedBaseVolume = FMath::Max(0.0f, BaseVolume);
	const float ClampedVariance = FMath::Clamp(Variance, 0.0f, 1.0f);
	if (ClampedBaseVolume <= 0.0f || ClampedVariance <= 0.0f)
	{
		return ClampedBaseVolume;
	}

	return ClampedBaseVolume * FMath::FRandRange(1.0f - ClampedVariance, 1.0f + ClampedVariance);
}

float FAetherEnemyActionPresentationPolicy::GetRandomizedSoundPitch(float MinPitch, float MaxPitch)
{
	const float ClampedMinPitch = FMath::Max(0.1f, FMath::Min(MinPitch, MaxPitch));
	const float ClampedMaxPitch = FMath::Max(0.1f, FMath::Max(MinPitch, MaxPitch));
	return FMath::FRandRange(ClampedMinPitch, ClampedMaxPitch);
}
