#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AetherMainMenuGameMode.generated.h"

/** 메인 메뉴에서 플레이어 폰과 전투 HUD 생성을 억제하고 메뉴 전용 컨트롤러를 사용한다. */
UCLASS()
class AETHERFALL_API AAetherMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAetherMainMenuGameMode();

protected:
	/** 메뉴에는 플레이어 시작 위치가 필요하지 않으므로 오류를 비우고 시작 위치 검사를 통과시킨다. */
	virtual bool UpdatePlayerStartSpot(AController* Player, const FString& Portal, FString& OutErrorMessage) override;
	virtual void InitializeHUDForPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
};
