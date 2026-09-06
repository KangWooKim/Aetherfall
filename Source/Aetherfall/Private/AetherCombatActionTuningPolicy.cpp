/** 콤보별 수치 선택과 강공격·방어 비용 계산을 모은 정책이다. 잘못된 인덱스나 역전된 최소·최대 설정에는 명시된 대체 규칙을 적용한다. */
#include "AetherCombatActionTuningPolicy.h"

float FAetherCombatActionTuningPolicy::SelectLightAttackCost(int32 ComboStep, const TArray<float>& StaminaCosts)
{
	return SelectComboValue(ComboStep, StaminaCosts, 20.0f);
}

float FAetherCombatActionTuningPolicy::SelectLightAttackDamage(int32 ComboStep, const TArray<float>& DamageValues)
{
	return SelectComboValue(ComboStep, DamageValues, 20.0f);
}

float FAetherCombatActionTuningPolicy::CalculateHeavyAttackDamage(float BaseDamage, float StaggerDamageMultiplier, bool bStaggerBonus)
{
	const float ClampedBaseDamage = FMath::Max(0.0f, BaseDamage);
	if (!bStaggerBonus)
	{
		return ClampedBaseDamage;
	}

	return ClampedBaseDamage * FMath::Max(0.0f, StaggerDamageMultiplier);
}

/** 피해량 비례 옵션이 유효하면 기준 피해 대비 비용을 계산하고 정렬된 최소·최대 범위로 제한한다. */
float FAetherCombatActionTuningPolicy::CalculateGuardStaminaCost(float IncomingDamage, const FAetherGuardStaminaTuning& GuardTuning)
{
	const float BaseCost = FMath::Max(0.0f, GuardTuning.BaseCost);
	if (!GuardTuning.bScaleByIncomingDamage || GuardTuning.DamageReference <= 0.0f)
	{
		return BaseCost;
	}

	const float ScaledCost = BaseCost * FMath::Max(0.0f, IncomingDamage) / GuardTuning.DamageReference;
	const float MinCost = FMath::Min(GuardTuning.MinCost, GuardTuning.MaxCost);
	const float MaxCost = FMath::Max(GuardTuning.MinCost, GuardTuning.MaxCost);
	return FMath::Clamp(ScaledCost, FMath::Max(0.0f, MinCost), FMath::Max(0.0f, MaxCost));
}

/** 콤보 단계는 1부터 시작한다. 범위 밖 단계는 마지막 원소를, 빈 배열은 전달된 기본값을 사용한다. */
float FAetherCombatActionTuningPolicy::SelectComboValue(int32 ComboStep, const TArray<float>& Values, float DefaultValue)
{
	const int32 ValueIndex = ComboStep - 1;
	if (Values.IsValidIndex(ValueIndex))
	{
		return Values[ValueIndex];
	}

	return Values.Num() > 0 ? Values.Last() : DefaultValue;
}
