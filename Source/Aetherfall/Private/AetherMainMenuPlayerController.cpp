/** 로컬 플레이어의 메뉴 위젯을 생성하고 UI 전용 입력·포커스를 설정한 뒤 화면 준비를 알린다. */
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

/** 메뉴 Blueprint 클래스를 우선 로드하고 실패하면 C++ 위젯으로 대체해 UI 입력과 키보드 포커스를 연결한다. */
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
