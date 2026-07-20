#include "AetherProjectilePoolSubsystem.h"

#include "AetherSlashProjectile.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

namespace
{
	constexpr int32 MaxTrackedAetherSlashProjectiles = 12;
}

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
