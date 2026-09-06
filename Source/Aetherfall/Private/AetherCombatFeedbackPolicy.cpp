/** 전투 사건을 시각 효과와 효과음에 연결하며, 전용 자산이 없을 때 지정된 대체 자산을 선택한다. */
#include "AetherCombatFeedbackPolicy.h"

#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

/** 반격·처형·가드의 전용 효과가 비어 있으면 각 분기의 대체 효과를 반환한다. */
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

/** 전투 사건별 효과음을 선택하고, 대응 자산이 없는 종류는 재생하지 않도록 빈 값을 반환한다. */
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
