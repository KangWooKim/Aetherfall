#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AetherHUD.generated.h"

class AAetherEnemyBase;
class AAetherGameModeBase;
class AAetherfallCharacter;
class UAetherCombatComponent;
class UAetherLockOnComponent;

/** 캐릭터와 진행 상태를 캔버스 기반 HUD로 그리며, 표시 문구와 길 안내는 별도 표시 정책을 활용한다. */
UCLASS()
class AETHERFALL_API AAetherHUD : public AHUD
{
	GENERATED_BODY()

public:
	/** 캔버스와 플레이어가 준비되고 연출의 HUD 숨김 요청이 없을 때 각 상태 영역을 그린다. */
	virtual void DrawHUD() override;

private:
	void DrawPlayerStatus(const AAetherfallCharacter* PlayerCharacter);
	void DrawPrototypeQuickItemStatus(const AAetherfallCharacter* PlayerCharacter);
	void DrawPlayerDangerStatus(const AAetherfallCharacter* PlayerCharacter);
	/** 락온 대상을 우선하며, 락온이 없으면 가장 가까운 생존 적의 체력과 처형 가능 상태를 표시한다. */
	void DrawEnemyStatus(const AAetherfallCharacter* PlayerCharacter);
	void DrawBossEnemyStatus(const AAetherEnemyBase* Enemy, bool bHasLockedTarget, const UAetherCombatComponent* CombatComponent);
	/** 공격을 예고 중인 가장 가까운 적의 패턴과 방어 힌트를 표시한다. */
	void DrawIncomingThreatStatus(const AAetherfallCharacter* PlayerCharacter);
	void DrawPrototypeRoundStatus();
	void DrawPrototypeLevelStatus();
	void DrawPrototypeCheckpointStatus();
	void DrawPrototypeCheckpointFeedback();
	void DrawPrototypeProgressFeedback();
	/** 자막 활성화와 크기 설정을 적용해 현재 대사와 목표 힌트를 그린다. */
	void DrawPrototypeDialogue();
	/** 길 안내 정책이 반환한 목표와 조작 힌트를 캔버스 영역에 표시한다. */
	void DrawPrototypeRouteGuidance(const AAetherfallCharacter* PlayerCharacter);
	void DrawInteractionPrompt(const AAetherfallCharacter* PlayerCharacter);
	void DrawLockOnReticle(const AAetherEnemyBase* Enemy);
	void DrawThreatReticle(const AAetherEnemyBase* Enemy);
	void DrawStatusBar(const FString& Label, float CurrentValue, float MaxValue, float X, float Y, float Width, float Height, const FLinearColor& FillColor);
	FString GetPrototypeProgressStatusLabel(const AAetherGameModeBase* AetherGameMode) const;
	AAetherEnemyBase* FindNearestLivingEnemy(const AAetherfallCharacter* PlayerCharacter) const;
	/** 생존하며 공격 예고 중인 적을 월드에서 검색해 가장 가까운 위협을 반환한다. */
	AAetherEnemyBase* FindIncomingThreatEnemy(const AAetherfallCharacter* PlayerCharacter) const;
};
