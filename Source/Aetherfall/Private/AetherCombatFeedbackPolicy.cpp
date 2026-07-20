#include "AetherCombatFeedbackPolicy.h"

#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

UParticleSystem* FAetherCombatFeedbackPolicy::SelectImpactEffect(
	EAetherCombatFeedbackType FeedbackType,
	const FAetherCombatFeedbackAssets& FeedbackAssets)
{
	switch (FeedbackType)
	{
	case EAetherCombatFeedbackType::LightHit:
		return FeedbackAssets.LightHitImpactEffect;
	case EAetherCombatFeedbackType::HeavyHit:
		return FeedbackAssets.HeavyHitImpactEffect;
	case EAetherCombatFeedbackType::HeavyCounterHit:
		return FeedbackAssets.HeavyCounterHitImpactEffect ? FeedbackAssets.HeavyCounterHitImpactEffect : FeedbackAssets.HeavyHitImpactEffect;
	case EAetherCombatFeedbackType::Execution:
		return FeedbackAssets.ExecutionImpactEffect ? FeedbackAssets.ExecutionImpactEffect : FeedbackAssets.HeavyCounterHitImpactEffect;
	case EAetherCombatFeedbackType::ParrySuccess:
		return FeedbackAssets.ParryImpactEffect;
	case EAetherCombatFeedbackType::PlayerHit:
		return FeedbackAssets.PlayerHitImpactEffect;
	case EAetherCombatFeedbackType::GuardBlock:
		return FeedbackAssets.GuardBlockImpactEffect ? FeedbackAssets.GuardBlockImpactEffect : FeedbackAssets.PlayerHitImpactEffect;
	case EAetherCombatFeedbackType::AetherSlash:
		return FeedbackAssets.AetherSlashImpactEffect;
	default:
		return nullptr;
	}
}

USoundBase* FAetherCombatFeedbackPolicy::SelectImpactSound(
	EAetherCombatFeedbackType FeedbackType,
	const FAetherCombatFeedbackAssets& FeedbackAssets)
{
	switch (FeedbackType)
	{
	case EAetherCombatFeedbackType::LightHit:
		return FeedbackAssets.LightHitSound;
	case EAetherCombatFeedbackType::HeavyHit:
		return FeedbackAssets.HeavyHitSound;
	case EAetherCombatFeedbackType::HeavyCounterHit:
		return FeedbackAssets.HeavyCounterHitSound ? FeedbackAssets.HeavyCounterHitSound : FeedbackAssets.HeavyHitSound;
	case EAetherCombatFeedbackType::Execution:
		return FeedbackAssets.ExecutionSound ? FeedbackAssets.ExecutionSound : FeedbackAssets.HeavyCounterHitSound;
	case EAetherCombatFeedbackType::ParrySuccess:
		return FeedbackAssets.ParrySuccessSound;
	case EAetherCombatFeedbackType::PlayerHit:
		return FeedbackAssets.PlayerHitSound;
	case EAetherCombatFeedbackType::GuardBlock:
		return FeedbackAssets.GuardBlockSound ? FeedbackAssets.GuardBlockSound : FeedbackAssets.PlayerHitSound;
	case EAetherCombatFeedbackType::AetherSlash:
		return FeedbackAssets.AetherSlashHitSound;
	default:
		return nullptr;
	}
}
