#pragma once

#include "CoreMinimal.h"
#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"
#include "AetherSlashProjectile.generated.h"

class APawn;
class UAetherProjectilePoolSubsystem;
class USphereComponent;
class UPointLightComponent;
class UParticleSystem;
class UStaticMeshComponent;
class USoundBase;

UCLASS()
class AETHERFALL_API AAetherSlashProjectile : public AActor
{
	GENERATED_BODY()

public:
	AAetherSlashProjectile();

	virtual void Tick(float DeltaTime) override;
	virtual void LifeSpanExpired() override;

	void ActivateForPool(AActor* InOwner, APawn* InInstigator, const FVector& SpawnLocation, const FRotator& SpawnRotation);
	void DeactivateForPool();
	void InitializeSlash(AActor* InDamageCauser, AActor* InLockedTarget, const FVector& InDirection, float InDamage, float InSpeed, float InMaxDistance, float InTraceRadius);
	void SetImpactAssets(UParticleSystem* InImpactEffect, USoundBase* InImpactSound, float InEffectScale, float InSoundVolume);
	void SetOwningProjectilePool(UAetherProjectilePoolSubsystem* InOwningProjectilePool);
	bool IsAvailableForPool() const { return bAvailableForPool; }

protected:
	virtual void BeginPlay() override;

private:
	void ResetTransientState();
	void SweepForward(float DeltaTime);
	bool TryApplyDamage(AActor* TargetActor, bool bLockedTargetDamage);
	void FinishProjectile(bool bHitTarget);
	void ReleaseOrDestroy();
	void DrawFinishFeedback(bool bHitTarget) const;
	void PlayImpactFeedback(AActor* TargetActor) const;
	void ApplyHitStopToActor(AActor* Actor, float Duration) const;
	void ShowProjectileDebugMessage(const FString& Message, const FColor& Color) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SlashVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPointLightComponent> SlashGlow;

	TWeakObjectPtr<UAetherProjectilePoolSubsystem> OwningProjectilePool;
	TWeakObjectPtr<AActor> DamageCauser;
	TWeakObjectPtr<AActor> LockedTarget;
	TArray<FHitResult> ReusableHitResults;

	FVector TravelDirection = FVector::ForwardVector;
	float Damage = 32.0f;
	float ProjectileSpeed = 1800.0f;
	float MaxTravelDistance = 650.0f;
	float TraceRadius = 85.0f;
	float ImpactCameraKickStrength = 18.0f;
	float ImpactCameraKickDuration = 0.12f;
	float ImpactHitStopDuration = 0.055f;
	float ImpactHitStopTimeDilation = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Projectile|Debug")
	bool bDrawProjectileDebug = false;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Projectile|Debug")
	bool bShowProjectileScreenDebugMessages = false;

	TObjectPtr<UParticleSystem> ImpactEffect;
	TObjectPtr<USoundBase> ImpactSound;
	float ImpactEffectScale = 1.0f;
	float ImpactSoundVolume = 1.0f;
	float DistanceTraveled = 0.0f;
	bool bFinished = false;
	bool bAvailableForPool = false;
};
