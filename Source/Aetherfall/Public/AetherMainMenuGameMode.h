#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AetherMainMenuGameMode.generated.h"

UCLASS()
class AETHERFALL_API AAetherMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAetherMainMenuGameMode();

protected:
	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	virtual void InitializeHUDForPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
};
