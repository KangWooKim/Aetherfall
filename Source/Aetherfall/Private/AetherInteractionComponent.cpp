/** 플레이어 주변에서 상호작용 인터페이스를 구현한 가장 가까운 액터를 선택하고 실행을 위임한다. */
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

/** 선택 대상과 소유자가 살아 있으면 인터페이스 실행을 요청한다. 반환값은 대상 기능의 성공 여부가 아닌 요청 전달 여부다. */
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

/** 월드 액터를 순회하며 인터페이스와 비어 있지 않은 문구를 확인한 뒤 반경 안의 최근접 대상을 약한 참조로 보관한다. */
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
