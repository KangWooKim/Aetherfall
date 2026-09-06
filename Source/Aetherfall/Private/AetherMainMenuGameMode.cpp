/** 메인 메뉴에서 플레이어 폰과 전투 HUD 생성을 억제하고 메뉴 전용 컨트롤러를 사용한다. */
#include "AetherMainMenuGameMode.h"

#include "AetherMainMenuPlayerController.h"

AAetherMainMenuGameMode::AAetherMainMenuGameMode()
{
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	PlayerControllerClass = AAetherMainMenuPlayerController::StaticClass();
	bStartPlayersAsSpectators = false;
}

/** 메뉴에는 플레이어 시작 위치가 필요하지 않으므로 오류를 비우고 시작 위치 검사를 통과시킨다. */
bool AAetherMainMenuGameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	OutErrorMessage.Reset();
	return true;
}

void AAetherMainMenuGameMode::InitializeHUDForPlayer_Implementation(APlayerController* NewPlayer)
{
	// 메인 메뉴는 플레이용 기본 Pawn을 생성하지 않는다.
}

void AAetherMainMenuGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// 메뉴 전용 컨트롤러가 화면과 UI 입력을 구성한다.
}

void AAetherMainMenuGameMode::RestartPlayer(AController* NewPlayer)
{
	// 실제 메뉴 화면 구성은 컨트롤러와 위젯 초기화 경로에서 처리한다.
}
