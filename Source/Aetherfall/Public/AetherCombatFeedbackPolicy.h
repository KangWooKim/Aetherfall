#pragma once

#include "CoreMinimal.h"
#include "AetherCombatFeedbackTypes.h"

class UParticleSystem;
class USoundBase;

/** 전투 사건별 효과와 사운드 및 대체 자산을 묶는다. */
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

/** 전투 사건을 시각 효과와 효과음에 연결하며, 전용 자산이 없을 때 지정된 대체 자산을 선택한다. */
class AETHERFALL_API FAetherCombatFeedbackPolicy
{
public:
	/** 반격·처형·가드의 전용 효과가 비어 있으면 각 분기의 대체 효과를 반환한다. */
	static UParticleSystem* SelectImpactEffect(
		EAetherCombatFeedbackType FeedbackType,
		const FAetherCombatFeedbackAssets& FeedbackAssets);

	/** 전투 사건별 효과음을 선택하고, 대응 자산이 없는 종류는 재생하지 않도록 빈 값을 반환한다. */
	static USoundBase* SelectImpactSound(
		EAetherCombatFeedbackType FeedbackType,
		const FAetherCombatFeedbackAssets& FeedbackAssets);
};
