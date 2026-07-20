#include "AetherPauseMenuComponent.h"

#include "AetherGameModeBase.h"
#include "AetherHealthComponent.h"
#include "AetherMainMenuWidget.h"
#include "AetherMenuFlowSubsystem.h"
#include "AetherPlayerController.h"
#include "AetherfallCharacter.h"
#include "Kismet/GameplayStatics.h"

UAetherPauseMenuComponent::UAetherPauseMenuComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PauseMenuWidgetBlueprintClass = TSoftClassPtr<UAetherMainMenuWidget>(
		FSoftObjectPath(TEXT("/Game/Aetherfall/UI/WBP_MainMenu.WBP_MainMenu_C")));
}

bool UAetherPauseMenuComponent::CanOpenPauseMenu() const
{
	const AAetherPlayerController* Controller = Cast<AAetherPlayerController>(GetOwner());
	const UWorld* World = GetWorld();
	if (!Controller || !Controller->IsLocalController() || !World || World->bIsTearingDown
		|| IsPauseMenuOpen() || bTransitionRequested)
	{
		return false;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	const UAetherMenuFlowSubsystem* Flow = GameInstance ? GameInstance->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr;
	if (!Flow || Flow->IsTransitionInProgress())
	{
		return false;
	}

	const AAetherGameModeBase* GameMode = World->GetAuthGameMode<AAetherGameModeBase>();
	if (!GameMode || GameMode->IsPrototypeDefeatRetryScheduled())
	{
		return false;
	}

	const AAetherfallCharacter* Character = Cast<AAetherfallCharacter>(Controller->GetPawn());
	const UAetherHealthComponent* Health = Character ? Character->GetHealthComponent() : nullptr;
	return Health && !Health->IsDead();
}

bool UAetherPauseMenuComponent::OpenPauseMenu()
{
	if (!CanOpenPauseMenu())
	{
		return false;
	}

	AAetherPlayerController* Controller = CastChecked<AAetherPlayerController>(GetOwner());
	PauseMenuWidget = GetOrCreatePauseMenuWidget(Controller);
	if (!PauseMenuWidget)
	{
		return false;
	}

	PauseMenuWidget->ConfigureMenuContext(EAetherMenuContext::PauseMenu);
	PauseMenuWidget->OnResumeRequested.AddUniqueDynamic(this, &UAetherPauseMenuComponent::HandleResumeRequested);
	PauseMenuWidget->OnReturnToMainMenuRequested.AddUniqueDynamic(this, &UAetherPauseMenuComponent::HandleReturnToMainMenuRequested);
	PauseMenuWidget->AddToViewport(200);

	if (!UGameplayStatics::SetGamePaused(this, true))
	{
		PauseMenuWidget->OnResumeRequested.RemoveAll(this);
		PauseMenuWidget->OnReturnToMainMenuRequested.RemoveAll(this);
		PauseMenuWidget->RemoveFromParent();
		bPauseMenuOpen = false;
		return false;
	}

	bPauseMenuOpen = true;
	Controller->bShowMouseCursor = true;
	Controller->bEnableClickEvents = true;
	Controller->bEnableMouseOverEvents = true;
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Controller->SetInputMode(InputMode);
	PauseMenuWidget->FocusPrimaryAction();
	return true;
}

UAetherMainMenuWidget* UAetherPauseMenuComponent::GetOrCreatePauseMenuWidget(AAetherPlayerController* Controller)
{
	if (PauseMenuWidget)
	{
		return PauseMenuWidget;
	}

	TSubclassOf<UAetherMainMenuWidget> WidgetClass = UAetherMainMenuWidget::StaticClass();
	if (UClass* BlueprintClass = PauseMenuWidgetBlueprintClass.LoadSynchronous())
	{
		WidgetClass = BlueprintClass;
	}

	return Controller ? CreateWidget<UAetherMainMenuWidget>(Controller, WidgetClass) : nullptr;
}

void UAetherPauseMenuComponent::RestoreGameplayInput()
{
	UGameplayStatics::SetGamePaused(this, false);
	if (AAetherPlayerController* Controller = Cast<AAetherPlayerController>(GetOwner()))
	{
		Controller->bShowMouseCursor = false;
		Controller->bEnableClickEvents = false;
		Controller->bEnableMouseOverEvents = false;
		Controller->SetInputMode(FInputModeGameOnly());
	}
}

void UAetherPauseMenuComponent::ClosePauseMenu()
{
	if (!IsPauseMenuOpen() || bTransitionRequested)
	{
		return;
	}

	PauseMenuWidget->OnResumeRequested.RemoveAll(this);
	PauseMenuWidget->OnReturnToMainMenuRequested.RemoveAll(this);
	PauseMenuWidget->RemoveFromParent();
	bPauseMenuOpen = false;
	RestoreGameplayInput();
}

void UAetherPauseMenuComponent::TogglePauseMenu()
{
	if (IsPauseMenuOpen())
	{
		ClosePauseMenu();
	}
	else
	{
		OpenPauseMenu();
	}
}

void UAetherPauseMenuComponent::HandleResumeRequested()
{
	ClosePauseMenu();
}

void UAetherPauseMenuComponent::HandleReturnToMainMenuRequested()
{
	UAetherMenuFlowSubsystem* Flow = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UAetherMenuFlowSubsystem>()
		: nullptr;
	if (!Flow || Flow->IsTransitionInProgress() || bTransitionRequested)
	{
		return;
	}

	bTransitionRequested = true;
	UGameplayStatics::SetGamePaused(this, false);
	if (!Flow->ReturnToMainMenu())
	{
		bTransitionRequested = false;
		UGameplayStatics::SetGamePaused(this, true);
		if (PauseMenuWidget)
		{
			PauseMenuWidget->RestorePauseMenuAfterTransitionFailure(
				NSLOCTEXT("AetherPauseMenu", "ReturnFailed", "The title could not be loaded. Your journey remains paused."));
		}
	}
}

void UAetherPauseMenuComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->OnResumeRequested.RemoveAll(this);
		PauseMenuWidget->OnReturnToMainMenuRequested.RemoveAll(this);
		PauseMenuWidget->RemoveFromParent();
		PauseMenuWidget = nullptr;
	}
	bPauseMenuOpen = false;
	if (!bTransitionRequested)
	{
		RestoreGameplayInput();
	}
	Super::EndPlay(EndPlayReason);
}
