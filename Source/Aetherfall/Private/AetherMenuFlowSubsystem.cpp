#include "AetherMenuFlowSubsystem.h"

#include "AetherLoadingScreenSubsystem.h"
#include "AetherSaveSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/UObjectGlobals.h"

void UAetherMenuFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UAetherMenuFlowSubsystem::HandlePostLoadMap);
}

void UAetherMenuFlowSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	Super::Deinitialize();
}

bool UAetherMenuFlowSubsystem::ContinueGame()
{
	if (IsTransitionInProgress())
	{
		return false;
	}

	const UAetherSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	const FAetherSaveSlotSummary Summary = SaveSubsystem ? SaveSubsystem->GetPrototypeCheckpointSummary() : FAetherSaveSlotSummary();
	if (!Summary.bLoadable)
	{
		SetFlowState(EAetherMenuFlowState::Failed, Summary.StatusText.IsEmpty()
			? NSLOCTEXT("AetherMenu", "ContinueUnavailable", "No valid journey is available to continue.")
			: Summary.StatusText);
		return false;
	}

	return OpenMap(GameplayMapName, NSLOCTEXT("AetherMenu", "ContinueLoading", "Returning to the last checkpoint..."));
}

bool UAetherMenuFlowSubsystem::StartNewGame(bool bOverwriteExistingProgress)
{
	if (IsTransitionInProgress())
	{
		return false;
	}

	UAetherSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	if (!SaveSubsystem)
	{
		SetFlowState(EAetherMenuFlowState::Failed, NSLOCTEXT("AetherMenu", "SaveServiceUnavailable", "The save service is unavailable."));
		return false;
	}

	if (SaveSubsystem->HasPrototypeCheckpointSnapshot())
	{
		if (!bOverwriteExistingProgress)
		{
			SetFlowState(EAetherMenuFlowState::Failed, NSLOCTEXT("AetherMenu", "OverwriteConfirmationRequired", "Existing progress requires confirmation."));
			return false;
		}

		if (!SaveSubsystem->ClearPrototypeCheckpointSnapshot())
		{
			SetFlowState(EAetherMenuFlowState::Failed, NSLOCTEXT("AetherMenu", "ProgressClearFailed", "Existing progress could not be cleared."));
			return false;
		}
	}

	return OpenMap(GameplayMapName, NSLOCTEXT("AetherMenu", "NewGameLoading", "Beginning a new journey..."));
}

bool UAetherMenuFlowSubsystem::LoadGame()
{
	return ContinueGame();
}

bool UAetherMenuFlowSubsystem::ReturnToMainMenu()
{
	return OpenMap(MainMenuMapName, NSLOCTEXT("AetherMenu", "MenuLoading", "Returning to the title..."));
}

void UAetherMenuFlowSubsystem::QuitGame(APlayerController* PlayerController)
{
	if (IsTransitionInProgress())
	{
		return;
	}

	SetFlowState(EAetherMenuFlowState::Transitioning, NSLOCTEXT("AetherMenu", "QuitLoading", "Closing Aetherfall..."));
	UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
}

bool UAetherMenuFlowSubsystem::OpenMap(FName MapName, const FText& LoadingMessage)
{
	if (IsTransitionInProgress() || MapName.IsNone() || !GetWorld())
	{
		return false;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAetherLoadingScreenSubsystem* LoadingScreen = GameInstance->GetSubsystem<UAetherLoadingScreenSubsystem>())
		{
			LoadingScreen->BeginLoadingScreen(LoadingMessage, true);
		}
	}

	SetFlowState(EAetherMenuFlowState::Transitioning, LoadingMessage);
	UGameplayStatics::OpenLevel(this, MapName);
	return true;
}

void UAetherMenuFlowSubsystem::SetFlowState(EAetherMenuFlowState NewState, const FText& Message)
{
	FlowState = NewState;
	OnMenuFlowChanged.Broadcast(FlowState, Message);
}

void UAetherMenuFlowSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (LoadedWorld && FlowState == EAetherMenuFlowState::Transitioning)
	{
		SetFlowState(EAetherMenuFlowState::Idle, FText::GetEmpty());
	}
}
