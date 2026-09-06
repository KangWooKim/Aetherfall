#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AetherProjectilePoolSubsystem.generated.h"

class AAetherSlashProjectile;
class APawn;

/** 월드 수명에 맞춰 검기 투사체를 보관·재사용하며 추적 상한을 넘는 인스턴스는 일회성으로 생성한다. */
UCLASS()
class AETHERFALL_API UAetherProjectilePoolSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 각 투사체의 풀 역참조를 먼저 끊어 종료 중 재반환을 피하고 월드 소유 인스턴스를 정리한다. */
	virtual void Deinitialize() override;

	/** 반환된 인스턴스를 우선 활성화하고, 없으면 생성한다. 풀 추적 상한 12개는 동시 발사 수 제한이 아니다. */
	AAetherSlashProjectile* AcquireAetherSlashProjectile(
		AActor* Owner,
		APawn* Instigator,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation);

	/** 이 풀에 등록된 투사체는 비활성화하고, 등록되지 않은 투사체는 소유 풀을 끊은 뒤 파괴한다. */
	void ReleaseAetherSlashProjectile(AAetherSlashProjectile* Projectile);

	int32 GetTrackedAetherSlashProjectileCount() const;
	int32 GetAvailableAetherSlashProjectileCount() const;

private:
	void CompactInvalidProjectiles();

	UPROPERTY(Transient)
	TArray<TObjectPtr<AAetherSlashProjectile>> AetherSlashProjectilePool;
};
