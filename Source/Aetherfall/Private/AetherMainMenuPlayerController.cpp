#include "AetherMainMenuPlayerController.h"

#include "AetherLoadingScreenSubsystem.h"
#include "AetherMainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "Engine/GameInstance.h"

AAetherMainMenuPlayerController::AAetherMainMenuPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	MenuWidgetClass = UAetherMainMenuWidget::StaticClass();
	MenuWidgetBlueprintClass = TSoftClassPtr<UAetherMainMenuWidget>(
		FSoftObjectPath(TEXT("/Game/Aetherfall/UI/WBP_MainMenu.WBP_MainMenu_C")));
}

void AAetherMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	TSubclassOf<UAetherMainMenuWidget> ResolvedWidgetClass = MenuWidgetClass;
	if (UClass* BlueprintWidgetClass = MenuWidgetBlueprintClass.LoadSynchronous())
	{
		ResolvedWidgetClass = BlueprintWidgetClass;
	}
	if (!ResolvedWidgetClass)
	{
		ResolvedWidgetClass = UAetherMainMenuWidget::StaticClass();
	}

	MenuWidget = CreateWidget<UAetherMainMenuWidget>(this, ResolvedWidgetClass);
	if (!MenuWidget)
	{
		return;
	}

	MenuWidget->AddToViewport(100);
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(MenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	MenuWidget->SetKeyboardFocus();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAetherLoadingScreenSubsystem* LoadingScreen = GameInstance->GetSubsystem<UAetherLoadingScreenSubsystem>())
		{
			LoadingScreen->NotifyGameplayWorldReady();
		}
	}
}
