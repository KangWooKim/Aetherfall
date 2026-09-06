/** 전용 저장 슬롯을 사용해 새 게임·메뉴 복귀·계속하기 맵 전환과 미래 저장 버전 거부를 검증하는 비배포용 명령이다. */
#include "AetherMenuFlowSubsystem.h"
#include "AetherPrototypeSaveGame.h"
#include "AetherPrototypeSaveSchemaPolicy.h"
#include "AetherSaveSubsystem.h"

#if !UE_BUILD_SHIPPING

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogAetherMenuFlowValidation, Log, All);

struct FAetherSaveSubsystemRuntimeValidationAccess
{
	static FString GetSlotName(const UAetherSaveSubsystem& SaveSubsystem)
	{
		return SaveSubsystem.PrototypeCheckpointSlotName;
	}

	static int32 GetUserIndex(const UAetherSaveSubsystem& SaveSubsystem)
	{
		return SaveSubsystem.PrototypeCheckpointUserIndex;
	}

	static void SetSlot(UAetherSaveSubsystem& SaveSubsystem, const FString& SlotName, int32 UserIndex)
	{
		SaveSubsystem.PrototypeCheckpointSlotName = SlotName;
		SaveSubsystem.PrototypeCheckpointUserIndex = UserIndex;
	}
};

namespace
{
constexpr TCHAR ValidationSlotName[] = TEXT("PrototypeCheckpoint_Sprint154Validation");
constexpr int32 ValidationUserIndex = 154;

enum class EAetherMenuFlowValidationPhase : uint8
{
	Idle,
	AwaitingNewGameMap,
	AwaitingMenuReturn,
	AwaitingContinueMap
};

struct FAetherMenuFlowValidationState
{
	bool bRunning = false;
	bool bFailed = false;
	EAetherMenuFlowValidationPhase Phase = EAetherMenuFlowValidationPhase::Idle;
	FString OriginalSlotName;
	int32 OriginalUserIndex = 0;
	TWeakObjectPtr<UGameInstance> GameInstance;
	FDelegateHandle PostLoadMapHandle;
};

FAetherMenuFlowValidationState ValidationState;

void RecordCheck(bool bCondition, const TCHAR* CheckName)
{
	ValidationState.bFailed |= !bCondition;
	UE_LOG(
		LogAetherMenuFlowValidation,
		Display,
		TEXT("[AetherMenuFlowValidation] %s: %s"),
		bCondition ? TEXT("PASS") : TEXT("FAIL"),
		CheckName);
}

UWorld* FindRuntimeWorld()
{
	if (!GEngine)
	{
		return nullptr;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* Candidate = WorldContext.World();
		if (Candidate && (WorldContext.WorldType == EWorldType::Game || WorldContext.WorldType == EWorldType::PIE))
		{
			return Candidate;
		}
	}
	return nullptr;
}

bool IsExpectedMap(const UWorld* World, const TCHAR* ExpectedMapName)
{
	return World && World->GetMapName().Contains(ExpectedMapName);
}

/** 검증 슬롯과 맵 콜백을 정리하고 원래 슬롯 설정을 복원한 뒤 누적 결과로 프로세스를 종료한다. */
void FinishValidation()
{
	UGameInstance* GameInstance = ValidationState.GameInstance.Get();
	UAetherSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	if (SaveSubsystem)
	{
		SaveSubsystem->ClearPrototypeCheckpointSnapshot();
		FAetherSaveSubsystemRuntimeValidationAccess::SetSlot(
			*SaveSubsystem,
			ValidationState.OriginalSlotName,
			ValidationState.OriginalUserIndex);
	}

	if (ValidationState.PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(ValidationState.PostLoadMapHandle);
		ValidationState.PostLoadMapHandle.Reset();
	}

	UE_LOG(
		LogAetherMenuFlowValidation,
		Display,
		TEXT("[AetherMenuFlowValidation] RESULT: %s"),
		ValidationState.bFailed ? TEXT("FAIL") : TEXT("PASS"));
	ValidationState.bRunning = false;
	FPlatformMisc::RequestExitWithStatus(false, ValidationState.bFailed ? 1 : 0);
}

void ContinueFromMenu()
{
	UGameInstance* GameInstance = ValidationState.GameInstance.Get();
	UAetherSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	UAetherMenuFlowSubsystem* MenuFlow = GameInstance ? GameInstance->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr;
	if (!SaveSubsystem || !MenuFlow)
	{
		RecordCheck(false, TEXT("menu services survived map return"));
		FinishValidation();
		return;
	}
	RecordCheck(!MenuFlow->IsTransitionInProgress(), TEXT("flow returned to idle after menu load"));

	UAetherPrototypeSaveGame* ValidationSave = NewObject<UAetherPrototypeSaveGame>(GameInstance);
	FAetherPrototypeSaveSchemaPolicy::StampCurrentSchema(*ValidationSave);
	ValidationSave->bHasActiveCheckpoint = true;
	ValidationSave->ActiveCheckpointLabel = FName(TEXT("CP_ForestStart"));
	ValidationSave->ActiveCheckpointProgressRank = 0;
	ValidationSave->ActiveCheckpointTransform = FTransform(FRotator::ZeroRotator, FVector(0.0, 0.0, 150.0));
	RecordCheck(SaveSubsystem->SavePrototypeCheckpointSnapshot(ValidationSave), TEXT("isolated continue checkpoint created"));

	const FAetherSaveSlotSummary Summary = SaveSubsystem->GetPrototypeCheckpointSummary();
	RecordCheck(
		Summary.bExists && Summary.bLoadable && Summary.CheckpointLabel == FName(TEXT("CP_ForestStart")),
		TEXT("load screen summary exposes valid checkpoint"));
	RecordCheck(SaveSubsystem->GetSaveSlotSummaries().Num() == 1, TEXT("single-slot load list remains extensible"));

	ValidationState.Phase = EAetherMenuFlowValidationPhase::AwaitingContinueMap;
	RecordCheck(MenuFlow->ContinueGame(), TEXT("continue accepted valid checkpoint"));
}

void FinishContinueValidation()
{
	UWorld* World = FindRuntimeWorld();
	UGameInstance* GameInstance = ValidationState.GameInstance.Get();
	UAetherSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	UAetherMenuFlowSubsystem* MenuFlow = GameInstance ? GameInstance->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr;
	if (!World || !SaveSubsystem || !MenuFlow)
	{
		RecordCheck(false, TEXT("runtime services survived continue transition"));
		FinishValidation();
		return;
	}

	RecordCheck(IsExpectedMap(World, TEXT("M_VerticalSlice")), TEXT("continue map remains active on next tick"));
	RecordCheck(!MenuFlow->IsTransitionInProgress(), TEXT("flow returned to idle after continue load"));
	RecordCheck(SaveSubsystem->HasPrototypeCheckpointSnapshot(), TEXT("continue preserved checkpoint progress"));
	FinishValidation();
}

void ReturnToMenu()
{
	UGameInstance* GameInstance = ValidationState.GameInstance.Get();
	UAetherMenuFlowSubsystem* MenuFlow = GameInstance ? GameInstance->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr;
	if (!MenuFlow)
	{
		RecordCheck(false, TEXT("menu flow survived gameplay transition"));
		FinishValidation();
		return;
	}
	RecordCheck(!MenuFlow->IsTransitionInProgress(), TEXT("flow returned to idle after new game load"));

	ValidationState.Phase = EAetherMenuFlowValidationPhase::AwaitingMenuReturn;
	RecordCheck(MenuFlow->ReturnToMainMenu(), TEXT("return to menu transition accepted"));
}

void HandlePostLoadMap(UWorld* LoadedWorld)
{
	UGameInstance* GameInstance = ValidationState.GameInstance.Get();
	UAetherSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	UAetherMenuFlowSubsystem* MenuFlow = GameInstance ? GameInstance->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr;
	if (!LoadedWorld || !SaveSubsystem || !MenuFlow)
	{
		RecordCheck(false, TEXT("runtime services available after map load"));
		FinishValidation();
		return;
	}

	switch (ValidationState.Phase)
	{
	case EAetherMenuFlowValidationPhase::AwaitingNewGameMap:
		RecordCheck(IsExpectedMap(LoadedWorld, TEXT("M_VerticalSlice")), TEXT("new game opened gameplay map"));
		RecordCheck(!SaveSubsystem->HasPrototypeCheckpointSnapshot(), TEXT("new game cleared isolated progress"));
		LoadedWorld->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateStatic(&ReturnToMenu));
		break;

	case EAetherMenuFlowValidationPhase::AwaitingMenuReturn:
		RecordCheck(IsExpectedMap(LoadedWorld, TEXT("M_MainMenu")), TEXT("return opened dedicated menu map"));
		LoadedWorld->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateStatic(&ContinueFromMenu));
		break;

	case EAetherMenuFlowValidationPhase::AwaitingContinueMap:
		RecordCheck(IsExpectedMap(LoadedWorld, TEXT("M_VerticalSlice")), TEXT("continue opened gameplay map"));
		LoadedWorld->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateStatic(&FinishContinueValidation));
		break;

	default:
		break;
	}
}

/** 원래 슬롯 정보를 보관하고 검증 슬롯으로 전환한 뒤 저장 거부 조건과 연속 맵 이동을 검사한다. */
void RunMenuFlowRuntimeValidation()
{
	if (ValidationState.bRunning)
	{
		UE_LOG(LogAetherMenuFlowValidation, Warning, TEXT("[AetherMenuFlowValidation] Validation already running"));
		return;
	}

	UWorld* World = FindRuntimeWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UAetherSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	UAetherMenuFlowSubsystem* MenuFlow = GameInstance ? GameInstance->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr;
	if (!World || !GameInstance || !SaveSubsystem || !MenuFlow)
	{
		UE_LOG(LogAetherMenuFlowValidation, Error, TEXT("[AetherMenuFlowValidation] Menu runtime services unavailable"));
		FPlatformMisc::RequestExitWithStatus(false, 1);
		return;
	}

	ValidationState = FAetherMenuFlowValidationState();
	ValidationState.bRunning = true;
	ValidationState.GameInstance = GameInstance;
	ValidationState.OriginalSlotName = FAetherSaveSubsystemRuntimeValidationAccess::GetSlotName(*SaveSubsystem);
	ValidationState.OriginalUserIndex = FAetherSaveSubsystemRuntimeValidationAccess::GetUserIndex(*SaveSubsystem);
	FAetherSaveSubsystemRuntimeValidationAccess::SetSlot(*SaveSubsystem, ValidationSlotName, ValidationUserIndex);
	SaveSubsystem->ClearPrototypeCheckpointSnapshot();

	ValidationState.PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddStatic(&HandlePostLoadMap);

	RecordCheck(!MenuFlow->ContinueGame(), TEXT("continue rejects missing save"));

	UAetherPrototypeSaveGame* FutureSave = NewObject<UAetherPrototypeSaveGame>(GameInstance);
	FutureSave->SaveSchemaVersion = FAetherPrototypeSaveSchemaPolicy::CurrentSchemaVersion + 1;
	FutureSave->SaveSchemaLabel = FName(TEXT("AetherPrototypeCheckpointFuture"));
	FutureSave->bHasActiveCheckpoint = true;
	FutureSave->ActiveCheckpointLabel = FName(TEXT("CP_FutureVersion"));
	RecordCheck(SaveSubsystem->SavePrototypeCheckpointSnapshot(FutureSave), TEXT("future-version fixture created"));
	const FAetherSaveSlotSummary FutureSummary = SaveSubsystem->GetPrototypeCheckpointSummary();
	RecordCheck(FutureSummary.bExists && !FutureSummary.bLoadable, TEXT("future-version save is visible but not loadable"));
	RecordCheck(!MenuFlow->ContinueGame(), TEXT("continue rejects future-version save"));
	RecordCheck(!MenuFlow->IsTransitionInProgress(), TEXT("load failure restores idle menu input"));
	RecordCheck(SaveSubsystem->ClearPrototypeCheckpointSnapshot(), TEXT("future-version fixture removed"));

	UAetherPrototypeSaveGame* ExistingProgress = NewObject<UAetherPrototypeSaveGame>(GameInstance);
	FAetherPrototypeSaveSchemaPolicy::StampCurrentSchema(*ExistingProgress);
	ExistingProgress->bHasActiveCheckpoint = true;
	ExistingProgress->ActiveCheckpointLabel = FName(TEXT("CP_BrokenWall"));
	ExistingProgress->ActiveCheckpointProgressRank = 20;
	RecordCheck(SaveSubsystem->SavePrototypeCheckpointSnapshot(ExistingProgress), TEXT("isolated overwrite fixture created"));
	RecordCheck(!MenuFlow->StartNewGame(false), TEXT("new game requires overwrite confirmation"));
	RecordCheck(SaveSubsystem->HasPrototypeCheckpointSnapshot(), TEXT("unconfirmed new game preserves progress"));

	ValidationState.Phase = EAetherMenuFlowValidationPhase::AwaitingNewGameMap;
	RecordCheck(MenuFlow->StartNewGame(true), TEXT("confirmed new game transition accepted"));
	RecordCheck(!MenuFlow->ReturnToMainMenu(), TEXT("duplicate transition input blocked"));
}

FAutoConsoleCommand RunMenuFlowValidationCommand(
	TEXT("Aether.Menu.ValidateRuntime"),
	TEXT("Runs isolated non-shipping menu flow validation and exits with its result."),
	FConsoleCommandDelegate::CreateStatic(&RunMenuFlowRuntimeValidation));
}

#endif
