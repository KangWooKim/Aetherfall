#include "AetherfallCharacter.h"

#include "AetherCombatComponent.h"
#include "AetherHealthComponent.h"
#include "AetherInventoryComponent.h"
#include "AetherInteractionComponent.h"
#include "AetherLockOnComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"

AAetherfallCharacter::AAetherfallCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->bOrientRotationToMovement = true;
	MovementComponent->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	MovementComponent->MaxWalkSpeed = 500.0f;
	MovementComponent->MinAnalogWalkSpeed = 20.0f;
	MovementComponent->BrakingDecelerationWalking = 2000.0f;
	MovementComponent->BrakingDecelerationFalling = 1500.0f;
	MovementComponent->AirControl = 0.35f;
	MovementComponent->JumpZVelocity = 700.0f;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 420.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 60.0f, 70.0f);
	BaseCameraSocketOffset = CameraBoom->SocketOffset;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 12.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	CombatComponent = CreateDefaultSubobject<UAetherCombatComponent>(TEXT("CombatComponent"));
	HealthComponent = CreateDefaultSubobject<UAetherHealthComponent>(TEXT("HealthComponent"));
	InventoryComponent = CreateDefaultSubobject<UAetherInventoryComponent>(TEXT("InventoryComponent"));
	LockOnComponent = CreateDefaultSubobject<UAetherLockOnComponent>(TEXT("LockOnComponent"));
	InteractionComponent = CreateDefaultSubobject<UAetherInteractionComponent>(TEXT("InteractionComponent"));

	PrototypeBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeBody"));
	PrototypeBody->SetupAttachment(RootComponent);
	PrototypeBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PrototypeBody->SetRelativeLocation(FVector(0.0f, 0.0f, -42.0f));
	PrototypeBody->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.6f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		PrototypeBody->SetStaticMesh(CylinderMesh.Object);
	}

	PrototypeWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PrototypeWeapon"));
	PrototypeWeapon->SetupAttachment(RootComponent);
	PrototypeWeapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PrototypeWeapon->SetRelativeLocation(FVector(40.0f, 48.0f, 20.0f));
	PrototypeWeapon->SetRelativeRotation(FRotator(0.0f, 0.0f, 12.0f));
	PrototypeWeapon->SetRelativeScale3D(FVector(0.08f, 0.18f, 1.35f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PrototypeWeapon->SetStaticMesh(CubeMesh.Object);
	}

	WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponStaticMesh"));
	WeaponStaticMesh->SetupAttachment(GetMesh(), WeaponAttachSocketName);
	WeaponStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkeletalMesh"));
	WeaponSkeletalMesh->SetupAttachment(GetMesh(), WeaponAttachSocketName);
	WeaponSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAetherfallCharacter::BeginPlay()
{
	Super::BeginPlay();

	RefreshPrototypeVisualMode();
	RefreshWeaponAttachment();
}

void AAetherfallCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshPrototypeVisualMode();
	RefreshWeaponAttachment();
}

void AAetherfallCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CameraBoom || CameraImpactDuration <= 0.0f)
	{
		SetActorTickEnabled(false);
		return;
	}

	CameraImpactElapsed += DeltaTime;
	const float Alpha = FMath::Clamp(CameraImpactElapsed / CameraImpactDuration, 0.0f, 1.0f);
	const float Remaining = 1.0f - Alpha;
	const float SidePulse = FMath::Sin(CameraImpactElapsed * 84.0f) * CameraImpactStrength * 0.22f * Remaining;
	CameraImpactOffset = FVector(-CameraImpactStrength * Remaining, SidePulse, CameraImpactStrength * 0.16f * Remaining);
	CameraBoom->SocketOffset = BaseCameraSocketOffset + CameraImpactOffset;

	if (Alpha >= 1.0f)
	{
		CameraImpactElapsed = 0.0f;
		CameraImpactDuration = 0.0f;
		CameraImpactStrength = 0.0f;
		CameraImpactOffset = FVector::ZeroVector;
		CameraBoom->SocketOffset = BaseCameraSocketOffset;
		SetActorTickEnabled(false);
	}
}

void AAetherfallCharacter::SetDesiredMovementDirection(const FVector& WorldDirection)
{
	DesiredMovementDirection = WorldDirection.GetSafeNormal2D();
}

void AAetherfallCharacter::ClearDesiredMovementDirection()
{
	DesiredMovementDirection = FVector::ZeroVector;
}

FVector AAetherfallCharacter::GetDesiredMovementDirection() const
{
	if (!DesiredMovementDirection.IsNearlyZero())
	{
		return DesiredMovementDirection;
	}

	return GetActorForwardVector().GetSafeNormal2D();
}

float AAetherfallCharacter::GetGroundSpeed() const
{
	FVector GroundVelocity = GetVelocity();
	GroundVelocity.Z = 0.0f;
	return GroundVelocity.Size();
}

float AAetherfallCharacter::GetMovementDirectionAngle() const
{
	FVector GroundVelocity = GetVelocity();
	GroundVelocity.Z = 0.0f;
	const FVector MoveDirection = GroundVelocity.GetSafeNormal();
	if (MoveDirection.IsNearlyZero())
	{
		return 0.0f;
	}

	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = GetActorRightVector().GetSafeNormal2D();
	const float ForwardDot = FVector::DotProduct(Forward, MoveDirection);
	const float RightDot = FVector::DotProduct(Right, MoveDirection);
	return FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));
}

bool AAetherfallCharacter::IsMovingOnGround() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	return MovementComponent && !MovementComponent->IsFalling() && GetGroundSpeed() > 3.0f;
}

bool AAetherfallCharacter::IsFallingForAnimation() const
{
	const UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	return MovementComponent && MovementComponent->IsFalling();
}

bool AAetherfallCharacter::IsLockedOnForAnimation() const
{
	return LockOnComponent && LockOnComponent->GetLockedTarget() != nullptr;
}

bool AAetherfallCharacter::IsGuardingForAnimation() const
{
	return CombatComponent && CombatComponent->IsGuarding();
}

bool AAetherfallCharacter::IsAttackingForAnimation() const
{
	return CombatComponent && CombatComponent->IsAttacking();
}

bool AAetherfallCharacter::IsDodgingForAnimation() const
{
	return CombatComponent && CombatComponent->IsDodging();
}

bool AAetherfallCharacter::IsCombatMovementBlockedForAnimation() const
{
	return CombatComponent && CombatComponent->ShouldBlockMovementInput();
}

void AAetherfallCharacter::PlayCameraImpactFeedback(float Strength, float Duration)
{
	if (!CameraBoom || Strength <= 0.0f || Duration <= 0.0f)
	{
		return;
	}

	CameraImpactStrength = FMath::Max(CameraImpactStrength, Strength);
	CameraImpactDuration = FMath::Max(0.01f, Duration);
	CameraImpactElapsed = 0.0f;
	SetActorTickEnabled(true);
}

void AAetherfallCharacter::RefreshWeaponAttachment()
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

void AAetherfallCharacter::RefreshPrototypeVisualMode()
{
	const bool bHasSkeletalMesh = GetMesh() && GetMesh()->GetSkeletalMeshAsset();
	const bool bShowPrototypeVisuals = !bAutoHidePrototypeVisualsWhenSkeletalMeshAssigned || !bHasSkeletalMesh;

	if (PrototypeBody)
	{
		PrototypeBody->SetVisibility(bShowPrototypeVisuals, true);
		PrototypeBody->SetHiddenInGame(!bShowPrototypeVisuals, true);
	}

	if (PrototypeWeapon)
	{
		PrototypeWeapon->SetVisibility(bShowPrototypeVisuals, true);
		PrototypeWeapon->SetHiddenInGame(!bShowPrototypeVisuals, true);
	}
}
