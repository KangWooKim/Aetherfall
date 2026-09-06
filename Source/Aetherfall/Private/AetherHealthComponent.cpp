/** 체력 범위와 사망 상태를 관리하고 피해·회복·복원에 따른 변경 이벤트를 알린다. */
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

/** 죽은 대상과 양수 이외의 피해를 거부하고, 체력 변경 알림 후 체력이 0이면 사망 상태와 이벤트를 갱신한다. */
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

/** 생존 대상만 최대 체력까지 회복시키며 실제 증가량을 반환해 아이템 소비 여부를 판단하게 한다. */
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

/** 저장 복원용 체력값과 사망 플래그를 맞추고 체력 변경만 알린다. 사망 이벤트는 발생시키지 않는다. */
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

/** 최대 체력을 1 이상으로 제한하고 요청에 따라 현재 체력을 초기화하거나 범위 안으로 보정한다. */
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
