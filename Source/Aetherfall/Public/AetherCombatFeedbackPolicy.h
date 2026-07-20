#pragma once

#include "CoreMinimal.h"
#include "AetherCombatFeedbackTypes.h"

class UParticleSystem;
class USoundBase;

struct FAetherCombatFeedbackAssets
{
	UParticleSystem* LightHitImpactEffect = nullptr;
	UParticleSystem* HeavyHitImpactEffect = nullptr;
	UParticleSystem* HeavyCounterHitImpactEffect = nullptr;
	UParticleSystem* ExecutionImpactEffect = nullptr;
	UParticleSystem* ParryImpactEffect = nullptr;
	UParticleSystem* PlayerHitImpactEffect = nullptr;
	UParticleSystem* GuardBlockImpactEffect = nullptr;
	UParticleSystem* AetherSlashImpactEffect = nullptr;

	USoundBase* LightHitSound = nullptr;
	USoundBase* HeavyHitSound = nullptr;
	USoundBase* HeavyCounterHitSound = nullptr;
	USoundBase* ExecutionSound = nullptr;
	USoundBase* ParrySuccessSound = nullptr;
	USoundBase* PlayerHitSound = nullptr;
	USoundBase* GuardBlockSound = nullptr;
	USoundBase* AetherSlashHitSound = nullptr;
};

class AETHERFALL_API FAetherCombatFeedbackPolicy
{
public:
	static UParticleSystem* SelectImpactEffect(
		EAetherCombatFeedbackType FeedbackType,
		const FAetherCombatFeedbackAssets& FeedbackAssets);

	static USoundBase* SelectImpactSound(
		EAetherCombatFeedbackType FeedbackType,
		const FAetherCombatFeedbackAssets& FeedbackAssets);
};
