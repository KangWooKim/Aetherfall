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

/** 직선 구체 스윕으로 한 대상에게 피해를 주는 검기다. 종료 시 풀에 반환하며 추적되지 않은 인스턴스는 파괴한다. */
UCLASS()
class AETHERFALL_API AAetherSlashProjectile : public AActor
{
	GENERATED_BODY()

public:
	AAetherSlashProjectile();

	virtual void Tick(float DeltaTime) override;
	virtual void LifeSpanExpired() override;

	/** 소유자·위치·가시성·충돌·틱을 활성화하고 사용 중 상태로 전환한다. 발사별 수치는 별도 초기화한다. */
	void ActivateForPool(AActor* InOwner, APawn* InInstigator, const FVector& SpawnLocation, const FRotator& SpawnRotation);
	/** 수명 타이머와 이전 발사 상태를 지우고 표시·충돌·틱 및 소유 관계를 해제해 재사용 대기로 둔다. */
	void DeactivateForPool();
	/** 방향과 이동 수치를 보정하고 새 발사의 누적 거리 및 최대 수명을 설정한다. */
	void InitializeSlash(AActor* InDamageCauser, AActor* InLockedTarget, const FVector& InDirection, float InDamage, float InSpeed, float InMaxDistance, float InTraceRadius);
	void SetImpactAssets(UParticleSystem* InImpactEffect, USoundBase* InImpactSound, float InEffectScale, float InSoundVolume);
	void SetOwningProjectilePool(UAetherProjectilePoolSubsystem* InOwningProjectilePool);
	bool IsAvailableForPool() const { return bAvailableForPool; }

protected:
	virtual void BeginPlay() override;

private:
	/** 이전 발사의 대상·피해·거리·효과 참조를 초기화하되 적중 배열의 용량은 재사용한다. */
	void ResetTransientState();
	/** 남은 사거리 안에서 스윕하고 실제 적중 후보 중 잠금 대상을 우선한다. 피해 적용에 성공한 첫 대상에서 발사를 종료한다. */
	void SweepForward(float DeltaTime);
	/** 공통 피해 정책이 적용을 승인한 경우에만 적중 연출을 재생한다. */
	bool TryApplyDamage(AActor* TargetActor, bool bLockedTargetDamage);
	/** 종료 플래그를 먼저 세워 중복 처리를 막고 종료 피드백 이후 반환 또는 파괴한다. */
	void FinishProjectile(bool bHitTarget);
	/** 유효한 소속 풀이 있으면 반환하고 풀이 없는 일회용 검기는 파괴한다. */
	void ReleaseOrDestroy();
	void DrawFinishFeedback(bool bHitTarget) const;
	void PlayImpactFeedback(AActor* TargetActor) const;
	/** 대상의 시간 배율을 잠시 낮추고 약한 참조를 캡처한 타이머로 복원한다. 여러 적중의 타이머를 통합 관리하지는 않는다. */
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
