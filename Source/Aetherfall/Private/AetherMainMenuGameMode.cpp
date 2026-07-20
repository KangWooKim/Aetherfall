#include "AetherMainMenuGameMode.h"

#include "AetherMainMenuPlayerController.h"

AAetherMainMenuGameMode::AAetherMainMenuGameMode()
{
	DefaultPawnClass = nullptr;
	HUDClass = nullptr;
	PlayerControllerClass = AAetherMainMenuPlayerController::StaticClass();
	bStartPlayersAsSpectators = false;
}

bool AAetherMainMenuGameMode::UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage)
{
	OutErrorMessage.Reset();
	return true;
}

void AAetherMainMenuGameMode::InitializeHUDForPlayer_Implementation(APlayerController* NewPlayer)
{
	// The menu is entirely UMG-driven and intentionally has no AHUD instance.
}

void AAetherMainMenuGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// The menu controller owns an explicit camera and UMG layer; no world pawn is required.
}

void AAetherMainMenuGameMode::RestartPlayer(AController* NewPlayer)
{
	// Local controllers may request a restart after login; the menu remains pawnless by design.
}
