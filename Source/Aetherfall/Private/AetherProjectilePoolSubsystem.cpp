/** 월드 수명에 맞춰 검기 투사체를 보관·재사용하며 추적 상한을 넘는 인스턴스는 일회성으로 생성한다. */
#include "AetherProjectilePoolSubsystem.h"

#include "AetherSlashProjectile.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

namespace
{
	constexpr int32 MaxTrackedAetherSlashProjectiles = 12;
}

/** 각 투사체의 풀 역참조를 먼저 끊어 종료 중 재반환을 피하고 월드 소유 인스턴스를 정리한다. */
void UAetherProjectilePoolSubsystem::Deinitialize()
{
	for (TObjectPtr<AAetherSlashProjectile>& ProjectilePtr : AetherSlashProjectilePool)
	{
		AAetherSlashProjectile* Projectile = ProjectilePtr.Get();
		if (!IsValid(Projectile))
		{
			continue;
		}

		Projectile->SetOwningProjectilePool(nullptr);
		Projectile->Destroy();
	}

	AetherSlashProjectilePool.Reset();

	Super::Deinitialize();
}

/** 반환된 인스턴스를 우선 활성화하고, 없으면 생성한다. 풀 추적 상한 12개는 동시 발사 수 제한이 아니다. */
AAetherSlashProjectile* UAetherProjectilePoolSubsystem::AcquireAetherSlashProjectile(
	AActor* Owner,
	APawn* Instigator,
	const FVector& SpawnLocation,
	const FRotator& SpawnRotation)
{
	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		return nullptr;
	}

	CompactInvalidProjectiles();

	for (TObjectPtr<AAetherSlashProjectile>& ProjectilePtr : AetherSlashProjectilePool)
	{
		AAetherSlashProjectile* Projectile = ProjectilePtr.Get();
		if (!IsValid(Projectile) || !Projectile->IsAvailableForPool())
		{
			continue;
		}

		Projectile->ActivateForPool(Owner, Instigator, SpawnLocation, SpawnRotation);
		return Projectile;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = Owner;
	SpawnParameters.Instigator = Instigator;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAetherSlashProjectile* Projectile = World->SpawnActor<AAetherSlashProjectile>(
		AAetherSlashProjectile::StaticClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);
	if (!Projectile)
	{
		return nullptr;
	}

	// 상한은 풀에서 보관할 수다. 초과 생성된 검기는 추적하지 않고 사용 후 파괴한다.
	if (AetherSlashProjectilePool.Num() < MaxTrackedAetherSlashProjectiles)
	{
		Projectile->SetOwningProjectilePool(this);
		AetherSlashProjectilePool.Add(Projectile);
	}
	else
	{
		Projectile->SetOwningProjectilePool(nullptr);
	}

	Projectile->ActivateForPool(Owner, Instigator, SpawnLocation, SpawnRotation);
	return Projectile;
}

/** 이 풀에 등록된 투사체는 비활성화하고, 등록되지 않은 투사체는 소유 풀을 끊은 뒤 파괴한다. */
void UAetherProjectilePoolSubsystem::ReleaseAetherSlashProjectile(AAetherSlashProjectile* Projectile)
{
	if (!IsValid(Projectile))
	{
		return;
	}

	if (AetherSlashProjectilePool.IndexOfByKey(Projectile) == INDEX_NONE)
	{
		Projectile->SetOwningProjectilePool(nullptr);
		Projectile->Destroy();
		return;
	}

	Projectile->DeactivateForPool();
}

int32 UAetherProjectilePoolSubsystem::GetTrackedAetherSlashProjectileCount() const
{
	int32 TrackedCount = 0;
	for (const TObjectPtr<AAetherSlashProjectile>& ProjectilePtr : AetherSlashProjectilePool)
	{
		const AAetherSlashProjectile* Projectile = ProjectilePtr.Get();
		if (IsValid(Projectile))
		{
			++TrackedCount;
		}
	}

	return TrackedCount;
}

int32 UAetherProjectilePoolSubsystem::GetAvailableAetherSlashProjectileCount() const
{
	int32 AvailableCount = 0;
	for (const TObjectPtr<AAetherSlashProjectile>& ProjectilePtr : AetherSlashProjectilePool)
	{
		const AAetherSlashProjectile* Projectile = ProjectilePtr.Get();
		if (IsValid(Projectile) && Projectile->IsAvailableForPool())
		{
			++AvailableCount;
		}
	}

	return AvailableCount;
}

void UAetherProjectilePoolSubsystem::CompactInvalidProjectiles()
{
	AetherSlashProjectilePool.RemoveAll([](const TObjectPtr<AAetherSlashProjectile>& Projectile)
	{
		return !IsValid(Projectile);
	});
}
