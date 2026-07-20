#include "AetherLockOnComponent.h"

#include "AetherEnemyBase.h"
#include "AetherHealthComponent.h"
#include "AetherfallCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"

UAetherLockOnComponent::UAetherLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAetherLockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AAetherfallCharacter>(GetOwner());
}

void UAetherLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!LockedTarget.IsValid())
	{
		return;
	}

	if (!IsValidLockOnTarget(LockedTarget.Get(), BreakRange))
	{
		if (bAutoSwitchOnTargetLost)
		{
			if (AAetherEnemyBase* ReplacementTarget = FindBestTarget(LockedTarget.Get()))
			{
				SetLockedTarget(ReplacementTarget, TEXT("Lock-on target switched"));
				return;
			}
		}

		ClearLockOn();
		ShowLockOnDebugMessage(TEXT("Lock-on target lost"), FColor::Yellow);
		return;
	}

	UpdateRotation(DeltaTime);
}

void UAetherLockOnComponent::ToggleLockOn()
{
	if (IsLockedOn())
	{
		ClearLockOn();
		ShowLockOnDebugMessage(TEXT("Lock-on cleared"), FColor::Silver);
		return;
	}

	AAetherEnemyBase* BestTarget = FindBestTarget();
	if (!BestTarget)
	{
		ShowLockOnDebugMessage(TEXT("No lock-on target"), FColor::Yellow);
		return;
	}

	SetLockedTarget(BestTarget, TEXT("Lock-on target acquired"));
}

void UAetherLockOnComponent::ClearLockOn()
{
	LockedTarget.Reset();
}

void UAetherLockOnComponent::SwitchTarget(float Direction)
{
	if (!IsLockedOn())
	{
		ToggleLockOn();
		return;
	}

	AAetherEnemyBase* NewTarget = FindSwitchTarget(Direction);
	if (!NewTarget)
	{
		ShowLockOnDebugMessage(TEXT("No switch target"), FColor::Yellow);
		return;
	}

	SetLockedTarget(NewTarget, TEXT("Lock-on target switched"));
}

bool UAetherLockOnComponent::IsLockedOn() const
{
	return LockedTarget.IsValid() && IsValidLockOnTarget(LockedTarget.Get(), BreakRange);
}

AAetherEnemyBase* UAetherLockOnComponent::GetLockedTarget() const
{
	return IsLockedOn() ? LockedTarget.Get() : nullptr;
}

void UAetherLockOnComponent::SetLockedTarget(AAetherEnemyBase* NewTarget, const FString& Message)
{
	if (!NewTarget)
	{
		return;
	}

	LockedTarget = NewTarget;
	ShowLockOnDebugMessage(Message, FColor::Cyan);

	if (UWorld* World = GetWorld())
	{
		DrawDebugSphere(World, GetTargetFocusLocation(NewTarget), 82.0f, 24, FColor::Cyan, false, 0.8f, 0, 3.0f);
	}
}

AAetherEnemyBase* UAetherLockOnComponent::FindBestTarget(const AAetherEnemyBase* ExcludedTarget) const
{
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	const UWorld* World = GetWorld();
	if (!Character || !World)
	{
		return nullptr;
	}

	const FVector CharacterLocation = Character->GetActorLocation();
	const FRotator ViewRotation = Character->GetController() ? Character->GetController()->GetControlRotation() : Character->GetActorRotation();
	const FVector ViewForward = ViewRotation.Vector().GetSafeNormal2D();
	AAetherEnemyBase* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	for (TActorIterator<AAetherEnemyBase> EnemyIt(World); EnemyIt; ++EnemyIt)
	{
		AAetherEnemyBase* Candidate = *EnemyIt;
		if (Candidate == ExcludedTarget)
		{
			continue;
		}

		if (!IsValidLockOnTarget(Candidate, LockOnRange))
		{
			continue;
		}

		const FVector ToCandidate = Candidate->GetActorLocation() - CharacterLocation;
		const FVector Direction = ToCandidate.GetSafeNormal2D();
		const float FacingScore = FMath::Clamp(FVector::DotProduct(ViewForward, Direction), -1.0f, 1.0f);
		if (FacingScore < -0.15f)
		{
			continue;
		}

		const float DistanceSquared = ToCandidate.SizeSquared2D();
		const float Score = DistanceSquared * (2.0f - FacingScore);
		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

AAetherEnemyBase* UAetherLockOnComponent::FindSwitchTarget(float Direction) const
{
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	const AAetherEnemyBase* CurrentTarget = LockedTarget.Get();
	const UWorld* World = GetWorld();
	if (!Character || !CurrentTarget || !World)
	{
		return nullptr;
	}

	const FRotator ViewRotation = Character->GetController() ? Character->GetController()->GetControlRotation() : Character->GetActorRotation();
	const FVector ViewForward = ViewRotation.Vector().GetSafeNormal2D();
	const FVector CharacterLocation = Character->GetActorLocation();
	const float DesiredSide = Direction >= 0.0f ? 1.0f : -1.0f;

	AAetherEnemyBase* BestDirectionalTarget = nullptr;
	float BestDirectionalScore = TNumericLimits<float>::Max();
	AAetherEnemyBase* BestFallbackTarget = nullptr;
	float BestFallbackScore = TNumericLimits<float>::Max();

	for (TActorIterator<AAetherEnemyBase> EnemyIt(World); EnemyIt; ++EnemyIt)
	{
		AAetherEnemyBase* Candidate = *EnemyIt;
		if (Candidate == CurrentTarget || !IsValidLockOnTarget(Candidate, LockOnRange))
		{
			continue;
		}

		const FVector ToCandidate = Candidate->GetActorLocation() - CharacterLocation;
		const FVector DirectionToCandidate = ToCandidate.GetSafeNormal2D();
		const float Side = FVector::CrossProduct(ViewForward, DirectionToCandidate).Z;
		const float ForwardScore = FMath::Clamp(FVector::DotProduct(ViewForward, DirectionToCandidate), -1.0f, 1.0f);
		const float DistanceScore = ToCandidate.SizeSquared2D() * 0.0001f;
		const float FallbackScore = DistanceScore + (2.0f - ForwardScore);

		if (FallbackScore < BestFallbackScore)
		{
			BestFallbackScore = FallbackScore;
			BestFallbackTarget = Candidate;
		}

		if (Side * DesiredSide <= 0.08f)
		{
			continue;
		}

		const float DirectionalScore = FallbackScore + FMath::Abs(Side) * 0.1f;
		if (DirectionalScore < BestDirectionalScore)
		{
			BestDirectionalScore = DirectionalScore;
			BestDirectionalTarget = Candidate;
		}
	}

	return BestDirectionalTarget ? BestDirectionalTarget : BestFallbackTarget;
}

bool UAetherLockOnComponent::IsValidLockOnTarget(const AAetherEnemyBase* Candidate, float MaxRange) const
{
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character || !Candidate)
	{
		return false;
	}

	const UAetherHealthComponent* HealthComponent = Candidate->GetHealthComponent();
	if (!HealthComponent || HealthComponent->IsDead())
	{
		return false;
	}

	return FVector::DistSquared2D(Character->GetActorLocation(), Candidate->GetActorLocation()) <= FMath::Square(MaxRange);
}

FVector UAetherLockOnComponent::GetTargetFocusLocation(const AAetherEnemyBase* Target) const
{
	return Target ? Target->GetActorLocation() + FVector(0.0f, 0.0f, 95.0f) : FVector::ZeroVector;
}

void UAetherLockOnComponent::UpdateRotation(float DeltaTime)
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	AAetherEnemyBase* Target = LockedTarget.Get();
	if (!Character || !Target)
	{
		return;
	}

	const FVector ToTarget = (Target->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	const FRotator TargetYawRotation = ToTarget.Rotation();
	Character->SetActorRotation(FMath::RInterpTo(Character->GetActorRotation(), TargetYawRotation, DeltaTime, CharacterRotationInterpSpeed));

	if (bRotateCameraToTarget)
	{
		if (AController* Controller = Character->GetController())
		{
			const FRotator CurrentControlRotation = Controller->GetControlRotation();
			const FRotator DesiredControlRotation(CurrentControlRotation.Pitch, TargetYawRotation.Yaw, 0.0f);
			Controller->SetControlRotation(FMath::RInterpTo(CurrentControlRotation, DesiredControlRotation, DeltaTime, CameraRotationInterpSpeed));
		}
	}
}

void UAetherLockOnComponent::ShowLockOnDebugMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherLockOn] %s"), *Message);

	if (bShowLockOnScreenDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.25f, Color, Message);
	}
}
