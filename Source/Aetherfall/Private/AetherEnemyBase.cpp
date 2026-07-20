#include "AetherEnemyBase.h"

#include "AetherAudioSettingsLibrary.h"
#include "AetherCombatComponent.h"
#include "AetherEnemyActionPresentationPolicy.h"
#include "AetherEnemyAttackPatternPolicy.h"
#include "AetherGameModeBase.h"
#include "AetherHealthComponent.h"
#include "AetherfallCharacter.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimEnums.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AAetherEnemyBase::AAetherEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(38.0f, 88.0f);
	GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->MaxWalkSpeed = 260.0f;
	MovementComponent->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	MovementComponent->bOrientRotationToMovement = true;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	PrototypeBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeBody"));
	PrototypeBody->SetupAttachment(RootComponent);
	PrototypeBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PrototypeBody->SetRelativeLocation(FVector(0.0f, 0.0f, -42.0f));
	PrototypeBody->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.45f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeMesh.Succeeded())
	{
		PrototypeBody->SetStaticMesh(ConeMesh.Object);
	}

	WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponStaticMesh"));
	WeaponStaticMesh->SetupAttachment(GetMesh(), WeaponAttachSocketName);
	WeaponStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkeletalMesh"));
	WeaponSkeletalMesh->SetupAttachment(GetMesh(), WeaponAttachSocketName);
	WeaponSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HealthComponent = CreateDefaultSubobject<UAetherHealthComponent>(TEXT("HealthComponent"));

	InitializeDefaultArchetypeProfiles();
	ApplyEnemyArchetype(EnemyArchetype);
}

void AAetherEnemyBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshPrototypeVisualMode();
	RefreshWeaponAttachment();
}

void AAetherEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	RefreshPrototypeVisualMode();
	RefreshWeaponAttachment();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &AAetherEnemyBase::HandleHealthChanged);
		HealthComponent->OnDeath.AddDynamic(this, &AAetherEnemyBase::HandleDeath);
	}

	ApplyEnemyArchetype(EnemyArchetype);
	UpdateTarget();
	ApplyPrototypeAnimationRootMotionPolicy();
	LogEnemyAnimationConfiguration();
	UpdatePrototypeLoopAnimation();
	ShowEnemyDebugMessage(FString::Printf(TEXT("Prototype enemy spawned (%s)"), *GetEnemyArchetypeName().ToString()), FColor::Orange);
	AnnounceAurelPhaseOneIntro();
}

void AAetherEnemyBase::InitializeDefaultArchetypeProfiles()
{
	auto MakeAttack = [](const TCHAR* Name, float Damage, float Range, float Windup, float Recovery, float Padding, float Radius, float Weight, const TCHAR* DefenseHint = TEXT(""))
	{
		FAetherEnemyAttackPatternData Attack;
		Attack.PatternName = FName(Name);
		Attack.DefenseHintLabel = (DefenseHint && DefenseHint[0] != TEXT('\0')) ? FName(DefenseHint) : NAME_None;
		Attack.Damage = Damage;
		Attack.Range = Range;
		Attack.WindupDuration = Windup;
		Attack.RecoveryDuration = Recovery;
		Attack.RangePadding = Padding;
		Attack.TelegraphRadius = Radius;
		Attack.SelectionWeight = Weight;
		return Attack;
	};

	ArchetypeProfiles.Reset();

	FAetherEnemyArchetypeData& Skirmisher = ArchetypeProfiles.AddDefaulted_GetRef();
	Skirmisher.Archetype = EAetherEnemyArchetype::Skirmisher;
	Skirmisher.DisplayName = FName(TEXT("Skirmisher"));
	Skirmisher.CombatRoleLabel = FName(TEXT("Fast Pressure"));
	Skirmisher.ArchetypeColor = FLinearColor(0.15f, 0.82f, 1.0f, 1.0f);
	Skirmisher.MaxHealth = 80.0f;
	Skirmisher.MovementSpeed = 330.0f;
	Skirmisher.VisualScale = 0.9f;
	Skirmisher.AttackCooldown = 1.15f;
	Skirmisher.DetectionRange = 1300.0f;
	Skirmisher.StopDistance = 110.0f;
	Skirmisher.AttackStartRangeBuffer = 14.0f;
	Skirmisher.AttackWindupTurnSpeed = 13.0f;
	Skirmisher.EnemySpacingRadius = 145.0f;
	Skirmisher.EnemySpacingStrength = 1.0f;
	Skirmisher.AttackPatterns = {
		MakeAttack(TEXT("Quick"), 14.0f, 165.0f, 0.42f, 0.32f, 28.0f, 72.0f, 2.0f),
		MakeAttack(TEXT("Standard"), 18.0f, 170.0f, 0.62f, 0.42f, 30.0f, 90.0f, 0.8f)
	};

	FAetherEnemyArchetypeData& Vanguard = ArchetypeProfiles.AddDefaulted_GetRef();
	Vanguard.Archetype = EAetherEnemyArchetype::Vanguard;
	Vanguard.DisplayName = FName(TEXT("Vanguard"));
	Vanguard.CombatRoleLabel = FName(TEXT("Balanced"));
	Vanguard.ArchetypeColor = FLinearColor(1.0f, 0.72f, 0.25f, 1.0f);
	Vanguard.MaxHealth = 100.0f;
	Vanguard.MovementSpeed = 260.0f;
	Vanguard.VisualScale = 1.0f;
	Vanguard.AttackCooldown = 1.35f;
	Vanguard.DetectionRange = DetectionRange;
	Vanguard.StopDistance = StopDistance;
	Vanguard.AttackStartRangeBuffer = 16.0f;
	Vanguard.AttackWindupTurnSpeed = 11.0f;
	Vanguard.EnemySpacingRadius = 160.0f;
	Vanguard.EnemySpacingStrength = 1.0f;
	Vanguard.AttackPatterns = {
		MakeAttack(TEXT("Quick"), 16.0f, 165.0f, 0.42f, 0.32f, 24.0f, 72.0f, 1.45f),
		MakeAttack(TEXT("Standard"), AttackDamage, AttackRange, AttackWindupDuration, AttackRecoveryDuration, AttackRangePadding, AttackTelegraphRadius, 1.1f),
		MakeAttack(TEXT("Heavy"), 30.0f, 180.0f, 1.05f, 0.85f, 45.0f, 130.0f, 0.35f)
	};

	FAetherEnemyArchetypeData& Brute = ArchetypeProfiles.AddDefaulted_GetRef();
	Brute.Archetype = EAetherEnemyArchetype::Brute;
	Brute.DisplayName = FName(TEXT("Brute"));
	Brute.CombatRoleLabel = FName(TEXT("Heavy Breaker"));
	Brute.ArchetypeColor = FLinearColor(1.0f, 0.20f, 0.14f, 1.0f);
	Brute.MaxHealth = 150.0f;
	Brute.MovementSpeed = 185.0f;
	Brute.VisualScale = 1.25f;
	Brute.AttackCooldown = 1.95f;
	Brute.DetectionRange = 1150.0f;
	Brute.StopDistance = 145.0f;
	Brute.AttackStartRangeBuffer = 12.0f;
	Brute.AttackWindupTurnSpeed = 8.0f;
	Brute.EnemySpacingRadius = 190.0f;
	Brute.EnemySpacingStrength = 1.25f;
	Brute.AttackPatterns = {
		MakeAttack(TEXT("Quick"), 16.0f, 160.0f, 0.62f, 0.50f, 28.0f, 85.0f, 0.3f),
		MakeAttack(TEXT("Standard"), 24.0f, 185.0f, 0.84f, 0.66f, 38.0f, 110.0f, 0.95f),
		MakeAttack(TEXT("Heavy"), 34.0f, 200.0f, 1.35f, 1.10f, 55.0f, 150.0f, 0.95f)
	};

	FAetherEnemyArchetypeData& Cael = ArchetypeProfiles.AddDefaulted_GetRef();
	Cael.Archetype = EAetherEnemyArchetype::Cael;
	Cael.DisplayName = FName(TEXT("Cael"));
	Cael.CombatRoleLabel = FName(TEXT("Crypt Elite"));
	Cael.ArchetypeColor = FLinearColor(0.62f, 0.36f, 1.0f, 1.0f);
	Cael.MaxHealth = 260.0f;
	Cael.MovementSpeed = 215.0f;
	Cael.VisualScale = 1.35f;
	Cael.AttackCooldown = 1.65f;
	Cael.DetectionRange = 1450.0f;
	Cael.StopDistance = 150.0f;
	Cael.AttackStartRangeBuffer = 8.0f;
	Cael.AttackWindupTurnSpeed = 9.0f;
	Cael.EnemySpacingRadius = 220.0f;
	Cael.EnemySpacingStrength = 1.35f;
	Cael.AttackPatterns = {
		MakeAttack(TEXT("Oath Cut"), 22.0f, 175.0f, 0.58f, 0.46f, 30.0f, 90.0f, 0.85f),
		MakeAttack(TEXT("Seal Breaker"), 34.0f, 205.0f, 1.08f, 0.82f, 48.0f, 135.0f, 1.15f),
		MakeAttack(TEXT("Crypt Cleave"), 42.0f, 225.0f, 1.42f, 1.05f, 58.0f, 165.0f, 0.75f)
	};

	FAetherEnemyArchetypeData& Aurel = ArchetypeProfiles.AddDefaulted_GetRef();
	Aurel.Archetype = EAetherEnemyArchetype::Aurel;
	Aurel.DisplayName = FName(TEXT("Aurel"));
	Aurel.CombatRoleLabel = FName(TEXT("Hollow Warden"));
	Aurel.ArchetypeColor = FLinearColor(0.18f, 0.76f, 1.0f, 1.0f);
	Aurel.MaxHealth = 420.0f;
	Aurel.MovementSpeed = 225.0f;
	Aurel.VisualScale = 1.55f;
	Aurel.AttackCooldown = 1.55f;
	Aurel.DetectionRange = 1650.0f;
	Aurel.StopDistance = 165.0f;
	Aurel.AttackStartRangeBuffer = 10.0f;
	Aurel.AttackWindupTurnSpeed = 8.5f;
	Aurel.EnemySpacingRadius = 260.0f;
	Aurel.EnemySpacingStrength = 1.5f;
	Aurel.HitReactionDuration = 0.16f;
	Aurel.HitKnockbackStrength = 120.0f;
	Aurel.HitKnockbackUpwardStrength = 18.0f;
	Aurel.ParryStaggerDuration = 1.65f;
	Aurel.ParryStaggerKnockbackStrength = 115.0f;
	Aurel.ParryStaggerKnockbackUpwardStrength = 18.0f;
	Aurel.bResistAttackInterruptsDuringWindup = true;
	Aurel.AttackPatterns = {
		MakeAttack(TEXT("Warden Cut"), 26.0f, 185.0f, 0.62f, 0.48f, 32.0f, 100.0f, 1.0f, TEXT("GUARD OR PARRY")),
		MakeAttack(TEXT("Oath Sweep"), 38.0f, 220.0f, 1.06f, 0.82f, 52.0f, 150.0f, 1.0f, TEXT("DODGE WIDE")),
		MakeAttack(TEXT("Rift Breaker"), 48.0f, 245.0f, 1.48f, 1.12f, 62.0f, 180.0f, 1.0f, TEXT("PARRY LATE"))
	};

	AurelPhaseTwoAttackPatterns = {
		MakeAttack(TEXT("Warden Rush"), 32.0f, 255.0f, 0.72f, 0.54f, 55.0f, 145.0f, 1.0f, TEXT("DODGE LATE")),
		MakeAttack(TEXT("Rift Feint"), 24.0f, 190.0f, 0.44f, 0.34f, 34.0f, 95.0f, 1.0f, TEXT("GUARD FAST")),
		MakeAttack(TEXT("Aether Cleave"), 54.0f, 285.0f, 1.18f, 0.90f, 72.0f, 210.0f, 1.0f, TEXT("PARRY OR RUN"))
	};
}

const FAetherEnemyArchetypeData* AAetherEnemyBase::FindArchetypeProfile(EAetherEnemyArchetype ArchetypeToFind) const
{
	return ArchetypeProfiles.FindByPredicate([ArchetypeToFind](const FAetherEnemyArchetypeData& Profile)
	{
		return Profile.Archetype == ArchetypeToFind;
	});
}

void AAetherEnemyBase::ApplyEnemyArchetype(EAetherEnemyArchetype NewArchetype)
{
	EnemyArchetype = NewArchetype;

	const FAetherEnemyArchetypeData* Profile = FindArchetypeProfile(NewArchetype);
	if (!Profile)
	{
		Profile = FindArchetypeProfile(EAetherEnemyArchetype::Vanguard);
	}
	if (!Profile)
	{
		return;
	}

	DetectionRange = Profile->DetectionRange;
	AttackCooldown = Profile->AttackCooldown;
	StopDistance = Profile->StopDistance;
	AttackStartRangeBuffer = Profile->AttackStartRangeBuffer;
	AttackWindupTurnSpeed = Profile->AttackWindupTurnSpeed;
	EnemySpacingRadius = Profile->EnemySpacingRadius;
	EnemySpacingStrength = Profile->EnemySpacingStrength;
	HitReactionDuration = Profile->HitReactionDuration;
	HitKnockbackStrength = Profile->HitKnockbackStrength;
	HitKnockbackUpwardStrength = Profile->HitKnockbackUpwardStrength;
	ParryStaggerDuration = Profile->ParryStaggerDuration;
	ParryStaggerKnockbackStrength = Profile->ParryStaggerKnockbackStrength;
	ParryStaggerKnockbackUpwardStrength = Profile->ParryStaggerKnockbackUpwardStrength;
	bResistAttackInterruptsDuringWindup = Profile->bResistAttackInterruptsDuringWindup;
	AttackPatterns = Profile->AttackPatterns;
	ResetAurelBossRuntimeState();
	if (AttackPatterns.Num() > 0)
	{
		ActiveAttackPattern = AttackPatterns[0];
	}

	if (HealthComponent)
	{
		HealthComponent->SetMaxHealth(Profile->MaxHealth, true);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = Profile->MovementSpeed;
	}

	if (PrototypeBody)
	{
		PrototypeBody->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.45f) * Profile->VisualScale);
	}

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetRelativeScale3D(FVector(Profile->VisualScale));
	}
}

void AAetherEnemyBase::RefreshWeaponAttachment()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	if (WeaponStaticMesh)
	{
		WeaponStaticMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::KeepRelativeTransform, WeaponAttachSocketName);
		WeaponStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (WeaponSkeletalMesh)
	{
		WeaponSkeletalMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::KeepRelativeTransform, WeaponAttachSocketName);
		WeaponSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

FName AAetherEnemyBase::GetEnemyArchetypeName() const
{
	if (const FAetherEnemyArchetypeData* Profile = FindArchetypeProfile(EnemyArchetype))
	{
		return Profile->DisplayName;
	}

	switch (EnemyArchetype)
	{
	case EAetherEnemyArchetype::Skirmisher:
		return FName(TEXT("Skirmisher"));
	case EAetherEnemyArchetype::Brute:
		return FName(TEXT("Brute"));
	case EAetherEnemyArchetype::Cael:
		return FName(TEXT("Cael"));
	case EAetherEnemyArchetype::Aurel:
		return FName(TEXT("Aurel"));
	case EAetherEnemyArchetype::Vanguard:
	default:
		return FName(TEXT("Vanguard"));
	}
}

FName AAetherEnemyBase::GetEnemyCombatRoleLabel() const
{
	if (const FAetherEnemyArchetypeData* Profile = FindArchetypeProfile(EnemyArchetype))
	{
		return Profile->CombatRoleLabel;
	}

	return FName(TEXT("Balanced"));
}

FLinearColor AAetherEnemyBase::GetEnemyArchetypeColor() const
{
	if (const FAetherEnemyArchetypeData* Profile = FindArchetypeProfile(EnemyArchetype))
	{
		return Profile->ArchetypeColor;
	}

	return FLinearColor(1.0f, 0.72f, 0.25f, 1.0f);
}

FName AAetherEnemyBase::GetActiveAttackPatternName() const
{
	return GetActiveAttackPattern().PatternName;
}

FName AAetherEnemyBase::GetActiveAttackDefenseHintLabel() const
{
	return GetActiveAttackPattern().DefenseHintLabel;
}

bool AAetherEnemyBase::IsBossPhaseOne() const
{
	return AurelBossPhase.IsPhaseOne(EnemyArchetype);
}

bool AAetherEnemyBase::IsBossPhaseTwo() const
{
	return AurelBossPhase.IsPhaseTwo(EnemyArchetype);
}

void AAetherEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead)
	{
		return;
	}

	UpdateTarget();
	if (bIsAttackWindingUp)
	{
		UpdateAttackWindupFacing(DeltaTime);
		return;
	}

	if (bIsAttackRecovering || bIsHitReacting || bIsParryStaggered || bIsExecutionSuppressed || AurelBossPhase.IsPhaseShifting())
	{
		return;
	}

	ChaseTarget(DeltaTime);
	UpdatePrototypeLoopAnimation();
	TryAttackTarget();
}

void AAetherEnemyBase::UpdateTarget()
{
	if (TargetCharacter.IsValid())
	{
		return;
	}

	TargetCharacter = Cast<AAetherfallCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

void AAetherEnemyBase::ChaseTarget(float DeltaTime)
{
	AAetherfallCharacter* Target = TargetCharacter.Get();
	if (!Target || !Target->GetHealthComponent() || Target->GetHealthComponent()->IsDead())
	{
		return;
	}

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	const float Distance = ToTarget.Size2D();
	if (Distance > DetectionRange)
	{
		return;
	}

	const FVector Direction = ToTarget.GetSafeNormal2D();
	FVector DesiredMovement = Distance > StopDistance ? Direction : FVector::ZeroVector;

	if (bUseEnemySpacing)
	{
		const FVector SeparationDirection = CalculateEnemySeparationDirection();
		if (!SeparationDirection.IsNearlyZero())
		{
			const float SeparationScale = Distance <= StopDistance ? EnemySpacingStrength : EnemySpacingStrength * 0.65f;
			DesiredMovement += SeparationDirection * SeparationScale;
		}
	}

	if (!DesiredMovement.IsNearlyZero())
	{
		AddMovementInput(DesiredMovement.GetSafeNormal2D(), 1.0f);
	}

	const FVector FacingDirection = !Direction.IsNearlyZero() ? Direction : DesiredMovement.GetSafeNormal2D();
	if (!FacingDirection.IsNearlyZero())
	{
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), FacingDirection.Rotation(), DeltaTime, 8.0f));
	}
}

void AAetherEnemyBase::TryAttackTarget()
{
	AAetherfallCharacter* Target = TargetCharacter.Get();
	UWorld* World = GetWorld();
	if (!Target || !World || !Target->GetHealthComponent() || Target->GetHealthComponent()->IsDead())
	{
		return;
	}

	if (bIsAttackWindingUp || bIsAttackRecovering || bIsHitReacting || bIsParryStaggered || bIsExecutionSuppressed || AurelBossPhase.IsPhaseShifting())
	{
		return;
	}

	const float DistanceToTarget = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());
	const FAetherEnemyAttackPatternData* SelectedAttackPattern = SelectAttackPattern(DistanceToTarget);
	if (!SelectedAttackPattern)
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackCooldown)
	{
		return;
	}

	if (AAetherGameModeBase* GameMode = World->GetAuthGameMode<AAetherGameModeBase>())
	{
		if (!GameMode->TryAcquirePrototypeEnemyAttackSlot(this))
		{
			return;
		}
	}

	BeginAttackWindup(*SelectedAttackPattern);
}

void AAetherEnemyBase::BeginAttackWindup(const FAetherEnemyAttackPatternData& AttackPattern)
{
	UWorld* World = GetWorld();
	AAetherfallCharacter* Target = TargetCharacter.Get();
	if (!World || !Target || bIsExecutionSuppressed)
	{
		ReleaseAttackSlot();
		return;
	}

	LastAttackTime = World->GetTimeSeconds();
	ActiveAttackPattern = AttackPattern;
	bIsAttackWindingUp = true;
	bIsAttackRecovering = false;
	GetCharacterMovement()->StopMovementImmediately();
	SetActorRotation((Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D().Rotation());

	ShowEnemyDebugMessage(FString::Printf(TEXT("Enemy attack windup (%s / %s)"), *ActiveAttackPattern.PatternName.ToString(), *GetEnemyArchetypeName().ToString()), GetEnemyArchetypeColor().ToFColor(true));
	const FString AttackReason = FString::Printf(TEXT("Enemy attack %s"), *ActiveAttackPattern.PatternName.ToString());
	PlayEnemyActionVisual(
		FAetherEnemyActionPresentationPolicy::BuildAttackVisual(
			ActiveAttackPattern,
			BuildEnemyActionVisualAssets(),
			BuildEnemyActionVisualTuning()),
		AttackReason);
	PlayEnemySound(FAetherEnemyActionPresentationPolicy::SelectAttackWindupSound(ActiveAttackPattern, BuildEnemyActionSoundSet()));
	DrawAttackTelegraph();
	World->GetTimerManager().SetTimer(AttackWindupTimerHandle, this, &AAetherEnemyBase::ResolveAttack, ActiveAttackPattern.WindupDuration, false);
}

void AAetherEnemyBase::UpdateAttackWindupFacing(float DeltaTime)
{
	AAetherfallCharacter* Target = TargetCharacter.Get();
	if (!Target || !Target->GetHealthComponent() || Target->GetHealthComponent()->IsDead() || AttackWindupTurnSpeed <= 0.0f)
	{
		return;
	}

	const FVector Direction = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), Direction.Rotation(), DeltaTime, AttackWindupTurnSpeed));
	}
}

void AAetherEnemyBase::ResolveAttack()
{
	bIsAttackWindingUp = false;

	UWorld* World = GetWorld();
	if (bIsDead || !World || bIsExecutionSuppressed)
	{
		ReleaseAttackSlot();
		return;
	}

	AAetherfallCharacter* Target = TargetCharacter.Get();
	const FAetherEnemyAttackPatternData& AttackPattern = GetActiveAttackPattern();
	if (Target && IsTargetWithinAttackRange(AttackPattern, AttackPattern.RangePadding))
	{
		SetActorRotation((Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D().Rotation());
		ShowEnemyDebugMessage(FString::Printf(TEXT("Enemy attack hit (%s / %s)"), *AttackPattern.PatternName.ToString(), *GetEnemyArchetypeName().ToString()), FColor::Red);
		PlayEnemySound(AttackHitSound.Get());

		if (UAetherCombatComponent* TargetCombat = Target->GetCombatComponent())
		{
			TargetCombat->ReceiveIncomingHit(AttackPattern.Damage, this);
			if (bIsParryStaggered)
			{
				return;
			}
		}
	}
	else
	{
		ShowEnemyDebugMessage(FString::Printf(TEXT("Enemy attack missed (%s / %s)"), *AttackPattern.PatternName.ToString(), *GetEnemyArchetypeName().ToString()), FColor::Yellow);
		PlayEnemySound(AttackMissSound.Get(), 0.65f);
	}

	bIsAttackRecovering = true;
	World->GetTimerManager().SetTimer(AttackRecoveryTimerHandle, this, &AAetherEnemyBase::EndAttackRecovery, AttackPattern.RecoveryDuration, false);
}

void AAetherEnemyBase::EndAttackRecovery()
{
	bIsAttackRecovering = false;
	ReleaseAttackSlot();
	UpdatePrototypeLoopAnimation();
}

void AAetherEnemyBase::ReleaseAttackSlot()
{
	if (UWorld* World = GetWorld())
	{
		if (AAetherGameModeBase* GameMode = World->GetAuthGameMode<AAetherGameModeBase>())
		{
			GameMode->ReleasePrototypeEnemyAttackSlot(this);
		}
	}
}

void AAetherEnemyBase::BeginHitReaction(AActor* DamageCauser)
{
	UWorld* World = GetWorld();
	if (bIsDead || !World)
	{
		return;
	}

	if (bIsParryStaggered)
	{
		return;
	}

	if (AurelBossPhase.IsPhaseShifting())
	{
		ShowEnemyDebugMessage(
			TEXT("Aurel phase shift poise held"),
			GetEnemyArchetypeColor().ToFColor(true));
		PlayEnemySound(HitReactionSound.Get(), 0.55f);
		return;
	}

	if (bIsAttackWindingUp && bResistAttackInterruptsDuringWindup)
	{
		const FString PhaseLabel = AurelBossPhase.IsPhaseTwo(EnemyArchetype) ? TEXT("phase II") : TEXT("phase I");
		ShowEnemyDebugMessage(
			FString::Printf(TEXT("%s %s poise held (%s)"), *GetEnemyArchetypeName().ToString(), *PhaseLabel, *GetActiveAttackPatternName().ToString()),
			GetEnemyArchetypeColor().ToFColor(true));
		PlayEnemySound(HitReactionSound.Get(), 0.7f);
		return;
	}

	const bool bInterruptedAttack = bIsAttackWindingUp || bIsAttackRecovering;
	if (bInterruptedAttack)
	{
		World->GetTimerManager().ClearTimer(AttackWindupTimerHandle);
		World->GetTimerManager().ClearTimer(AttackRecoveryTimerHandle);
		ReleaseAttackSlot();
		ShowEnemyDebugMessage(TEXT("Enemy attack interrupted"), FColor::Yellow);
	}

	bIsAttackWindingUp = false;
	bIsAttackRecovering = false;
	bIsHitReacting = true;
	LastAttackTime = World->GetTimeSeconds();

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->StopMovementImmediately();

	const FVector KnockbackDirection = GetHitReactionDirection(DamageCauser);
	LaunchCharacter((KnockbackDirection * HitKnockbackStrength) + FVector(0.0f, 0.0f, HitKnockbackUpwardStrength), true, true);
	DrawDebugDirectionalArrow(
		World,
		GetActorLocation() + FVector(0.0f, 0.0f, 40.0f),
		GetActorLocation() + FVector(0.0f, 0.0f, 40.0f) + KnockbackDirection * 140.0f,
		35.0f,
		FColor::Magenta,
		false,
		HitReactionDuration,
		0,
		5.0f);

	ShowEnemyDebugMessage(TEXT("Enemy hit reaction / knockback"), FColor::Magenta);
	PlayEnemyActionVisual(
		FAetherEnemyActionPresentationPolicy::BuildHitReactionVisual(
			BuildEnemyActionVisualAssets(),
			BuildEnemyActionVisualTuning()),
		TEXT("Enemy hit reaction"));
	PlayEnemySound(HitReactionSound.Get(), 0.85f);
	World->GetTimerManager().SetTimer(HitReactionTimerHandle, this, &AAetherEnemyBase::EndHitReaction, HitReactionDuration, false);
}

void AAetherEnemyBase::EndHitReaction()
{
	bIsHitReacting = false;
	ShowEnemyDebugMessage(TEXT("Enemy hit reaction ended"), FColor::Magenta);
	UpdatePrototypeLoopAnimation();
}

void AAetherEnemyBase::SetPendingExecutionDeathMontage(UAnimMontage* NewDeathMontage)
{
	PendingExecutionDeathMontage = NewDeathMontage;
}

void AAetherEnemyBase::ApplyParryStagger(AActor* StaggerCauser)
{
	UWorld* World = GetWorld();
	if (bIsDead || !World || ParryStaggerDuration <= 0.0f)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(AttackWindupTimerHandle);
	World->GetTimerManager().ClearTimer(AttackRecoveryTimerHandle);
	World->GetTimerManager().ClearTimer(HitReactionTimerHandle);
	World->GetTimerManager().ClearTimer(ParryStaggerTimerHandle);
	ReleaseAttackSlot();

	bIsAttackWindingUp = false;
	bIsAttackRecovering = false;
	bIsHitReacting = false;
	bIsParryStaggered = true;
	LastAttackTime = World->GetTimeSeconds();

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->StopMovementImmediately();

	const FVector StaggerDirection = GetHitReactionDirection(StaggerCauser);
	LaunchCharacter((StaggerDirection * ParryStaggerKnockbackStrength) + FVector(0.0f, 0.0f, ParryStaggerKnockbackUpwardStrength), true, true);

	const FVector DebugLocation = GetActorLocation() + FVector(0.0f, 0.0f, 70.0f);
	DrawDebugSphere(World, DebugLocation, 105.0f, 24, FColor::Purple, false, ParryStaggerDuration, 0, 4.0f);
	DrawDebugDirectionalArrow(
		World,
		GetActorLocation() + FVector(0.0f, 0.0f, 45.0f),
		GetActorLocation() + FVector(0.0f, 0.0f, 45.0f) + StaggerDirection * 120.0f,
		30.0f,
		FColor::Purple,
		false,
		ParryStaggerDuration,
		0,
		5.0f);

	ShowEnemyDebugMessage(TEXT("Enemy parry staggered"), FColor::Purple);
	PlayEnemyActionVisual(
		FAetherEnemyActionPresentationPolicy::BuildParryStaggerVisual(
			BuildEnemyActionVisualAssets(),
			BuildEnemyActionVisualTuning()),
		TEXT("Enemy parry stagger"));
	PlayEnemySound(ParryStaggerSound.Get());
	World->GetTimerManager().SetTimer(ParryStaggerTimerHandle, this, &AAetherEnemyBase::EndParryStagger, ParryStaggerDuration, false);
}

void AAetherEnemyBase::RefreshParryStagger(float StaggerDuration)
{
	UWorld* World = GetWorld();
	if (bIsDead || !World || StaggerDuration <= 0.0f || bIsExecutionSuppressed)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(AttackWindupTimerHandle);
	World->GetTimerManager().ClearTimer(AttackRecoveryTimerHandle);
	World->GetTimerManager().ClearTimer(HitReactionTimerHandle);
	World->GetTimerManager().ClearTimer(ParryStaggerTimerHandle);
	ReleaseAttackSlot();

	bIsAttackWindingUp = false;
	bIsAttackRecovering = false;
	bIsHitReacting = false;
	bIsParryStaggered = true;
	LastAttackTime = World->GetTimeSeconds();

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->StopMovementImmediately();

	ShowEnemyDebugMessage(FString::Printf(TEXT("Enemy execution stagger refreshed %.2f"), StaggerDuration), FColor::Purple);
	World->GetTimerManager().SetTimer(ParryStaggerTimerHandle, this, &AAetherEnemyBase::EndParryStagger, StaggerDuration, false);
}

void AAetherEnemyBase::ApplyExecutionSuppression(AActor* SuppressionCauser, float SuppressionDuration)
{
	UWorld* World = GetWorld();
	if (bIsDead || !World || SuppressionDuration <= 0.0f)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(AttackWindupTimerHandle);
	World->GetTimerManager().ClearTimer(AttackRecoveryTimerHandle);
	World->GetTimerManager().ClearTimer(HitReactionTimerHandle);
	World->GetTimerManager().ClearTimer(ParryStaggerTimerHandle);
	World->GetTimerManager().ClearTimer(ExecutionSuppressionTimerHandle);
	ReleaseAttackSlot();

	bIsAttackWindingUp = false;
	bIsAttackRecovering = false;
	bIsHitReacting = false;
	bIsParryStaggered = false;
	bIsExecutionSuppressed = true;
	LastAttackTime = World->GetTimeSeconds();

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->StopMovementImmediately();

	const FVector PauseDirection = GetHitReactionDirection(SuppressionCauser);
	DrawDebugDirectionalArrow(
		World,
		GetActorLocation() + FVector(0.0f, 0.0f, 55.0f),
		GetActorLocation() + FVector(0.0f, 0.0f, 55.0f) + PauseDirection * 100.0f,
		26.0f,
		FColor::Yellow,
		false,
		SuppressionDuration,
		0,
		4.0f);

	ShowEnemyDebugMessage(TEXT("Enemy execution suppressed"), FColor::Yellow);
	World->GetTimerManager().SetTimer(ExecutionSuppressionTimerHandle, this, &AAetherEnemyBase::EndExecutionSuppression, SuppressionDuration, false);
}

void AAetherEnemyBase::EndParryStagger()
{
	bIsParryStaggered = false;
	ShowEnemyDebugMessage(TEXT("Enemy stagger ended"), FColor::Purple);
	UpdatePrototypeLoopAnimation();
}

void AAetherEnemyBase::EndExecutionSuppression()
{
	bIsExecutionSuppressed = false;
	ShowEnemyDebugMessage(TEXT("Enemy execution suppression ended"), FColor::Yellow);
	UpdatePrototypeLoopAnimation();
}

FVector AAetherEnemyBase::GetHitReactionDirection(AActor* DamageCauser) const
{
	if (DamageCauser && DamageCauser != this)
	{
		const FVector AwayFromCauser = GetActorLocation() - DamageCauser->GetActorLocation();
		const FVector Direction = AwayFromCauser.GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			return Direction;
		}
	}

	return -GetActorForwardVector().GetSafeNormal2D();
}

void AAetherEnemyBase::DrawAttackTelegraph() const
{
	if (!bDrawAttackTelegraphDebug)
	{
		return;
	}

	UWorld* World = GetWorld();
	AAetherfallCharacter* Target = TargetCharacter.Get();
	if (!World || !Target)
	{
		return;
	}

	const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
	const FVector End = Target->GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
	const FAetherEnemyAttackPatternData& AttackPattern = GetActiveAttackPattern();
	const FColor TelegraphColor = GetEnemyArchetypeColor().ToFColor(true);
	const float ScaledRadius = AttackPattern.TelegraphRadius * AttackTelegraphDebugRadiusScale;
	DrawDebugLine(World, Start, End, TelegraphColor, false, AttackPattern.WindupDuration, 0, 2.0f);
	DrawDebugSphere(World, End, ScaledRadius, 16, TelegraphColor, false, AttackPattern.WindupDuration, 0, 2.0f);
}

const FAetherEnemyAttackPatternData* AAetherEnemyBase::SelectAttackPattern(float DistanceToTarget)
{
	if (EnemyArchetype == EAetherEnemyArchetype::Aurel)
	{
		return AurelBossPhase.SelectAurelAttackPattern(
			AttackPatterns,
			DistanceToTarget,
			AttackRange,
			AttackStartRangeBuffer,
			ActiveAttackPattern);
	}

	return FAetherEnemyAttackPatternPolicy::SelectWeightedPattern(
		AttackPatterns,
		DistanceToTarget,
		AttackRange,
		AttackStartRangeBuffer,
		ActiveAttackPattern);
}

const FAetherEnemyAttackPatternData& AAetherEnemyBase::GetActiveAttackPattern() const
{
	if (!ActiveAttackPattern.PatternName.IsNone())
	{
		return ActiveAttackPattern;
	}

	if (AttackPatterns.Num() > 0)
	{
		return AttackPatterns[0];
	}

	return ActiveAttackPattern;
}

FVector AAetherEnemyBase::CalculateEnemySeparationDirection() const
{
	UWorld* World = GetWorld();
	if (!World || EnemySpacingRadius <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	FVector Separation = FVector::ZeroVector;
	const FVector CurrentLocation = GetActorLocation();
	for (TActorIterator<AAetherEnemyBase> EnemyIt(World); EnemyIt; ++EnemyIt)
	{
		const AAetherEnemyBase* OtherEnemy = *EnemyIt;
		if (!OtherEnemy || OtherEnemy == this || OtherEnemy->bIsDead)
		{
			continue;
		}

		const FVector AwayFromOther = CurrentLocation - OtherEnemy->GetActorLocation();
		const float Distance = AwayFromOther.Size2D();
		if (Distance <= KINDA_SMALL_NUMBER || Distance >= EnemySpacingRadius)
		{
			continue;
		}

		const float Weight = 1.0f - (Distance / EnemySpacingRadius);
		Separation += AwayFromOther.GetSafeNormal2D() * Weight;
	}

	return Separation.GetSafeNormal2D();
}

float AAetherEnemyBase::GetMaxAttackRange() const
{
	float MaxRange = AttackRange;
	for (const FAetherEnemyAttackPatternData& AttackPattern : AttackPatterns)
	{
		MaxRange = FMath::Max(MaxRange, AttackPattern.Range);
	}

	return MaxRange;
}

bool AAetherEnemyBase::IsTargetWithinAttackRange(const FAetherEnemyAttackPatternData& AttackPattern, float ExtraPadding) const
{
	const AAetherfallCharacter* Target = TargetCharacter.Get();
	if (!Target || !Target->GetHealthComponent() || Target->GetHealthComponent()->IsDead())
	{
		return false;
	}

	return FVector::Dist2D(GetActorLocation(), Target->GetActorLocation()) <= AttackPattern.Range + ExtraPadding;
}

bool AAetherEnemyBase::IsTargetAttackable(float RangePadding) const
{
	const AAetherfallCharacter* Target = TargetCharacter.Get();
	if (!Target || !Target->GetHealthComponent() || Target->GetHealthComponent()->IsDead())
	{
		return false;
	}

	return FVector::Dist2D(GetActorLocation(), Target->GetActorLocation()) <= GetMaxAttackRange() + RangePadding;
}

void AAetherEnemyBase::AnnounceAurelPhaseOneIntro()
{
	RegisterAurelBossPhaseFeedback(
		AurelBossPhase.EvaluatePhaseOneIntro(EnemyArchetype),
		GetEnemyArchetypeColor().ToFColor(true));
}

bool AAetherEnemyBase::TryStartAurelPhaseTwo(float CurrentHealth, float MaxHealth, AActor* DamageCauser)
{
	if (!GetWorld())
	{
		return false;
	}

	const FAetherAurelBossPhaseTransitionPlan TransitionPlan = AurelBossPhase.EvaluatePhaseTwoTransition(
		EnemyArchetype,
		CurrentHealth,
		MaxHealth,
		BuildAurelBossPhaseTuning());
	if (!TransitionPlan.bShouldStartPhaseTwo)
	{
		return false;
	}

	BeginAurelPhaseTwo(DamageCauser, TransitionPlan);
	return true;
}

void AAetherEnemyBase::BeginAurelPhaseTwo(AActor* DamageCauser, const FAetherAurelBossPhaseTransitionPlan& TransitionPlan)
{
	UWorld* World = GetWorld();
	if (!World || !TransitionPlan.bShouldStartPhaseTwo)
	{
		return;
	}

	AurelBossPhase.BeginPhaseTwoTransition();
	bIsAttackWindingUp = false;
	bIsAttackRecovering = false;
	bIsHitReacting = false;
	bIsParryStaggered = false;
	LastAttackTime = World->GetTimeSeconds();
	World->GetTimerManager().ClearTimer(AttackWindupTimerHandle);
	World->GetTimerManager().ClearTimer(AttackRecoveryTimerHandle);
	World->GetTimerManager().ClearTimer(HitReactionTimerHandle);
	World->GetTimerManager().ClearTimer(ParryStaggerTimerHandle);
	World->GetTimerManager().ClearTimer(AurelPhaseShiftTimerHandle);
	ReleaseAttackSlot();

	if (AurelPhaseTwoAttackPatterns.Num() > 0)
	{
		AttackPatterns = AurelPhaseTwoAttackPatterns;
		ActiveAttackPattern = AttackPatterns[0];
	}

	AttackCooldown = FMath::Max(0.55f, AttackCooldown * TransitionPlan.AttackCooldownMultiplier);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->MaxWalkSpeed = FMath::Max(MovementComponent->MaxWalkSpeed, TransitionPlan.PhaseTwoMovementSpeed);
	}

	RegisterAurelBossPhaseFeedback(TransitionPlan.Feedback, GetEnemyArchetypeColor().ToFColor(true));
	if (AAetherGameModeBase* GameMode = World->GetAuthGameMode<AAetherGameModeBase>())
	{
		GameMode->TryStartPrototypeDialogue(FName(TEXT("BossPhase_CathedralBoss_Phase2")));
	}

	if (TransitionPlan.AetherReward > 0.0f)
	{
		if (AAetherfallCharacter* PlayerCharacter = Cast<AAetherfallCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		{
			if (UAetherCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
			{
				CombatComponent->GrantPrototypeAetherGauge(TransitionPlan.AetherReward, TEXT("Aurel phase shift"));
			}
		}
	}

	World->GetTimerManager().SetTimer(AurelPhaseShiftTimerHandle, this, &AAetherEnemyBase::EndAurelPhaseShift, TransitionPlan.PhaseShiftDuration, false);
}

void AAetherEnemyBase::EndAurelPhaseShift()
{
	AurelBossPhase.EndPhaseShift();
	ShowEnemyDebugMessage(TEXT("Aurel phase II attack rhythm active"), GetEnemyArchetypeColor().ToFColor(true));
}

void AAetherEnemyBase::AnnounceAurelLowHealth(float CurrentHealth, float MaxHealth)
{
	const FAetherAurelBossPhaseFeedback Feedback =
		AurelBossPhase.EvaluateLowHealth(EnemyArchetype, CurrentHealth, MaxHealth, BuildAurelBossPhaseTuning());
	RegisterAurelBossPhaseFeedback(Feedback, FColor::Orange);
	if (Feedback.bShouldAnnounce)
	{
		if (AAetherGameModeBase* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AAetherGameModeBase>() : nullptr)
		{
			GameMode->TryStartPrototypeDialogue(FName(TEXT("BossHealth_CathedralBoss_Low")));
		}
	}
}

void AAetherEnemyBase::ResetAurelBossRuntimeState()
{
	AurelBossPhase.Reset();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AurelPhaseShiftTimerHandle);
	}
}

FAetherAurelBossPhaseTuning AAetherEnemyBase::BuildAurelBossPhaseTuning() const
{
	FAetherAurelBossPhaseTuning Tuning;
	Tuning.PhaseTwoHealthThresholdPercent = AurelPhaseTwoHealthThresholdPercent;
	Tuning.PhaseShiftDuration = AurelPhaseShiftDuration;
	Tuning.PhaseShiftAetherReward = AurelPhaseShiftAetherReward;
	Tuning.PhaseTwoAttackCooldownMultiplier = AurelPhaseTwoAttackCooldownMultiplier;
	Tuning.PhaseTwoMovementSpeed = AurelPhaseTwoMovementSpeed;
	Tuning.LowHealthWarningPercent = AurelLowHealthWarningPercent;
	return Tuning;
}

void AAetherEnemyBase::RegisterAurelBossPhaseFeedback(const FAetherAurelBossPhaseFeedback& Feedback, const FColor& DebugColor) const
{
	if (!Feedback.bShouldAnnounce)
	{
		return;
	}

	ShowEnemyDebugMessage(Feedback.DebugMessage, DebugColor);
	if (AAetherGameModeBase* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AAetherGameModeBase>() : nullptr)
	{
		GameMode->RegisterPrototypeProgressFeedback(Feedback.ProgressMessage, Feedback.ProgressColor);
	}
}

void AAetherEnemyBase::HandleHealthChanged(UAetherHealthComponent* ChangedHealthComponent, float CurrentHealth, float MaxHealth, AActor* DamageCauser)
{
	if (!DamageCauser)
	{
		return;
	}

	ShowEnemyDebugMessage(FString::Printf(TEXT("%s HP: %.0f / %.0f"), *GetEnemyArchetypeName().ToString(), CurrentHealth, MaxHealth), FColor::Green);
	const bool bStartedPhaseTwo = TryStartAurelPhaseTwo(CurrentHealth, MaxHealth, DamageCauser);
	AnnounceAurelLowHealth(CurrentHealth, MaxHealth);
	if (CurrentHealth > 0.0f && !bStartedPhaseTwo)
	{
		BeginHitReaction(DamageCauser);
	}
}

float AAetherEnemyBase::PlayPendingExecutionDeathMontage()
{
	UAnimMontage* PendingDeathMontage = PendingExecutionDeathMontage.Get();
	PendingExecutionDeathMontage = nullptr;

	if (!PendingDeathMontage)
	{
		return 0.0f;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	USkeletalMesh* SkeletalMesh = MeshComponent ? MeshComponent->GetSkeletalMeshAsset() : nullptr;
	if (!MeshComponent || !SkeletalMesh)
	{
		ShowEnemyDebugMessage(TEXT("No Skeletal Mesh for enemy execution death montage"), FColor::Yellow);
		return 0.0f;
	}

	if (PendingDeathMontage->GetSkeleton() && SkeletalMesh->GetSkeleton() && PendingDeathMontage->GetSkeleton() != SkeletalMesh->GetSkeleton())
	{
		ShowEnemyDebugMessage(TEXT("Enemy death montage skeleton mismatch (Execution death)"), FColor::Yellow);
		return 0.0f;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		ShowEnemyDebugMessage(TEXT("No AnimInstance for enemy death montage (Execution death) - assign enemy AnimBP with DefaultSlot"), FColor::Yellow);
		return 0.0f;
	}

	const float PlayedLength = AnimInstance->Montage_Play(PendingDeathMontage);
	if (PlayedLength <= 0.0f)
	{
		ShowEnemyDebugMessage(TEXT("Enemy death montage failed (Execution death)"), FColor::Yellow);
		return 0.0f;
	}

	ShowEnemyDebugMessage(TEXT("Montage played (Enemy execution death)"), FColor::Silver);
	return PlayedLength;
}

float AAetherEnemyBase::PlayEnemyActionMontage(UAnimMontage* Montage, const FString& Reason) const
{
	if (!Montage)
	{
		return 0.0f;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	USkeletalMesh* SkeletalMesh = MeshComponent ? MeshComponent->GetSkeletalMeshAsset() : nullptr;
	if (!MeshComponent || !SkeletalMesh)
	{
		ShowEnemyDebugMessage(FString::Printf(TEXT("No Skeletal Mesh for enemy montage (%s)"), *Reason), FColor::Yellow);
		return 0.0f;
	}

	if (Montage->GetSkeleton() && SkeletalMesh->GetSkeleton() && Montage->GetSkeleton() != SkeletalMesh->GetSkeleton())
	{
		ShowEnemyDebugMessage(FString::Printf(TEXT("Enemy montage skeleton mismatch (%s)"), *Reason), FColor::Yellow);
		return 0.0f;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		ShowEnemyDebugMessage(FString::Printf(TEXT("No AnimInstance for enemy montage (%s) - assign enemy AnimBP with DefaultSlot"), *Reason), FColor::Yellow);
		return 0.0f;
	}

	const float PlayedLength = AnimInstance->Montage_Play(Montage);
	if (PlayedLength <= 0.0f)
	{
		ShowEnemyDebugMessage(FString::Printf(TEXT("Enemy montage failed (%s)"), *Reason), FColor::Yellow);
		return 0.0f;
	}

	ShowEnemyDebugMessage(FString::Printf(TEXT("Montage played (%s)"), *Reason), FColor::Silver);
	return PlayedLength;
}

FAetherEnemyActionVisualAssets AAetherEnemyBase::BuildEnemyActionVisualAssets() const
{
	FAetherEnemyActionVisualAssets VisualAssets;
	VisualAssets.QuickAttackMontage = QuickAttackMontage.Get();
	VisualAssets.StandardAttackMontage = StandardAttackMontage.Get();
	VisualAssets.HeavyAttackMontage = HeavyAttackMontage.Get();
	VisualAssets.HitReactionMontage = HitReactionMontage.Get();
	VisualAssets.ParryStaggerMontage = ParryStaggerMontage.Get();
	VisualAssets.DeathMontage = DeathMontage.Get();
	VisualAssets.QuickAttackAnimation = QuickAttackAnimation.Get();
	VisualAssets.StandardAttackAnimation = StandardAttackAnimation.Get();
	VisualAssets.HeavyAttackAnimation = HeavyAttackAnimation.Get();
	VisualAssets.HitReactionAnimation = HitReactionAnimation.Get();
	VisualAssets.ParryStaggerAnimation = ParryStaggerAnimation.Get();
	VisualAssets.DeathAnimation = DeathAnimation.Get();
	return VisualAssets;
}

FAetherEnemyActionVisualTuning AAetherEnemyBase::BuildEnemyActionVisualTuning() const
{
	FAetherEnemyActionVisualTuning VisualTuning;
	VisualTuning.bUsePrototypeEnemyAnimationDriver = bUsePrototypeEnemyAnimationDriver;
	VisualTuning.bPreferPrototypeFallbackActionAnimations = bPreferPrototypeFallbackActionAnimations;
	VisualTuning.MoveAnimationReferenceSpeed = PrototypeMoveAnimationReferenceSpeed;
	VisualTuning.MoveAnimationMinPlayRate = PrototypeMoveAnimationMinPlayRate;
	VisualTuning.MoveAnimationMaxPlayRate = PrototypeMoveAnimationMaxPlayRate;
	VisualTuning.IdleAnimationPlayRate = PrototypeIdleAnimationPlayRate;
	return VisualTuning;
}

FAetherEnemyActionSoundSet AAetherEnemyBase::BuildEnemyActionSoundSet() const
{
	FAetherEnemyActionSoundSet SoundSet;
	SoundSet.QuickAttackWindupSound = QuickAttackWindupSound.Get();
	SoundSet.StandardAttackWindupSound = StandardAttackWindupSound.Get();
	SoundSet.HeavyAttackWindupSound = HeavyAttackWindupSound.Get();
	SoundSet.AttackHitSound = AttackHitSound.Get();
	SoundSet.AttackMissSound = AttackMissSound.Get();
	SoundSet.HitReactionSound = HitReactionSound.Get();
	SoundSet.ParryStaggerSound = ParryStaggerSound.Get();
	SoundSet.DeathSound = DeathSound.Get();
	SoundSet.EnemySoundVolume = EnemySoundVolume;
	SoundSet.EnemySoundVolumeVariance = EnemySoundVolumeVariance;
	SoundSet.EnemySoundPitchMin = EnemySoundPitchMin;
	SoundSet.EnemySoundPitchMax = EnemySoundPitchMax;
	return SoundSet;
}

void AAetherEnemyBase::UpdatePrototypeLoopAnimation()
{
	if (!bUsePrototypeEnemyAnimationDriver || !GetMesh() || !GetMesh()->GetSkeletalMeshAsset())
	{
		return;
	}

	if (bIsDead || bIsAttackWindingUp || bIsAttackRecovering || bIsHitReacting || bIsParryStaggered || bIsExecutionSuppressed || AurelBossPhase.IsPhaseShifting())
	{
		return;
	}

	const float Speed2D = GetVelocity().Size2D();
	const bool bShouldMove = Speed2D > PrototypeMoveAnimationSpeedThreshold;
	UAnimationAsset* DesiredAnimation = bShouldMove ? MoveAnimation.Get() : IdleAnimation.Get();
	const FAetherEnemyActionVisualTuning VisualTuning = BuildEnemyActionVisualTuning();
	const float DesiredPlayRate = bShouldMove
		? FAetherEnemyActionPresentationPolicy::ResolveMoveAnimationPlayRate(Speed2D, VisualTuning)
		: VisualTuning.IdleAnimationPlayRate;
	PlayEnemyLoopAnimation(DesiredAnimation, bShouldMove ? TEXT("Enemy move") : TEXT("Enemy idle"), DesiredPlayRate);
}

float AAetherEnemyBase::PlayEnemyActionAnimation(UAnimationAsset* Animation, const FString& Reason)
{
	if (!bUsePrototypeEnemyAnimationDriver || !Animation)
	{
		return 0.0f;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	USkeletalMesh* SkeletalMesh = MeshComponent ? MeshComponent->GetSkeletalMeshAsset() : nullptr;
	if (!MeshComponent || !SkeletalMesh)
	{
		UE_LOG(LogTemp, Log, TEXT("[AetherEnemyAnimation] No Skeletal Mesh for fallback animation (%s)"), *Reason);
		return 0.0f;
	}

	if (Animation->GetSkeleton() && SkeletalMesh->GetSkeleton() && Animation->GetSkeleton() != SkeletalMesh->GetSkeleton())
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[AetherEnemyAnimation] Fallback animation skeleton mismatch (%s) / animation %s / mesh skeleton %s"),
			*Reason,
			*GetPathNameSafe(Animation->GetSkeleton()),
			*GetPathNameSafe(SkeletalMesh->GetSkeleton()));
		return 0.0f;
	}

	MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	MeshComponent->PlayAnimation(Animation, false);
	MeshComponent->SetPlayRate(FMath::Max(0.05f, PrototypeActionAnimationPlayRate));
	ApplyPrototypeAnimationRootMotionPolicy();
	CurrentPrototypeAnimation = Animation;
	bCurrentPrototypeAnimationLooping = false;
	CurrentPrototypeAnimationPlayRate = FMath::Max(0.05f, PrototypeActionAnimationPlayRate);

	const float PlayLength = Animation->GetPlayLength();
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[AetherEnemyAnimation] Fallback animation played (%s) / asset %s / length %.2f / play rate %.2f"),
		*Reason,
		*GetPathNameSafe(Animation),
		PlayLength,
		CurrentPrototypeAnimationPlayRate);
	return PlayLength;
}

float AAetherEnemyBase::PlayEnemyActionVisual(const FAetherEnemyActionVisualSelection& VisualSelection, const FString& Reason)
{
	if (VisualSelection.bPreferFallbackAnimation)
	{
		const float FallbackLength = PlayEnemyActionAnimation(VisualSelection.FallbackAnimation, Reason);
		if (FallbackLength > 0.0f)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("[AetherEnemyAnimation] Action visual used preferred fallback (%s) / montage %s"),
				*Reason,
				*GetPathNameSafe(VisualSelection.Montage));
			return FallbackLength;
		}

		UE_LOG(
			LogTemp,
			Log,
			TEXT("[AetherEnemyAnimation] Preferred fallback unavailable, trying montage (%s) / montage %s"),
			*Reason,
			*GetPathNameSafe(VisualSelection.Montage));
	}

	const float MontageLength = PlayEnemyActionMontage(VisualSelection.Montage, Reason);
	if (MontageLength > 0.0f)
	{
		return MontageLength;
	}

	return PlayEnemyActionAnimation(VisualSelection.FallbackAnimation, Reason);
}

float AAetherEnemyBase::PlayEnemyDeathVisual(const FAetherEnemyActionVisualSelection& VisualSelection, const FString& Reason)
{
	if (VisualSelection.bPreferFallbackAnimation)
	{
		const float PreferredFallbackLength = PlayEnemyActionAnimation(VisualSelection.FallbackAnimation, Reason);
		if (PreferredFallbackLength > 0.0f)
		{
			return PreferredFallbackLength;
		}
	}

	const float PendingExecutionDeathLength = PlayPendingExecutionDeathMontage();
	if (PendingExecutionDeathLength > 0.0f)
	{
		return PendingExecutionDeathLength;
	}

	return PlayEnemyActionVisual(VisualSelection, Reason);
}

void AAetherEnemyBase::PlayEnemyLoopAnimation(UAnimationAsset* Animation, const FString& Reason, float PlayRate)
{
	if (!bUsePrototypeEnemyAnimationDriver || !Animation)
	{
		return;
	}

	const float ClampedPlayRate = FMath::Max(0.05f, PlayRate);
	if (CurrentPrototypeAnimation == Animation && bCurrentPrototypeAnimationLooping)
	{
		if (FMath::Abs(CurrentPrototypeAnimationPlayRate - ClampedPlayRate) > 0.025f)
		{
			if (USkeletalMeshComponent* MeshComponent = GetMesh())
			{
				MeshComponent->SetPlayRate(ClampedPlayRate);
			}
			CurrentPrototypeAnimationPlayRate = ClampedPlayRate;
			UE_LOG(LogTemp, Log, TEXT("[AetherEnemyAnimation] Loop animation play rate updated (%s) / asset %s / play rate %.2f"), *Reason, *GetPathNameSafe(Animation), ClampedPlayRate);
		}
		return;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	USkeletalMesh* SkeletalMesh = MeshComponent ? MeshComponent->GetSkeletalMeshAsset() : nullptr;
	if (!MeshComponent || !SkeletalMesh)
	{
		UE_LOG(LogTemp, Log, TEXT("[AetherEnemyAnimation] No Skeletal Mesh for loop animation (%s)"), *Reason);
		return;
	}

	if (Animation->GetSkeleton() && SkeletalMesh->GetSkeleton() && Animation->GetSkeleton() != SkeletalMesh->GetSkeleton())
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("[AetherEnemyAnimation] Loop animation skeleton mismatch (%s) / animation %s / mesh skeleton %s"),
			*Reason,
			*GetPathNameSafe(Animation->GetSkeleton()),
			*GetPathNameSafe(SkeletalMesh->GetSkeleton()));
		return;
	}

	MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	MeshComponent->PlayAnimation(Animation, true);
	MeshComponent->SetPlayRate(ClampedPlayRate);
	ApplyPrototypeAnimationRootMotionPolicy();
	CurrentPrototypeAnimation = Animation;
	bCurrentPrototypeAnimationLooping = true;
	CurrentPrototypeAnimationPlayRate = ClampedPlayRate;

	UE_LOG(LogTemp, Log, TEXT("[AetherEnemyAnimation] Loop animation played (%s) / asset %s / play rate %.2f"), *Reason, *GetPathNameSafe(Animation), ClampedPlayRate);
}

void AAetherEnemyBase::ApplyPrototypeAnimationRootMotionPolicy() const
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance())
	{
		AnimInstance->SetRootMotionMode(ERootMotionMode::NoRootMotionExtraction);
	}
}

void AAetherEnemyBase::LogEnemyAnimationConfiguration() const
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	USkeletalMesh* SkeletalMesh = MeshComponent ? MeshComponent->GetSkeletalMeshAsset() : nullptr;
	const UAnimInstance* AnimInstance = MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[AetherEnemyAnimation] Config / mesh %s / skeleton %s / anim instance %s / prefer fallback %d / move ref %.1f / move rate %.2f-%.2f / action rate %.2f / montage quick %s / standard %s / heavy %s / hit %s / stagger %s / death %s / fallback idle %s / move %s / quick %s / standard %s / heavy %s / hit %s / stagger %s / death %s"),
		*GetPathNameSafe(SkeletalMesh),
		SkeletalMesh ? *GetPathNameSafe(SkeletalMesh->GetSkeleton()) : TEXT("None"),
		AnimInstance ? *GetPathNameSafe(AnimInstance->GetClass()) : TEXT("None"),
		bPreferPrototypeFallbackActionAnimations ? 1 : 0,
		PrototypeMoveAnimationReferenceSpeed,
		PrototypeMoveAnimationMinPlayRate,
		PrototypeMoveAnimationMaxPlayRate,
		PrototypeActionAnimationPlayRate,
		*GetPathNameSafe(QuickAttackMontage.Get()),
		*GetPathNameSafe(StandardAttackMontage.Get()),
		*GetPathNameSafe(HeavyAttackMontage.Get()),
		*GetPathNameSafe(HitReactionMontage.Get()),
		*GetPathNameSafe(ParryStaggerMontage.Get()),
		*GetPathNameSafe(DeathMontage.Get()),
		*GetPathNameSafe(IdleAnimation.Get()),
		*GetPathNameSafe(MoveAnimation.Get()),
		*GetPathNameSafe(QuickAttackAnimation.Get()),
		*GetPathNameSafe(StandardAttackAnimation.Get()),
		*GetPathNameSafe(HeavyAttackAnimation.Get()),
		*GetPathNameSafe(HitReactionAnimation.Get()),
		*GetPathNameSafe(ParryStaggerAnimation.Get()),
		*GetPathNameSafe(DeathAnimation.Get()));
}

void AAetherEnemyBase::HandleDeath(UAetherHealthComponent* DeadHealthComponent, AActor* DamageCauser)
{
	bIsDead = true;
	bIsAttackWindingUp = false;
	bIsAttackRecovering = false;
	bIsHitReacting = false;
	bIsParryStaggered = false;
	bIsExecutionSuppressed = false;
	AurelBossPhase.EndPhaseShift();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackWindupTimerHandle);
		World->GetTimerManager().ClearTimer(AttackRecoveryTimerHandle);
		World->GetTimerManager().ClearTimer(HitReactionTimerHandle);
		World->GetTimerManager().ClearTimer(ParryStaggerTimerHandle);
		World->GetTimerManager().ClearTimer(ExecutionSuppressionTimerHandle);
		World->GetTimerManager().ClearTimer(AurelPhaseShiftTimerHandle);
	}
	ReleaseAttackSlot();

	SetActorEnableCollision(false);
	GetCharacterMovement()->DisableMovement();
	const float FallbackDeathVisualLength = PlayEnemyDeathVisual(
		FAetherEnemyActionPresentationPolicy::BuildDeathVisual(
			BuildEnemyActionVisualAssets(),
			BuildEnemyActionVisualTuning()),
		TEXT("Enemy death"));
	PlayEnemySound(DeathSound.Get());
	if (EnemyArchetype == EAetherEnemyArchetype::Aurel)
	{
		ShowEnemyDebugMessage(TEXT("Aurel defeated / rift seal ready"), GetEnemyArchetypeColor().ToFColor(true));
		if (AAetherGameModeBase* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AAetherGameModeBase>() : nullptr)
		{
			GameMode->RegisterPrototypeProgressFeedback(
				TEXT("AUREL DEFEATED - SEAL THE RIFT"),
				FLinearColor(0.18f, 0.76f, 1.0f, 1.0f));
		}
	}
	ShowEnemyDebugMessage(FString::Printf(TEXT("%s defeated"), *GetEnemyArchetypeName().ToString()), FColor::Red);
	SetLifeSpan(FMath::Max(2.0f, FallbackDeathVisualLength + 0.25f));
}

void AAetherEnemyBase::RefreshPrototypeVisualMode()
{
	const bool bHasSkeletalMesh = GetMesh() && GetMesh()->GetSkeletalMeshAsset();
	const bool bShowPrototypeVisuals = !bAutoHidePrototypeVisualsWhenSkeletalMeshAssigned || !bHasSkeletalMesh;

	if (PrototypeBody)
	{
		PrototypeBody->SetVisibility(bShowPrototypeVisuals, true);
		PrototypeBody->SetHiddenInGame(!bShowPrototypeVisuals, true);
	}
}

void AAetherEnemyBase::ShowEnemyDebugMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherEnemy] %s"), *Message);

	if (bShowEnemyScreenDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
	}
}

void AAetherEnemyBase::PlayEnemySound(USoundBase* Sound, float VolumeMultiplier) const
{
	const FAetherEnemySoundPlayback Playback =
		FAetherEnemyActionPresentationPolicy::BuildSoundPlayback(Sound, VolumeMultiplier, BuildEnemyActionSoundSet());
	if (!Playback.bShouldPlay)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UAetherAudioSettingsLibrary::SpawnSoundAtLocationForCategory(
		this,
		Sound,
		GetActorLocation(),
		EAetherAudioCategory::Sfx,
		Playback.Volume,
		Playback.Pitch);
}
