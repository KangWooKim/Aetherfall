#pragma once

#include "CoreMinimal.h"
#include "AetherLoadingScreenTypes.h"
#include "Blueprint/UserWidget.h"
#include "AetherLoadingScreenWidget.generated.h"

UCLASS(Blueprintable)
class AETHERFALL_API UAetherLoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, Category = "Aetherfall|Loading Screen")
	void ApplyLoadingScreenViewModel(const FAetherLoadingScreenViewModel& ViewModel);
};
