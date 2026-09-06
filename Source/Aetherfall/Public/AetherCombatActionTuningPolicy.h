#pragma once

#include "CoreMinimal.h"

/** 방어 피해에서 스태미나 비용과 파괴 여부를 계산할 조정값이다. */
struct FAetherGuardStaminaTuning
{
	float BaseCost = 12.0f;
	bool bScaleByIncomingDamage = true;
	float DamageReference = 24.0f;
	float MinCost = 8.0f;
	float MaxCost = 18.0f;
};

/** 콤보별 수치 선택과 강공격·방어 비용 계산을 모은 정책이다. 잘못된 인덱스나 역전된 최소·최대 설정에는 명시된 대체 규칙을 적용한다. */
class AETHERFALL_API FAetherCombatActionTuningPolicy
{
public:
	static float SelectLightAttackCost(int32 ComboStep, const TArray<float>& StaminaCosts);
	static float SelectLightAttackDamage(int32 ComboStep, const TArray<float>& DamageValues);
	static float CalculateHeavyAttackDamage(float BaseDamage, float StaggerDamageMultiplier, bool bStaggerBonus);
	/** 피해량 비례 옵션이 유효하면 기준 피해 대비 비용을 계산하고 정렬된 최소·최대 범위로 제한한다. */
	static float CalculateGuardStaminaCost(float IncomingDamage, const FAetherGuardStaminaTuning& GuardTuning);

private:
	/** 콤보 단계는 1부터 시작한다. 범위 밖 단계는 마지막 원소를, 빈 배열은 전달된 기본값을 사용한다. */
	static float SelectComboValue(int32 ComboStep, const TArray<float>& Values, float DefaultValue);
};
