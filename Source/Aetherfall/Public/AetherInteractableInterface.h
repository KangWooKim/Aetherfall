#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AetherInteractableInterface.generated.h"

UINTERFACE(BlueprintType)
class AETHERFALL_API UAetherInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class AETHERFALL_API IAetherInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Aetherfall|Interaction")
	FText GetInteractionPrompt(AActor* Interactor) const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Aetherfall|Interaction")
	void Interact(AActor* Interactor);
};
