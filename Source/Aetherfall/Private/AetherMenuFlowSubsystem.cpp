/** 저장 가능 여부와 중복 요청을 확인하며 메뉴·게임 맵 사이의 전환 상태와 로딩 요청을 관리한다. */
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

/** 슬롯 존재만이 아니라 스키마와 체크포인트를 포함한 로드 가능 여부를 확인한 후 게임 맵을 연다. */
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

/** 기존 진행이 있으면 덮어쓰기 인자를 요구하고 삭제 성공 후 게임 맵 전환을 요청한다. */
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

/** 중복 전환을 거부하고 로딩 화면과 전환 상태를 설정한 뒤 엔진에 레벨 열기를 요청한다. */
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

/** 맵 로드 완료 콜백에서 진행 중 전환 상태를 대기 상태로 되돌린다. */
void UAetherMenuFlowSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (LoadedWorld && FlowState == EAetherMenuFlowState::Transitioning)
	{
		SetFlowState(EAetherMenuFlowState::Idle, FText::GetEmpty());
	}
}
