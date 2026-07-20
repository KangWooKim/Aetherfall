#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherInteractionComponent.generated.h"

UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Interaction")
	AActor* GetFocusedInteractableActor() const { return FocusedInteractableActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Interaction")
	FText GetFocusedInteractionPrompt() const;

private:
	void RefreshFocusedInteractable();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aetherfall|Interaction", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float InteractionRadius = 220.0f;

	TWeakObjectPtr<AActor> FocusedInteractableActor;
};
