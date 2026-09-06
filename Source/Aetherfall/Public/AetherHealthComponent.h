#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAetherHealthChangedSignature, UAetherHealthComponent*, HealthComponent, float, CurrentHealth, float, MaxHealth, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAetherDeathSignature, UAetherHealthComponent*, HealthComponent, AActor*, DamageCauser);

/** 체력 범위와 사망 상태를 관리하고 피해·회복·복원에 따른 변경 이벤트를 알린다. */
UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherHealthComponent();

	/** 죽은 대상과 양수 이외의 피해를 거부하고, 체력 변경 알림 후 체력이 0이면 사망 상태와 이벤트를 갱신한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Health")
	bool ApplyDamage(float DamageAmount, AActor* DamageCauser);

	/** 생존 대상만 최대 체력까지 회복시키며 실제 증가량을 반환해 아이템 소비 여부를 판단하게 한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Health")
	float RestoreHealth(float HealAmount);

	/** 저장 복원용 체력값과 사망 플래그를 맞추고 체력 변경만 알린다. 사망 이벤트는 발생시키지 않는다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Health")
	void SetCurrentHealth(float NewCurrentHealth);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Health")
	void ResetHealth();

	/** 최대 체력을 1 이상으로 제한하고 요청에 따라 현재 체력을 초기화하거나 범위 안으로 보정한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Health")
	void SetMaxHealth(float NewMaxHealth, bool bResetCurrentHealth = true);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Health")
	FORCEINLINE bool IsDead() const { return bIsDead; }

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Health")
	FAetherHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Health")
	FAetherDeathSignature OnDeath;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "0.0"))
	float DeathHealthThreshold = 0.5f;

	UPROPERTY(VisibleInstanceOnly, Category = "Health")
	float CurrentHealth = 0.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "Health")
	bool bIsDead = false;
};
