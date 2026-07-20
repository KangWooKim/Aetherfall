#include "AetherInteractionComponent.h"

#include "AetherInteractableInterface.h"
#include "EngineUtils.h"

UAetherInteractionComponent::UAetherInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAetherInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	RefreshFocusedInteractable();
}

bool UAetherInteractionComponent::TryInteract()
{
	AActor* InteractableActor = FocusedInteractableActor.Get();
	AActor* OwnerActor = GetOwner();
	if (!InteractableActor || !OwnerActor)
	{
		UE_LOG(LogTemp, Log, TEXT("[AetherInteraction] No interactable target"));
		return false;
	}

	IAetherInteractableInterface::Execute_Interact(InteractableActor, OwnerActor);
	return true;
}

FText UAetherInteractionComponent::GetFocusedInteractionPrompt() const
{
	AActor* InteractableActor = FocusedInteractableActor.Get();
	AActor* OwnerActor = GetOwner();
	if (!InteractableActor || !OwnerActor)
	{
		return FText::GetEmpty();
	}

	return IAetherInteractableInterface::Execute_GetInteractionPrompt(InteractableActor, OwnerActor);
}

void UAetherInteractionComponent::RefreshFocusedInteractable()
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World)
	{
		FocusedInteractableActor.Reset();
		return;
	}

	AActor* BestActor = nullptr;
	float BestDistanceSquared = FMath::Square(InteractionRadius);
	const FVector OwnerLocation = OwnerActor->GetActorLocation();

	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Candidate = *ActorIt;
		if (!Candidate || Candidate == OwnerActor || !Candidate->GetClass()->ImplementsInterface(UAetherInteractableInterface::StaticClass()))
		{
			continue;
		}

		if (IAetherInteractableInterface::Execute_GetInteractionPrompt(Candidate, OwnerActor).IsEmpty())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(OwnerLocation, Candidate->GetActorLocation());
		if (DistanceSquared > BestDistanceSquared)
		{
			continue;
		}

		BestDistanceSquared = DistanceSquared;
		BestActor = Candidate;
	}

	FocusedInteractableActor = BestActor;
}
