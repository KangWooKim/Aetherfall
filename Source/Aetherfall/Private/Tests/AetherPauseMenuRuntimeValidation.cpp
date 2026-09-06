/** 전용 설정·진행 슬롯으로 일시 정지 UI, 실제 시간 기준 표시 설정 복원, 메뉴 복귀와 계속하기를 검증한다. */
#if !UE_BUILD_SHIPPING

#include "AetherGameModeBase.h"
#include "AetherMainMenuWidget.h"
#include "AetherMenuFlowSubsystem.h"
#include "AetherPauseMenuComponent.h"
#include "AetherPlayerController.h"
#include "AetherPrototypeSaveGame.h"
#include "AetherPrototypeSaveSchemaPolicy.h"
#include "AetherSaveSubsystem.h"
#include "AetherSettingsSubsystem.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Widgets/SWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogAetherPauseMenuValidation, Log, All);

namespace
{
constexpr TCHAR PauseSettingsSlot[] = TEXT("AetherSettings_Validation_Sprint155Pause");
constexpr TCHAR PauseProgressSlot[] = TEXT("PrototypeCheckpoint_Sprint155PauseValidation");
constexpr int32 PauseProgressUserIndex = 155;

UWorld* FindRuntimeWorld()
{
	if (!GEngine)
	{
		return nullptr;
	}
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE))
		{
			return World;
		}
	}
	return nullptr;
}

bool RouteSlateKey(const FKey& Key)
{
	if (!FSlateApplication::IsInitialized())
	{
		return false;
	}
	FSlateApplication& Slate = FSlateApplication::Get();
	const FModifierKeysState Modifiers;
	const FKeyEvent Down(Key, Modifiers, 0, false, 0, 0);
	const FKeyEvent Up(Key, Modifiers, 0, false, 0, 0);
	const bool bHandled = Slate.ProcessKeyDownEvent(Down);
	Slate.ProcessKeyUpEvent(Up);
	return bHandled;
}

struct FBasicPauseValidationState
{
	TWeakObjectPtr<UAetherPauseMenuComponent> Component;
	TWeakObjectPtr<UAetherSettingsSubsystem> Settings;
	FAetherSettingsSnapshot OriginalSettings;
	FTSTicker::FDelegateHandle Ticker;
	double TimeoutAt = 0.0;
	bool bFailed = false;
	bool bRunning = false;
};

enum class EReturnValidationPhase : uint8
{
	AwaitMainMenu,
	AwaitGameplay
};

struct FReturnPauseValidationState
{
	TWeakObjectPtr<UGameInstance> GameInstance;
	FString OriginalSlot;
	int32 OriginalUserIndex = 0;
	FTSTicker::FDelegateHandle Ticker;
	double TimeoutAt = 0.0;
	EReturnValidationPhase Phase = EReturnValidationPhase::AwaitMainMenu;
	bool bFailed = false;
	bool bRunning = false;
};

FBasicPauseValidationState BasicState;
FReturnPauseValidationState ReturnState;

void RecordBasic(bool bCondition, const TCHAR* Label)
{
	UE_LOG(LogAetherPauseMenuValidation, Display, TEXT("[AetherPauseValidation] %s: %s"), Label, bCondition ? TEXT("PASS") : TEXT("FAIL"));
	BasicState.bFailed |= !bCondition;
}

void RecordReturn(bool bCondition, const TCHAR* Label)
{
	UE_LOG(LogAetherPauseMenuValidation, Display, TEXT("[AetherPauseReturnValidation] %s: %s"), Label, bCondition ? TEXT("PASS") : TEXT("FAIL"));
	ReturnState.bFailed |= !bCondition;
}
}

struct FAetherPauseMenuRuntimeValidationAccess
{
	static void SetSaveSlot(UAetherSaveSubsystem& Save, const FString& Slot, int32 UserIndex)
	{
		Save.PrototypeCheckpointSlotName = Slot;
		Save.PrototypeCheckpointUserIndex = UserIndex;
	}

	/** 설정 슬롯 격리를 먼저 확인한 뒤 일시 정지·포커스·표시 설정 확인 제한 시간을 검사한다. */
	static void RunBasic()
	{
		if (BasicState.bRunning)
		{
			return;
		}
		UWorld* World = FindRuntimeWorld();
		AAetherPlayerController* Controller = World ? Cast<AAetherPlayerController>(World->GetFirstPlayerController()) : nullptr;
		UAetherPauseMenuComponent* Component = Controller ? Controller->FindComponentByClass<UAetherPauseMenuComponent>() : nullptr;
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		UAetherSettingsSubsystem* Settings = GameInstance ? GameInstance->GetSubsystem<UAetherSettingsSubsystem>() : nullptr;
		if (!World || !Controller || !Component || !Settings)
		{
			UE_LOG(LogAetherPauseMenuValidation, Error, TEXT("[AetherPauseValidation] Runtime services unavailable"));
			FPlatformMisc::RequestExitWithStatus(false, 1);
			return;
		}

		FString ValidationSlot;
		if (!FParse::Value(FCommandLine::Get(), TEXT("AetherSettingsSlot="), ValidationSlot)
			|| ValidationSlot != PauseSettingsSlot)
		{
			UE_LOG(LogAetherPauseMenuValidation, Error, TEXT("[AetherPauseValidation] Isolated settings slot override is required"));
			FPlatformMisc::RequestExitWithStatus(false, 1);
			return;
		}

		BasicState = FBasicPauseValidationState();
		BasicState.bRunning = true;
		BasicState.Component = Component;
		BasicState.Settings = Settings;
		BasicState.OriginalSettings = Settings->GetCurrentSettings();
		UGameplayStatics::SetGamePaused(World, false);

		RecordBasic(Component->OpenPauseMenu(), TEXT("Escape/Menu open path creates pause menu"));
		UAetherMainMenuWidget* Widget = Component->PauseMenuWidget;
		RecordBasic(UGameplayStatics::IsGamePaused(World), TEXT("world simulation is paused"));
		RecordBasic(Widget && Widget->MenuContext == EAetherMenuContext::PauseMenu, TEXT("shared widget uses pause context"));
		RecordBasic(Controller->bShowMouseCursor && Controller->bEnableClickEvents && Controller->bEnableMouseOverEvents, TEXT("UI cursor mode enabled"));
		RecordBasic(Widget && Widget->ContinueButton && Widget->ContinueButton->GetIsEnabled(), TEXT("Resume is enabled"));
		RecordBasic(Widget && Widget->ContinueButton->HasKeyboardFocus(), TEXT("Resume receives initial focus"));
		RecordBasic(Widget && Widget->NewGameButton->GetVisibility() == ESlateVisibility::Collapsed
			&& Widget->LoadButton->GetVisibility() == ESlateVisibility::Collapsed
			&& Widget->CreditsButton->GetVisibility() == ESlateVisibility::Collapsed,
			TEXT("main-menu-only commands are hidden"));
		UAetherMainMenuWidget* OriginalWidget = Widget;
		RecordBasic(!Component->OpenPauseMenu() && Component->PauseMenuWidget == OriginalWidget, TEXT("duplicate open is rejected"));

		RecordBasic(RouteSlateKey(EKeys::Gamepad_DPad_Down), TEXT("gamepad navigation routes through Slate"));
		RecordBasic(Widget->SettingsButton->HasKeyboardFocus(), TEXT("gamepad focus advances to Settings"));
		RecordBasic(RouteSlateKey(EKeys::Gamepad_FaceButton_Bottom), TEXT("gamepad confirm routes through Slate"));
		RecordBasic(Widget->ActiveContentIndex == 2 && Widget->DisplayTabButton->HasKeyboardFocus(), TEXT("Settings opens with Display focus"));
		for (int32 Index = 0; Index < 4; ++Index)
		{
			Widget->ShowSettingsCategory(Index);
			RecordBasic(Widget->SettingsCategorySwitcher->GetActiveWidgetIndex() == Index, TEXT("pause settings exposes every category"));
		}

		Widget->HandleSettingsDefaultsClicked();
		RecordBasic(FMath::IsNearlyEqual(Settings->GetPendingSettings().Custom.Gamma, 2.2f), TEXT("Defaults uses shared settings contract"));
		RecordBasic(RouteSlateKey(EKeys::Gamepad_FaceButton_Right), TEXT("gamepad B cancels Settings"));
		RecordBasic(Widget->ActiveContentIndex == 0 && Widget->ContinueButton->HasKeyboardFocus(), TEXT("Settings returns to Resume focus"));

		Widget->QuitButton->SetKeyboardFocus();
		RouteSlateKey(EKeys::Enter);
		RecordBasic(Widget->ActivePopupAction == UAetherMainMenuWidget::EPopupAction::ReturnToMainMenu
			&& Widget->PopupConfirmButton->HasKeyboardFocus(), TEXT("Return popup captures focus"));
		RecordBasic(RouteSlateKey(EKeys::Escape), TEXT("Escape cancels Return popup"));
		RecordBasic(Widget->ActivePopupAction == UAetherMainMenuWidget::EPopupAction::None
			&& Widget->ContinueButton->HasKeyboardFocus() && UGameplayStatics::IsGamePaused(World),
			TEXT("popup cancel restores paused Resume focus"));

		Widget->HandleSettingsClicked();
		const int32 AlternateMode = BasicState.OriginalSettings.Video.WindowMode == EAetherWindowMode::Windowed ? 1 : 2;
		Widget->WindowModeCombo->SetSelectedIndex(AlternateMode);
		Widget->HandleSettingsApplyClicked();
		RecordBasic(Settings->IsAwaitingVideoConfirmation(), TEXT("display confirmation starts while paused"));
		RecordBasic(Widget->ActivePopupAction == UAetherMainMenuWidget::EPopupAction::VideoConfirmation, TEXT("video confirmation popup is visible"));
		BasicState.TimeoutAt = FPlatformTime::Seconds() + 18.0;
		BasicState.Ticker = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateStatic(&FAetherPauseMenuRuntimeValidationAccess::TickBasic),
			0.0f);
	}

	/** 월드가 멈춰 있어도 코어 틱커로 실제 시간 제한을 확인하고 표시 설정 복원 및 재개 입력을 검증한다. */
	static bool TickBasic(float DeltaTime)
	{
		UAetherSettingsSubsystem* Settings = BasicState.Settings.Get();
		UAetherPauseMenuComponent* Component = BasicState.Component.Get();
		UWorld* World = FindRuntimeWorld();
		if (!Settings || !Component || !World)
		{
			RecordBasic(false, TEXT("runtime services survive paused timeout"));
			BasicState.Ticker.Reset();
			FinishBasic();
			return false;
		}
		if (Settings->IsAwaitingVideoConfirmation() && FPlatformTime::Seconds() < BasicState.TimeoutAt)
		{
			return true;
		}
		if (Settings->IsAwaitingVideoConfirmation())
		{
			RecordBasic(false, TEXT("real-time video confirmation completed before validation timeout"));
			Settings->RevertVideoSettings();
		}
		else
		{
			RecordBasic(true, TEXT("real-time video confirmation completed before validation timeout"));
		}

		const FAetherSettingsSnapshot Restored = Settings->GetCurrentSettings();
		RecordBasic(Restored.Video.WindowMode == BasicState.OriginalSettings.Video.WindowMode
			&& Restored.Video.Resolution == BasicState.OriginalSettings.Video.Resolution,
			TEXT("display settings automatically revert while world remains paused"));
		RecordBasic(UGameplayStatics::IsGamePaused(World), TEXT("video timeout does not resume gameplay"));

		UAetherMainMenuWidget* Widget = Component->PauseMenuWidget;
		RecordBasic(Widget && Widget->ActivePopupAction == UAetherMainMenuWidget::EPopupAction::None
			&& Widget->ActiveContentIndex == 2, TEXT("timeout closes popup and keeps Settings active"));
		RecordBasic(RouteSlateKey(EKeys::Gamepad_FaceButton_Right), TEXT("B closes Settings after timeout"));
		RecordBasic(Widget && Widget->ActiveContentIndex == 0 && Widget->ContinueButton->HasKeyboardFocus(),
			TEXT("closing Settings restores Resume focus"));
		RecordBasic(RouteSlateKey(EKeys::Gamepad_Special_Right), TEXT("gamepad Menu closes pause menu"));
		AAetherPlayerController* Controller = Cast<AAetherPlayerController>(Component->GetOwner());
		RecordBasic(!Component->IsPauseMenuOpen() && !UGameplayStatics::IsGamePaused(World), TEXT("Resume restores unpaused world"));
		RecordBasic(Controller && !Controller->bShowMouseCursor && !Controller->bEnableClickEvents
			&& !Controller->bEnableMouseOverEvents, TEXT("Resume restores gameplay cursor state"));

		BasicState.Ticker.Reset();
		FinishBasic();
		return false;
	}

	static void FinishBasic()
	{
		UAetherSettingsSubsystem* Settings = BasicState.Settings.Get();
		UAetherPauseMenuComponent* Component = BasicState.Component.Get();
		if (Settings)
		{
			if (Settings->IsAwaitingVideoConfirmation())
			{
				Settings->RevertVideoSettings();
			}
			Settings->BeginSettingsEdit();
			Settings->SetPendingSettings(BasicState.OriginalSettings);
			if (Settings->ApplyPendingSettings() && Settings->IsAwaitingVideoConfirmation())
			{
				Settings->ConfirmVideoSettings();
			}
		}
		if (Component && Component->IsPauseMenuOpen())
		{
			Component->bTransitionRequested = false;
			Component->ClosePauseMenu();
		}
		if (UWorld* World = FindRuntimeWorld())
		{
			UGameplayStatics::SetGamePaused(World, false);
		}
		const bool bDeleted = !UGameplayStatics::DoesSaveGameExist(PauseSettingsSlot, 0)
			|| UGameplayStatics::DeleteGameInSlot(PauseSettingsSlot, 0);
		RecordBasic(bDeleted, TEXT("isolated settings fixture removed"));
		UE_LOG(LogAetherPauseMenuValidation, Display, TEXT("[AetherPauseValidation] RESULT: %s"), BasicState.bFailed ? TEXT("FAIL") : TEXT("PASS"));
		const bool bFailed = BasicState.bFailed;
		BasicState = FBasicPauseValidationState();
		FPlatformMisc::RequestExitWithStatus(false, bFailed ? 1 : 0);
	}

	/** 검증용 체크포인트를 만들고 일시 정지 메뉴에서 복귀·계속하기 후 저장 상태가 유지되는지 확인한다. */
	static void RunReturnContinue()
	{
		if (ReturnState.bRunning)
		{
			return;
		}
		UWorld* World = FindRuntimeWorld();
		AAetherPlayerController* Controller = World ? Cast<AAetherPlayerController>(World->GetFirstPlayerController()) : nullptr;
		UAetherPauseMenuComponent* Component = Controller ? Controller->FindComponentByClass<UAetherPauseMenuComponent>() : nullptr;
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		UAetherSaveSubsystem* Save = GameInstance ? GameInstance->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
		UAetherMenuFlowSubsystem* Flow = GameInstance ? GameInstance->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr;
		if (!World || !Controller || !Component || !GameInstance || !Save || !Flow)
		{
			UE_LOG(LogAetherPauseMenuValidation, Error, TEXT("[AetherPauseReturnValidation] Runtime services unavailable"));
			FPlatformMisc::RequestExitWithStatus(false, 1);
			return;
		}

		ReturnState = FReturnPauseValidationState();
		ReturnState.bRunning = true;
		ReturnState.GameInstance = GameInstance;
		ReturnState.OriginalSlot = Save->PrototypeCheckpointSlotName;
		ReturnState.OriginalUserIndex = Save->PrototypeCheckpointUserIndex;
		SetSaveSlot(*Save, PauseProgressSlot, PauseProgressUserIndex);
		Save->ClearPrototypeCheckpointSnapshot();

		UAetherPrototypeSaveGame* Fixture = NewObject<UAetherPrototypeSaveGame>(GameInstance);
		FAetherPrototypeSaveSchemaPolicy::StampCurrentSchema(*Fixture);
		Fixture->bHasActiveCheckpoint = true;
		Fixture->ActiveCheckpointLabel = FName(TEXT("CP_ForestStart"));
		Fixture->ActiveCheckpointProgressRank = 0;
		Fixture->ActiveCheckpointTransform = FTransform(FRotator::ZeroRotator, FVector(0.0, 0.0, 150.0));
		RecordReturn(Save->SavePrototypeCheckpointSnapshot(Fixture), TEXT("isolated checkpoint fixture created"));
		RecordReturn(Component->OpenPauseMenu(), TEXT("pause menu opens before return"));
		UAetherMainMenuWidget* Widget = Component->PauseMenuWidget;
		if (!Widget)
		{
			RecordReturn(false, TEXT("pause widget exists before return"));
			FinishReturn();
			return;
		}

		Widget->HandleQuitClicked();
		RecordReturn(Widget->ActivePopupAction == UAetherMainMenuWidget::EPopupAction::ReturnToMainMenu,
			TEXT("Return requires confirmation"));
		Widget->HandlePopupConfirmClicked();
		RecordReturn(Component->bTransitionRequested && Flow->IsTransitionInProgress(), TEXT("confirmed Return enters guarded transition"));
		RecordReturn(!UGameplayStatics::IsGamePaused(World), TEXT("Return releases world pause for map travel"));
		RecordReturn(Save->HasPrototypeCheckpointSnapshot(), TEXT("Return does not clear checkpoint save"));

		ReturnState.Phase = EReturnValidationPhase::AwaitMainMenu;
		ReturnState.TimeoutAt = FPlatformTime::Seconds() + 35.0;
		ReturnState.Ticker = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateStatic(&FAetherPauseMenuRuntimeValidationAccess::TickReturn),
			0.0f);
	}

	static bool TickReturn(float DeltaTime)
	{
		UWorld* World = FindRuntimeWorld();
		UGameInstance* GameInstance = ReturnState.GameInstance.Get();
		UAetherSaveSubsystem* Save = GameInstance ? GameInstance->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
		UAetherMenuFlowSubsystem* Flow = GameInstance ? GameInstance->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr;
		if (!World || !Save || !Flow || FPlatformTime::Seconds() >= ReturnState.TimeoutAt)
		{
			RecordReturn(false, TEXT("Return/Continue transition completed before timeout"));
			ReturnState.Ticker.Reset();
			FinishReturn();
			return false;
		}

		const FString MapName = World->GetMapName();
		if (ReturnState.Phase == EReturnValidationPhase::AwaitMainMenu
			&& MapName.Contains(TEXT("M_MainMenu")) && !Flow->IsTransitionInProgress())
		{
			const FAetherSaveSlotSummary Summary = Save->GetPrototypeCheckpointSummary();
			RecordReturn(Summary.bLoadable && Summary.CheckpointLabel == FName(TEXT("CP_ForestStart")),
				TEXT("main menu sees preserved checkpoint"));
			ReturnState.Phase = EReturnValidationPhase::AwaitGameplay;
			RecordReturn(Flow->ContinueGame(), TEXT("Continue accepts preserved checkpoint"));
			return true;
		}

		if (ReturnState.Phase == EReturnValidationPhase::AwaitGameplay
			&& MapName.Contains(TEXT("M_VerticalSlice")) && !Flow->IsTransitionInProgress())
		{
			const AAetherGameModeBase* GameMode = World->GetAuthGameMode<AAetherGameModeBase>();
			RecordReturn(Save->HasPrototypeCheckpointSnapshot(), TEXT("Continue keeps isolated checkpoint save"));
			RecordReturn(GameMode && GameMode->HasActivePrototypeCheckpoint()
				&& GameMode->GetActivePrototypeCheckpointLabel() == FName(TEXT("CP_ForestStart")),
				TEXT("Continue restores checkpoint into gameplay GameMode"));
			RecordReturn(true, TEXT("Return/Continue transition completed before timeout"));
			ReturnState.Ticker.Reset();
			FinishReturn();
			return false;
		}
		return true;
	}

	static void FinishReturn()
	{
		UGameInstance* GameInstance = ReturnState.GameInstance.Get();
		UAetherSaveSubsystem* Save = GameInstance ? GameInstance->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
		if (Save)
		{
			Save->ClearPrototypeCheckpointSnapshot();
			SetSaveSlot(*Save, ReturnState.OriginalSlot, ReturnState.OriginalUserIndex);
		}
		else
		{
			RecordReturn(false, TEXT("save subsystem survives transition cleanup"));
		}
		if (UWorld* World = FindRuntimeWorld())
		{
			UGameplayStatics::SetGamePaused(World, false);
		}
		UE_LOG(LogAetherPauseMenuValidation, Display, TEXT("[AetherPauseReturnValidation] RESULT: %s"),
			ReturnState.bFailed ? TEXT("FAIL") : TEXT("PASS"));
		const bool bFailed = ReturnState.bFailed;
		ReturnState = FReturnPauseValidationState();
		FPlatformMisc::RequestExitWithStatus(false, bFailed ? 1 : 0);
	}
};

namespace
{
FAutoConsoleCommand RunPauseMenuValidationCommand(
	TEXT("Aether.Pause.ValidateRuntime"),
	TEXT("Validates pause lifecycle, shared settings UI, focus and real-time video revert."),
	FConsoleCommandDelegate::CreateStatic(&FAetherPauseMenuRuntimeValidationAccess::RunBasic));

FAutoConsoleCommand RunPauseReturnValidationCommand(
	TEXT("Aether.Pause.ValidateReturnContinue"),
	TEXT("Validates pause Return confirmation and checkpoint-preserving Continue across map travel."),
	FConsoleCommandDelegate::CreateStatic(&FAetherPauseMenuRuntimeValidationAccess::RunReturnContinue));
}

#endif

