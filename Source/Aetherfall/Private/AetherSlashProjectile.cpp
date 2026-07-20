#include "AetherSlashProjectile.h"

#include "AetherAudioSettingsLibrary.h"
#include "AetherCombatDamagePolicy.h"
#include "AetherCombatTargetSelectionPolicy.h"
#include "AetherProjectilePoolSubsystem.h"
#include "AetherfallCharacter.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AAetherSlashProjectile::AAetherSlashProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->InitSphereRadius(85.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);

	SlashVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SlashVisual"));
	SlashVisual->SetupAttachment(RootComponent);
	SlashVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SlashVisual->SetCastShadow(false);
	SlashVisual->SetRelativeScale3D(FVector(0.08f, 1.4f, 0.55f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SlashMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (SlashMesh.Succeeded())
	{
		SlashVisual->SetStaticMesh(SlashMesh.Object);
	}

	SlashGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("SlashGlow"));
	SlashGlow->SetupAttachment(RootComponent);
	SlashGlow->SetLightColor(FLinearColor(0.0f, 0.72f, 1.0f));
	SlashGlow->SetIntensity(2800.0f);
	SlashGlow->SetAttenuationRadius(260.0f);
	SlashGlow->SetSourceRadius(55.0f);
}

void AAetherSlashProjectile::BeginPlay()
{
	Super::BeginPlay();

	ShowProjectileDebugMessage(TEXT("Aether Slash projectile spawned"), FColor(0, 180, 255));
}

void AAetherSlashProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SweepForward(DeltaTime);
}

void AAetherSlashProjectile::LifeSpanExpired()
{
	FinishProjectile(false);
}

void AAetherSlashProjectile::ActivateForPool(AActor* InOwner, APawn* InInstigator, const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	SetOwner(InOwner);
	SetInstigator(InInstigator);
	SetActorLocationAndRotation(SpawnLocation, SpawnRotation, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	if (SlashVisual)
	{
		SlashVisual->SetVisibility(true, true);
	}
	if (SlashGlow)
	{
		SlashGlow->SetVisibility(true, true);
	}

	bAvailableForPool = false;
	bFinished = false;
}

void AAetherSlashProjectile::DeactivateForPool()
{
	SetLifeSpan(0.0f);
	ResetTransientState();
	bFinished = true;
	bAvailableForPool = true;

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	SetOwner(nullptr);
	SetInstigator(nullptr);

	if (SlashVisual)
	{
		SlashVisual->SetVisibility(false, true);
	}
	if (SlashGlow)
	{
		SlashGlow->SetVisibility(false, true);
	}
}

void AAetherSlashProjectile::ResetTransientState()
{
	DamageCauser.Reset();
	LockedTarget.Reset();
	ReusableHitResults.Reset();
	TravelDirection = FVector::ForwardVector;
	Damage = 32.0f;
	ProjectileSpeed = 1800.0f;
	MaxTravelDistance = 650.0f;
	TraceRadius = 85.0f;
	ImpactEffect = nullptr;
	ImpactSound = nullptr;
	ImpactEffectScale = 1.0f;
	ImpactSoundVolume = 1.0f;
	DistanceTraveled = 0.0f;
}

void AAetherSlashProjectile::InitializeSlash(AActor* InDamageCauser, AActor* InLockedTarget, const FVector& InDirection, float InDamage, float InSpeed, float InMaxDistance, float InTraceRadius)
{
	bAvailableForPool = false;
	DamageCauser = InDamageCauser;
	LockedTarget = InLockedTarget;
	TravelDirection = InDirection.GetSafeNormal();
	if (TravelDirection.IsNearlyZero())
	{
		TravelDirection = GetActorForwardVector().GetSafeNormal();
	}
	if (TravelDirection.IsNearlyZero())
	{
		TravelDirection = FVector::ForwardVector;
	}

	Damage = InDamage;
	ProjectileSpeed = FMath::Max(1.0f, InSpeed);
	MaxTravelDistance = FMath::Max(1.0f, InMaxDistance);
	TraceRadius = FMath::Max(1.0f, InTraceRadius);
	DistanceTraveled = 0.0f;
	bFinished = false;
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);

	if (CollisionSphere)
	{
		CollisionSphere->SetSphereRadius(TraceRadius);
	}

	const float RadiusScale = TraceRadius / 85.0f;
	if (SlashVisual)
	{
		SlashVisual->SetRelativeScale3D(FVector(0.08f, 1.4f * RadiusScale, 0.55f * RadiusScale));
	}
	if (SlashGlow)
	{
		SlashGlow->SetAttenuationRadius(FMath::Max(160.0f, TraceRadius * 3.0f));
	}

	SetActorRotation(TravelDirection.Rotation());
	SetLifeSpan((MaxTravelDistance / ProjectileSpeed) + 0.35f);
}

void AAetherSlashProjectile::SetImpactAssets(UParticleSystem* InImpactEffect, USoundBase* InImpactSound, float InEffectScale, float InSoundVolume)
{
	ImpactEffect = InImpactEffect;
	ImpactSound = InImpactSound;
	ImpactEffectScale = FMath::Max(0.01f, InEffectScale);
	ImpactSoundVolume = FMath::Max(0.0f, InSoundVolume);
}

void AAetherSlashProjectile::SetOwningProjectilePool(UAetherProjectilePoolSubsystem* InOwningProjectilePool)
{
	OwningProjectilePool = InOwningProjectilePool;
}

void AAetherSlashProjectile::SweepForward(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World || bFinished || DeltaTime <= 0.0f)
	{
		return;
	}

	const float RemainingDistance = MaxTravelDistance - DistanceTraveled;
	if (RemainingDistance <= 0.0f)
	{
		FinishProjectile(false);
		return;
	}

	const float StepDistance = FMath::Min(ProjectileSpeed * DeltaTime, RemainingDistance);
	const FVector Start = GetActorLocation();
	const FVector End = Start + TravelDirection * StepDistance;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AetherSlashProjectileTrace), false, this);
	if (AActor* Causer = DamageCauser.Get())
	{
		QueryParams.AddIgnoredActor(Causer);
	}

	ReusableHitResults.Reset();
	ReusableHitResults.Reserve(8);
	const FCollisionShape Shape = FCollisionShape::MakeSphere(TraceRadius);
	const bool bHit = World->SweepMultiByChannel(ReusableHitResults, Start, End, FQuat::Identity, ECC_Pawn, Shape, QueryParams);

	const FColor TraceColor = bHit ? FColor(0, 180, 255) : FColor(0, 80, 160);
	if (bDrawProjectileDebug)
	{
		DrawDebugLine(World, Start, End, TraceColor, false, 0.12f, 0, 5.0f);
		DrawDebugSphere(World, End, TraceRadius, 16, TraceColor, false, 0.12f, 0, 2.5f);
	}

	if (bHit)
	{
		ShowProjectileDebugMessage(FString::Printf(TEXT("Aether Slash hit candidates: %d"), ReusableHitResults.Num()), FColor(0, 180, 255));

		const FAetherCombatTargetSelection TargetSelection =
			FAetherCombatTargetSelectionPolicy::BuildTargetSelection(ReusableHitResults, DamageCauser.Get(), nullptr, LockedTarget.Get());

		if (TargetSelection.IsLockedPriority())
		{
			ShowProjectileDebugMessage(TEXT("Aether Slash locked target priority"), FColor(0, 180, 255));
			if (TryApplyDamage(TargetSelection.PriorityTarget, true))
			{
				FinishProjectile(true);
				return;
			}
		}

		for (AActor* HitActor : TargetSelection.AdditionalTargets)
		{
			if (TryApplyDamage(HitActor, false))
			{
				FinishProjectile(true);
				return;
			}
		}
	}

	SetActorLocation(End);
	DistanceTraveled += StepDistance;

	if (DistanceTraveled >= MaxTravelDistance)
	{
		FinishProjectile(false);
	}
}

bool AAetherSlashProjectile::TryApplyDamage(AActor* TargetActor, bool bLockedTargetDamage)
{
	AActor* Causer = DamageCauser.Get();
	FAetherCombatDamageRequest DamageRequest;
	DamageRequest.TargetActor = TargetActor;
	DamageRequest.DamageCauser = Causer ? Causer : this;
	DamageRequest.DamageAmount = Damage;
	const FAetherCombatDamageResult DamageResult = FAetherCombatDamagePolicy::TryApplyDamage(DamageRequest);
	if (!DamageResult.WasApplied())
	{
		return false;
	}

	if (bLockedTargetDamage)
	{
		ShowProjectileDebugMessage(FString::Printf(TEXT("Aether Slash dealt %.0f damage to locked target"), Damage), FColor::Green);
	}
	else
	{
		ShowProjectileDebugMessage(FString::Printf(TEXT("Aether Slash dealt %.0f damage"), Damage), FColor::Green);
	}

	PlayImpactFeedback(TargetActor);
	return true;
}

void AAetherSlashProjectile::FinishProjectile(bool bHitTarget)
{
	if (bFinished)
	{
		return;
	}

	bFinished = true;
	SetLifeSpan(0.0f);
	DrawFinishFeedback(bHitTarget);
	if (!bHitTarget)
	{
		ShowProjectileDebugMessage(TEXT("Aether Slash missed"), FColor::Yellow);
	}
	else
	{
		ShowProjectileDebugMessage(TEXT("Aether Slash impact flash"), FColor(0, 180, 255));
	}

	ReleaseOrDestroy();
}

void AAetherSlashProjectile::ReleaseOrDestroy()
{
	if (UAetherProjectilePoolSubsystem* Pool = OwningProjectilePool.Get())
	{
		Pool->ReleaseAetherSlashProjectile(this);
		return;
	}

	Destroy();
}

void AAetherSlashProjectile::DrawFinishFeedback(bool bHitTarget) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FColor BurstColor = bHitTarget ? FColor(0, 220, 255) : FColor::Yellow;
	const float BurstRadius = bHitTarget ? TraceRadius * 1.35f : TraceRadius * 0.85f;
	const FVector Location = GetActorLocation();
	if (bDrawProjectileDebug)
	{
		DrawDebugSphere(World, Location, BurstRadius, 24, BurstColor, false, 0.35f, 0, 4.0f);
		DrawDebugPoint(World, Location, 18.0f, BurstColor, false, 0.35f, 0);
	}
}

void AAetherSlashProjectile::PlayImpactFeedback(AActor* TargetActor) const
{
	AAetherfallCharacter* Character = Cast<AAetherfallCharacter>(DamageCauser.Get());
	if (!Character)
	{
		return;
	}

	if (ImpactCameraKickStrength > 0.0f && ImpactCameraKickDuration > 0.0f)
	{
		Character->PlayCameraImpactFeedback(ImpactCameraKickStrength, ImpactCameraKickDuration);
		ShowProjectileDebugMessage(TEXT("Camera impact kick (Aether Slash)"), FColor::Silver);
	}

	if (ImpactHitStopDuration > 0.0f)
	{
		ApplyHitStopToActor(Character, ImpactHitStopDuration);
		if (TargetActor && TargetActor != Character)
		{
			ApplyHitStopToActor(TargetActor, ImpactHitStopDuration);
		}
		ShowProjectileDebugMessage(FString::Printf(TEXT("Hit stop (Aether Slash) / %.2f"), ImpactHitStopDuration), FColor::Silver);
	}

	UWorld* World = GetWorld();
	if (!World || !TargetActor)
	{
		return;
	}

	const FVector ImpactLocation = TargetActor->GetActorLocation();
	if (ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, ImpactEffect, ImpactLocation, FRotator::ZeroRotator, FVector(ImpactEffectScale));
	}
	if (ImpactSound)
	{
		UAetherAudioSettingsLibrary::SpawnSoundAtLocationForCategory(
			this,
			ImpactSound,
			ImpactLocation,
			EAetherAudioCategory::Sfx,
			ImpactSoundVolume);
	}
}

void AAetherSlashProjectile::ApplyHitStopToActor(AActor* Actor, float Duration) const
{
	UWorld* World = GetWorld();
	if (!Actor || !World || Duration <= 0.0f)
	{
		return;
	}

	const float AppliedTimeDilation = FMath::Clamp(ImpactHitStopTimeDilation, 0.01f, 1.0f);
	const float PreviousTimeDilation = Actor->CustomTimeDilation;
	const float RestoreTimeDilation = PreviousTimeDilation <= AppliedTimeDilation + KINDA_SMALL_NUMBER
		? 1.0f
		: PreviousTimeDilation;
	Actor->CustomTimeDilation = AppliedTimeDilation;

	TWeakObjectPtr<AActor> WeakActor = Actor;
	FTimerDelegate RestoreDelegate = FTimerDelegate::CreateLambda([WeakActor, RestoreTimeDilation]()
	{
		if (AActor* RestoredActor = WeakActor.Get())
		{
			RestoredActor->CustomTimeDilation = RestoreTimeDilation;
		}
	});

	FTimerHandle RestoreTimerHandle;
	World->GetTimerManager().SetTimer(RestoreTimerHandle, RestoreDelegate, Duration, false);
}

void AAetherSlashProjectile::ShowProjectileDebugMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherCombat] %s"), *Message);

	if (bShowProjectileScreenDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.25f, Color, Message);
	}
}
