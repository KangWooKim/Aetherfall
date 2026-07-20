#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AetherProjectilePoolSubsystem.generated.h"

class AAetherSlashProjectile;
class APawn;

UCLASS()
class AETHERFALL_API UAetherProjectilePoolSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	AAetherSlashProjectile* AcquireAetherSlashProjectile(
		AActor* Owner,
		APawn* Instigator,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation);

	void ReleaseAetherSlashProjectile(AAetherSlashProjectile* Projectile);

	int32 GetTrackedAetherSlashProjectileCount() const;
	int32 GetAvailableAetherSlashProjectileCount() const;

private:
	void CompactInvalidProjectiles();

	UPROPERTY(Transient)
	TArray<TObjectPtr<AAetherSlashProjectile>> AetherSlashProjectilePool;
};
