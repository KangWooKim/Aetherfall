/** 비배포 빌드에서 실제 메뉴 위젯의 포커스·키보드·게임패드·마우스 입력 경로를 확인하고 결과 종료 코드를 반환하는 검증 명령이다. */
#include "AetherMainMenuWidget.h"

#if !UE_BUILD_SHIPPING

#include "AetherSaveSubsystem.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/UObjectIterator.h"

DEFINE_LOG_CATEGORY_STATIC(LogAetherMenuWidgetValidation, Log, All);

namespace
{
bool bValidationRunning = false;
bool bValidationFailed = false;

void RecordWidgetCheck(bool bCondition, const TCHAR* CheckName)
{
	bValidationFailed |= !bCondition;
	UE_LOG(
		LogAetherMenuWidgetValidation,
		Display,
		TEXT("[AetherMenuWidgetValidation] %s: %s"),
		bCondition ? TEXT("PASS") : TEXT("FAIL"),
		CheckName);
}

UWorld* FindRuntimeWorld()
{
	if (!GEngine)
	{
		return nullptr;
	}

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		UWorld* World = Context.World();
		if (World && (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE))
		{
			return World;
		}
	}
	return nullptr;
}

UAetherMainMenuWidget* FindMenuWidget(UWorld* World)
{
	for (TObjectIterator<UAetherMainMenuWidget> It; It; ++It)
	{
		if (It->GetWorld() == World && It->IsInViewport())
		{
			return *It;
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

	const FKeyEvent Event(Key, FModifierKeysState(), 0, false, 0, 0);
	const bool bHandled = FSlateApplication::Get().ProcessKeyDownEvent(Event);
	FSlateApplication::Get().ProcessKeyUpEvent(Event);
	return bHandled;
}

bool RouteSlateMouseClick(UButton* Button)
{
	if (!Button || !FSlateApplication::IsInitialized())
	{
		return false;
	}

	const FGeometry& Geometry = Button->GetCachedGeometry();
	const FVector2D Size = Geometry.GetAbsoluteSize();
	if (Size.X <= 0.0f || Size.Y <= 0.0f)
	{
		return false;
	}

	const FVector2D Center = Geometry.GetAbsolutePosition() + Size * 0.5f;
	TSet<FKey> NoPressedButtons;
	const FPointerEvent MoveEvent(
		0,
		Center,
		FVector2D::ZeroVector,
		NoPressedButtons,
		EKeys::Invalid,
		0.0f,
		FModifierKeysState());
	FSlateApplication::Get().ProcessMouseMoveEvent(MoveEvent, true);
	const bool bHovered = Button->IsHovered();

	TSet<FKey> PressedButtons;
	PressedButtons.Add(EKeys::LeftMouseButton);
	const FPointerEvent DownEvent(
		0,
		Center,
		Center,
		PressedButtons,
		EKeys::LeftMouseButton,
		0.0f,
		FModifierKeysState());
	const bool bDownHandled = FSlateApplication::Get().ProcessMouseButtonDownEvent(nullptr, DownEvent);
	const FPointerEvent UpEvent(
		0,
		Center,
		Center,
		NoPressedButtons,
		EKeys::LeftMouseButton,
		0.0f,
		FModifierKeysState());
	const bool bUpHandled = FSlateApplication::Get().ProcessMouseButtonUpEvent(UpEvent);
	return bHovered && bDownHandled && bUpHandled;
}
}

struct FAetherMainMenuWidgetValidationAccess
{
	static void Validate()
	{
		UWorld* World = FindRuntimeWorld();
		UAetherMainMenuWidget* Widget = FindMenuWidget(World);
		if (!World || !Widget)
		{
			UE_LOG(LogAetherMenuWidgetValidation, Error, TEXT("[AetherMenuWidgetValidation] Runtime menu widget unavailable"));
			bValidationRunning = false;
			FPlatformMisc::RequestExitWithStatus(false, 1);
			return;
		}

		RecordWidgetCheck(Widget->ContentSwitcher && Widget->SettingsCategorySwitcher, TEXT("screen switchers exist"));
		RecordWidgetCheck(
			Widget->ContinueButton && Widget->NewGameButton && Widget->LoadButton && Widget->SettingsButton
				&& Widget->CreditsButton && Widget->QuitButton,
			TEXT("all primary menu buttons exist"));
		RecordWidgetCheck(
			Widget->PopupOverlay && Widget->PopupConfirmButton && Widget->PopupCancelButton,
			TEXT("modal popup controls exist"));

		if (!Widget->ContentSwitcher || !Widget->SettingsCategorySwitcher || !Widget->SettingsButton
			|| !Widget->CreditsButton || !Widget->QuitButton || !Widget->PopupOverlay)
		{
			Finish();
			return;
		}

		Widget->ShowMainScreen();
		RecordWidgetCheck(Widget->ContentSwitcher->GetActiveWidgetIndex() == 0, TEXT("main screen is active"));

		UGameInstance* GameInstance = World->GetGameInstance();
		UAetherSaveSubsystem* SaveSubsystem = GameInstance ? GameInstance->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
		if (SaveSubsystem)
		{
			const FAetherSaveSlotSummary Summary = SaveSubsystem->GetPrototypeCheckpointSummary();
			RecordWidgetCheck(Widget->ContinueButton->GetIsEnabled() == Summary.bLoadable, TEXT("Continue enabled state matches save loadability"));
			RecordWidgetCheck(Widget->LoadButton->GetIsEnabled() == Summary.bExists, TEXT("Load enabled state matches save existence"));
		}
		else
		{
			RecordWidgetCheck(false, TEXT("save subsystem exists"));
		}

		Widget->SettingsButton->SetKeyboardFocus();
		RecordWidgetCheck(Widget->SettingsButton->HasKeyboardFocus(), TEXT("keyboard focus can target a primary button"));
		RecordWidgetCheck(RouteSlateKey(EKeys::Gamepad_DPad_Down), TEXT("gamepad directional input routed through Slate"));
		RecordWidgetCheck(Widget->CreditsButton->HasKeyboardFocus(), TEXT("gamepad directional input advances focus"));

		Widget->SettingsButton->SetKeyboardFocus();
		RecordWidgetCheck(RouteSlateKey(EKeys::Enter), TEXT("keyboard confirm input routed through Slate"));
		RecordWidgetCheck(Widget->ContentSwitcher->GetActiveWidgetIndex() == 2, TEXT("keyboard confirm opens Settings"));
		RecordWidgetCheck(Widget->DisplayTabButton->HasKeyboardFocus(), TEXT("Settings restores category focus"));

		for (int32 CategoryIndex = 0; CategoryIndex < 4; ++CategoryIndex)
		{
			Widget->ShowSettingsCategory(CategoryIndex);
			RecordWidgetCheck(
				Widget->SettingsCategorySwitcher->GetActiveWidgetIndex() == CategoryIndex,
				TEXT("settings category switcher accepts every category"));
		}

		const FKeyEvent GamepadBack(EKeys::Gamepad_FaceButton_Right, FModifierKeysState(), 0, false, 0, 0);
		RecordWidgetCheck(Widget->NativeOnKeyDown(FGeometry(), GamepadBack).IsEventHandled(), TEXT("gamepad B is handled as Back"));
		RecordWidgetCheck(Widget->ContentSwitcher->GetActiveWidgetIndex() == 0, TEXT("gamepad B returns Settings to main"));

		Widget->QuitButton->SetKeyboardFocus();
		RecordWidgetCheck(RouteSlateKey(EKeys::Gamepad_FaceButton_Bottom), TEXT("gamepad confirm input routed through Slate"));
		RecordWidgetCheck(
			Widget->PopupOverlay->GetVisibility() == ESlateVisibility::Visible
				&& Widget->ActivePopupAction == UAetherMainMenuWidget::EPopupAction::QuitGame,
			TEXT("gamepad confirm opens Quit confirmation"));
		RecordWidgetCheck(Widget->PopupConfirmButton->HasKeyboardFocus(), TEXT("Quit popup captures focus"));

		const FKeyEvent Escape(EKeys::Escape, FModifierKeysState(), 0, false, 0, 0);
		RecordWidgetCheck(Widget->NativeOnKeyDown(FGeometry(), Escape).IsEventHandled(), TEXT("Escape is handled as cancel"));
		RecordWidgetCheck(
			Widget->PopupOverlay->GetVisibility() == ESlateVisibility::Collapsed
				&& Widget->ActivePopupAction == UAetherMainMenuWidget::EPopupAction::None,
			TEXT("Escape closes popup without quitting"));

		Widget->ShowMainScreen();
		RecordWidgetCheck(RouteSlateMouseClick(Widget->CreditsButton), TEXT("mouse hover and click route through Slate"));
		RecordWidgetCheck(Widget->ContentSwitcher->GetActiveWidgetIndex() == 3, TEXT("mouse click opens Credits"));
		Widget->NativeOnKeyDown(FGeometry(), GamepadBack);

		Widget->LoadButton->SetKeyboardFocus();
		if (Widget->LoadButton->GetIsEnabled())
		{
			RouteSlateKey(EKeys::Enter);
			RecordWidgetCheck(Widget->ContentSwitcher->GetActiveWidgetIndex() == 1, TEXT("Load command opens load summary"));
			Widget->NativeOnKeyDown(FGeometry(), GamepadBack);
		}
		else
		{
			RecordWidgetCheck(true, TEXT("Load command correctly disabled without a save"));
		}

		Widget->CreditsButton->SetKeyboardFocus();
		RouteSlateKey(EKeys::Gamepad_FaceButton_Bottom);
		RecordWidgetCheck(Widget->ContentSwitcher->GetActiveWidgetIndex() == 3, TEXT("gamepad confirm opens Credits"));
		Widget->NativeOnKeyDown(FGeometry(), GamepadBack);
		RecordWidgetCheck(Widget->ContentSwitcher->GetActiveWidgetIndex() == 0, TEXT("Credits Back returns to main"));

		Finish();
	}

	static void Finish()
	{
		UE_LOG(
			LogAetherMenuWidgetValidation,
			Display,
			TEXT("[AetherMenuWidgetValidation] RESULT: %s"),
			bValidationFailed ? TEXT("FAIL") : TEXT("PASS"));
		bValidationRunning = false;
		FPlatformMisc::RequestExitWithStatus(false, bValidationFailed ? 1 : 0);
	}

	/** 종료 확인 팝업을 확인한 후 실제 종료를 요청하며, 제한 시간 내 종료되지 않으면 실패로 처리한다. */
	static void ValidateQuitConfirmation()
	{
		UWorld* World = FindRuntimeWorld();
		UAetherMainMenuWidget* Widget = FindMenuWidget(World);
		if (!World || !Widget || !Widget->PopupOverlay)
		{
			UE_LOG(LogAetherMenuWidgetValidation, Error, TEXT("[AetherMenuQuitValidation] Runtime menu widget unavailable"));
			FPlatformMisc::RequestExitWithStatus(false, 1);
			return;
		}

		Widget->HandleQuitClicked();
		const bool bPopupReady = Widget->PopupOverlay->GetVisibility() == ESlateVisibility::Visible
			&& Widget->ActivePopupAction == UAetherMainMenuWidget::EPopupAction::QuitGame
			&& Widget->PopupConfirmButton
			&& Widget->PopupConfirmButton->HasKeyboardFocus();
		UE_LOG(
			LogAetherMenuWidgetValidation,
			Display,
			TEXT("[AetherMenuQuitValidation] CONFIRMATION RESULT: %s"),
			bPopupReady ? TEXT("PASS") : TEXT("FAIL"));
		if (!bPopupReady)
		{
			FPlatformMisc::RequestExitWithStatus(false, 1);
			return;
		}

		UE_LOG(LogAetherMenuWidgetValidation, Display, TEXT("[AetherMenuQuitValidation] Dispatching confirmed Quit request"));
		Widget->HandlePopupConfirmClicked();
		FTimerHandle QuitTimeoutHandle;
		World->GetTimerManager().SetTimer(QuitTimeoutHandle, FTimerDelegate::CreateLambda([]
		{
			UE_LOG(LogAetherMenuWidgetValidation, Error, TEXT("[AetherMenuQuitValidation] Quit request did not exit"));
			FPlatformMisc::RequestExitWithStatus(false, 1);
		}), 2.0f, false);
	}

	static void ShowSettingsCategoryForCapture(int32 CategoryIndex)
	{
		UWorld* World = FindRuntimeWorld();
		UAetherMainMenuWidget* Widget = FindMenuWidget(World);
		if (!Widget)
		{
			UE_LOG(LogAetherMenuWidgetValidation, Error, TEXT("[AetherMenuCapture] Runtime menu widget unavailable"));
			return;
		}
		Widget->ShowSettingsScreen();
		Widget->ShowSettingsCategory(CategoryIndex);
		UE_LOG(LogAetherMenuWidgetValidation, Display, TEXT("[AetherMenuCapture] Showing settings category %d"), CategoryIndex);
	}
};

namespace
{
/** 실행 월드가 준비된 후 짧은 지연을 두어 위젯 배치와 입력 경로를 검증한다. */
void RunMenuWidgetRuntimeValidation()
{
	if (bValidationRunning)
	{
		UE_LOG(LogAetherMenuWidgetValidation, Warning, TEXT("[AetherMenuWidgetValidation] Validation already running"));
		return;
	}

	UWorld* World = FindRuntimeWorld();
	if (!World)
	{
		UE_LOG(LogAetherMenuWidgetValidation, Error, TEXT("[AetherMenuWidgetValidation] Runtime world unavailable"));
		FPlatformMisc::RequestExitWithStatus(false, 1);
		return;
	}

	bValidationRunning = true;
	bValidationFailed = false;
	FTimerHandle ValidationDelayHandle;
	World->GetTimerManager().SetTimer(
		ValidationDelayHandle,
		FTimerDelegate::CreateStatic(&FAetherMainMenuWidgetValidationAccess::Validate),
		0.25f,
		false);
}

void RunMenuQuitRuntimeValidation()
{
	UWorld* World = FindRuntimeWorld();
	if (!World)
	{
		UE_LOG(LogAetherMenuWidgetValidation, Error, TEXT("[AetherMenuQuitValidation] Runtime world unavailable"));
		FPlatformMisc::RequestExitWithStatus(false, 1);
		return;
	}
	World->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateStatic(&FAetherMainMenuWidgetValidationAccess::ValidateQuitConfirmation));
}

void ShowSettingsCategoryForCapture(const TArray<FString>& Arguments)
{
	UWorld* World = FindRuntimeWorld();
	if (!World)
	{
		UE_LOG(LogAetherMenuWidgetValidation, Error, TEXT("[AetherMenuCapture] Runtime world unavailable"));
		return;
	}

	const int32 CategoryIndex = Arguments.IsEmpty() ? 0 : FMath::Clamp(FCString::Atoi(*Arguments[0]), 0, 3);
	World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda([CategoryIndex]
	{
		FAetherMainMenuWidgetValidationAccess::ShowSettingsCategoryForCapture(CategoryIndex);
	}));
}

FAutoConsoleCommand RunMenuWidgetValidationCommand(
	TEXT("Aether.Menu.ValidateWidgetRuntime"),
	TEXT("Runs non-shipping menu widget focus/navigation validation and exits with its result."),
	FConsoleCommandDelegate::CreateStatic(&RunMenuWidgetRuntimeValidation));

FAutoConsoleCommand RunMenuQuitValidationCommand(
	TEXT("Aether.Menu.ValidateQuitRuntime"),
	TEXT("Opens the Quit confirmation, confirms it, and expects the process to exit."),
	FConsoleCommandDelegate::CreateStatic(&RunMenuQuitRuntimeValidation));

FAutoConsoleCommand ShowSettingsCategoryCaptureCommand(
	TEXT("Aether.Menu.ShowSettingsCategory"),
	TEXT("Shows a settings category (0-3) for non-shipping visual capture."),
	FConsoleCommandWithArgsDelegate::CreateStatic(&ShowSettingsCategoryForCapture));
}

#endif
