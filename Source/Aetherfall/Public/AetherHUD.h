#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AetherHUD.generated.h"

class AAetherEnemyBase;
class AAetherGameModeBase;
class AAetherfallCharacter;
class UAetherCombatComponent;
class UAetherLockOnComponent;

UCLASS()
class AETHERFALL_API AAetherHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawPlayerStatus(const AAetherfallCharacter* PlayerCharacter);
	void DrawPrototypeQuickItemStatus(const AAetherfallCharacter* PlayerCharacter);
	void DrawPlayerDangerStatus(const AAetherfallCharacter* PlayerCharacter);
	void DrawEnemyStatus(const AAetherfallCharacter* PlayerCharacter);
	void DrawBossEnemyStatus(const AAetherEnemyBase* Enemy, bool bHasLockedTarget, const UAetherCombatComponent* CombatComponent);
	void DrawIncomingThreatStatus(const AAetherfallCharacter* PlayerCharacter);
	void DrawPrototypeRoundStatus();
	void DrawPrototypeLevelStatus();
	void DrawPrototypeCheckpointStatus();
	void DrawPrototypeCheckpointFeedback();
	void DrawPrototypeProgressFeedback();
	void DrawPrototypeDialogue();
	void DrawPrototypeRouteGuidance(const AAetherfallCharacter* PlayerCharacter);
	void DrawInteractionPrompt(const AAetherfallCharacter* PlayerCharacter);
	void DrawLockOnReticle(const AAetherEnemyBase* Enemy);
	void DrawThreatReticle(const AAetherEnemyBase* Enemy);
	void DrawStatusBar(const FString& Label, float CurrentValue, float MaxValue, float X, float Y, float Width, float Height, const FLinearColor& FillColor);
	FString GetPrototypeProgressStatusLabel(const AAetherGameModeBase* AetherGameMode) const;
	AAetherEnemyBase* FindNearestLivingEnemy(const AAetherfallCharacter* PlayerCharacter) const;
	AAetherEnemyBase* FindIncomingThreatEnemy(const AAetherfallCharacter* PlayerCharacter) const;
};
