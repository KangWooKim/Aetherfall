#include "AetherHealthComponent.h"

UAetherHealthComponent::UAetherHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAetherHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	ResetHealth();
}

bool UAetherHealthComponent::ApplyDamage(float DamageAmount, AActor* DamageCauser)
{
	if (bIsDead || DamageAmount <= 0.0f)
	{
		return false;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	if (CurrentHealth <= DeathHealthThreshold)
	{
		CurrentHealth = 0.0f;
	}

	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, DamageCauser);

	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		OnDeath.Broadcast(this, DamageCauser);
	}

	return true;
}

float UAetherHealthComponent::RestoreHealth(float HealAmount)
{
	if (bIsDead || HealAmount <= 0.0f)
	{
		return 0.0f;
	}

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
	const float ActualHealAmount = CurrentHealth - OldHealth;
	if (ActualHealAmount <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, nullptr);
	return ActualHealAmount;
}

void UAetherHealthComponent::SetCurrentHealth(float NewCurrentHealth)
{
	CurrentHealth = FMath::Clamp(NewCurrentHealth, 0.0f, MaxHealth);
	if (CurrentHealth <= DeathHealthThreshold)
	{
		CurrentHealth = 0.0f;
	}

	bIsDead = CurrentHealth <= 0.0f;
	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, nullptr);
}

void UAetherHealthComponent::ResetHealth()
{
	CurrentHealth = MaxHealth;
	bIsDead = false;
	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, nullptr);
}

void UAetherHealthComponent::SetMaxHealth(float NewMaxHealth, bool bResetCurrentHealth)
{
	MaxHealth = FMath::Max(1.0f, NewMaxHealth);
	if (bResetCurrentHealth)
	{
		ResetHealth();
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
	if (CurrentHealth <= DeathHealthThreshold)
	{
		CurrentHealth = 0.0f;
	}
	bIsDead = CurrentHealth <= 0.0f;
	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, nullptr);
}
