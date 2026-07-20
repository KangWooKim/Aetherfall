#include "AetherPrototypeDamageTarget.h"

#include "AetherHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AAetherPrototypeDamageTarget::AAetherPrototypeDamageTarget()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComponent"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitCapsuleSize(38.0f, 90.0f);
	CollisionComponent->SetCollisionObjectType(ECC_Pawn);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 1.8f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMesh.Object);
	}

	HealthComponent = CreateDefaultSubobject<UAetherHealthComponent>(TEXT("HealthComponent"));
}

void AAetherPrototypeDamageTarget::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &AAetherPrototypeDamageTarget::HandleHealthChanged);
		HealthComponent->OnDeath.AddDynamic(this, &AAetherPrototypeDamageTarget::HandleDeath);
	}
}

void AAetherPrototypeDamageTarget::HandleHealthChanged(UAetherHealthComponent* ChangedHealthComponent, float CurrentHealth, float MaxHealth, AActor* DamageCauser)
{
	if (!DamageCauser)
	{
		return;
	}

	ShowTargetDebugMessage(FString::Printf(TEXT("Target HP: %.0f / %.0f"), CurrentHealth, MaxHealth), FColor::Green);
}

void AAetherPrototypeDamageTarget::HandleDeath(UAetherHealthComponent* DeadHealthComponent, AActor* DamageCauser)
{
	ShowTargetDebugMessage(TEXT("Prototype target defeated"), FColor::Red);
	SetActorEnableCollision(false);
	SetLifeSpan(2.0f);
}

void AAetherPrototypeDamageTarget::ShowTargetDebugMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherTarget] %s"), *Message);

	if (bShowTargetScreenDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, Color, Message);
	}
}
