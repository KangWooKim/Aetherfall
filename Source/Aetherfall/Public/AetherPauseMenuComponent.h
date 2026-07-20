#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherPauseMenuComponent.generated.h"

class AAetherPlayerController;
class UAetherMainMenuWidget;
#if !UE_BUILD_SHIPPING
struct FAetherPauseMenuRuntimeValidationAccess;
#endif

UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherPauseMenuComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherPauseMenuComponent();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Pause Menu")
	bool OpenPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Pause Menu")
	void ClosePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Pause Menu")
	void TogglePauseMenu();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Pause Menu")
	bool IsPauseMenuOpen() const { return bPauseMenuOpen && PauseMenuWidget != nullptr; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
#if !UE_BUILD_SHIPPING
	friend struct FAetherPauseMenuRuntimeValidationAccess;
#endif
	bool CanOpenPauseMenu() const;
	UAetherMainMenuWidget* GetOrCreatePauseMenuWidget(AAetherPlayerController* Controller);
	void RestoreGameplayInput();

	UFUNCTION()
	void HandleResumeRequested();

	UFUNCTION()
	void HandleReturnToMainMenuRequested();

	UPROPERTY(Transient)
	TObjectPtr<UAetherMainMenuWidget> PauseMenuWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Pause Menu")
	TSoftClassPtr<UAetherMainMenuWidget> PauseMenuWidgetBlueprintClass;

	bool bPauseMenuOpen = false;
	bool bTransitionRequested = false;
};
