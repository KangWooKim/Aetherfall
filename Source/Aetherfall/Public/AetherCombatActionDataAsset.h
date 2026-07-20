#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AetherCombatActionDataAsset.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FAetherCombatLightAttackData
{
	GENERATED_BODY()

public:
	const TArray<float>& GetStaminaCosts() const { return StaminaCosts; }
	const TArray<float>& GetDamageValues() const { return DamageValues; }
	const TArray<TObjectPtr<UAnimMontage>>& GetMontages() const { return Montages; }
	float GetDuration() const { return Duration; }
	float GetTraceDistance() const { return TraceDistance; }
	float GetTraceRadius() const { return TraceRadius; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (AllowPrivateAccess = "true"))
	TArray<float> StaminaCosts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (AllowPrivateAccess = "true"))
	TArray<float> DamageValues;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UAnimMontage>> Montages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light", meta = (ClampMin = "0.1", AllowPrivateAccess = "true"))
	float Duration = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light|Trace", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float TraceDistance = 185.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Light|Trace", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float TraceRadius = 75.0f;
};

USTRUCT(BlueprintType)
struct FAetherCombatHeavyAttackData
{
	GENERATED_BODY()

public:
	UAnimMontage* GetAttackMontage() const { return AttackMontage.Get(); }
	UAnimMontage* GetCounterAttackMontage() const { return CounterAttackMontage.Get(); }
	float GetDuration() const { return Duration; }
	float GetImpactDelay() const { return ImpactDelay; }
	float GetStaminaCost() const { return StaminaCost; }
	float GetDamage() const { return Damage; }
	float GetStaggerDamageMultiplier() const { return StaggerDamageMultiplier; }
	float GetTraceDistance() const { return TraceDistance; }
	float GetTraceRadius() const { return TraceRadius; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heavy", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heavy", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> CounterAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heavy", meta = (ClampMin = "0.1", AllowPrivateAccess = "true"))
	float Duration = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heavy", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float ImpactDelay = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heavy", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float StaminaCost = 34.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heavy", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Damage = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heavy", meta = (ClampMin = "1.0", AllowPrivateAccess = "true"))
	float StaggerDamageMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heavy|Trace", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float TraceDistance = 210.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heavy|Trace", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float TraceRadius = 90.0f;
};

USTRUCT(BlueprintType)
struct FAetherCombatAetherSlashData
{
	GENERATED_BODY()

public:
	UAnimMontage* GetMontage() const { return Montage.Get(); }
	float GetCost() const { return Cost; }
	float GetDamage() const { return Damage; }
	float GetDuration() const { return Duration; }
	float GetImpactDelay() const { return ImpactDelay; }
	float GetCooldown() const { return Cooldown; }
	float GetProjectileSpeed() const { return ProjectileSpeed; }
	float GetTraceDistance() const { return TraceDistance; }
	float GetTraceRadius() const { return TraceRadius; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether Slash", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether Slash", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Cost = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether Slash", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Damage = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether Slash", meta = (ClampMin = "0.1", AllowPrivateAccess = "true"))
	float Duration = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether Slash", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float ImpactDelay = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether Slash", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float Cooldown = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether Slash|Projectile", meta = (ClampMin = "1.0", AllowPrivateAccess = "true"))
	float ProjectileSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether Slash|Trace", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float TraceDistance = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether Slash|Trace", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float TraceRadius = 85.0f;
};

UCLASS(BlueprintType)
class AETHERFALL_API UAetherCombatActionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	bool ShouldOverrideLightAttack() const { return bOverrideLightAttack; }
	bool ShouldOverrideHeavyAttack() const { return bOverrideHeavyAttack; }
	bool ShouldOverrideAetherSlash() const { return bOverrideAetherSlash; }

	const FAetherCombatLightAttackData& GetLightAttackData() const { return LightAttack; }
	const FAetherCombatHeavyAttackData& GetHeavyAttackData() const { return HeavyAttack; }
	const FAetherCombatAetherSlashData& GetAetherSlashData() const { return AetherSlash; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Actions|Light", meta = (AllowPrivateAccess = "true"))
	bool bOverrideLightAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Actions|Light", meta = (EditCondition = "bOverrideLightAttack", AllowPrivateAccess = "true"))
	FAetherCombatLightAttackData LightAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Actions|Heavy", meta = (AllowPrivateAccess = "true"))
	bool bOverrideHeavyAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Actions|Heavy", meta = (EditCondition = "bOverrideHeavyAttack", AllowPrivateAccess = "true"))
	FAetherCombatHeavyAttackData HeavyAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Actions|Aether Slash", meta = (AllowPrivateAccess = "true"))
	bool bOverrideAetherSlash = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Actions|Aether Slash", meta = (EditCondition = "bOverrideAetherSlash", AllowPrivateAccess = "true"))
	FAetherCombatAetherSlashData AetherSlash;
};
