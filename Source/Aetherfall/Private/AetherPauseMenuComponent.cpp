/** 플레이어 컨트롤러의 일시 정지 메뉴 수명과 입력 전환을 관리하고 공유 메뉴 위젯을 재사용한다. */
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

/** 로컬 컨트롤러, 생존 플레이어, 게임 모드와 전환·재시작 상태를 확인해 메뉴 열기 가능 여부를 판단한다. */
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

/** 공유 위젯을 일시 정지 문맥으로 표시하고 월드 정지에 성공하면 UI 전용 입력과 포커스를 설정한다. */
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

/** 보관된 위젯이 있으면 재사용하고, 없으면 Blueprint 클래스를 우선해 새 인스턴스를 만든다. */
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

/** 이벤트 연결과 화면 표시를 제거하되 위젯 참조를 남겨 재사용하고 게임 입력을 복원한다. */
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

/** 중복 복귀 요청을 막고 맵 이동 전에 정지를 해제한다. 요청이 즉시 거절되면 일시 정지를 복원한다. */
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

/** 위젯과 델리게이트를 최종 정리하고 맵 전환 중이 아닌 경우 게임 입력도 복원한다. */
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
