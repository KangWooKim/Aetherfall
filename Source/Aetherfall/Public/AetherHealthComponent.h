#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAetherHealthChangedSignature, UAetherHealthComponent*, HealthComponent, float, CurrentHealth, float, MaxHealth, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAetherDeathSignature, UAetherHealthComponent*, HealthComponent, AActor*, DamageCauser);

UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherHealthComponent();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Health")
	bool ApplyDamage(float DamageAmount, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Health")
	float RestoreHealth(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Health")
	void SetCurrentHealth(float NewCurrentHealth);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Health")
	void ResetHealth();

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
