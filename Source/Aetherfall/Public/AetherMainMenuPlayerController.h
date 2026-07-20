#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AetherMainMenuPlayerController.generated.h"

class UAetherMainMenuWidget;

UCLASS()
class AETHERFALL_API AAetherMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAetherMainMenuPlayerController();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Menu")
	TSubclassOf<UAetherMainMenuWidget> MenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Menu")
	TSoftClassPtr<UAetherMainMenuWidget> MenuWidgetBlueprintClass;

	UPROPERTY(Transient)
	TObjectPtr<UAetherMainMenuWidget> MenuWidget;
};
