/** 게임 모드 진행 정보에서 HUD용 목표·조작 안내·중요 구간 표시를 만드는 경로 안내 정책이다. */
#include "AetherPrototypeRouteGuide.h"

#include "AetherCombatComponent.h"
#include "AetherGameModeBase.h"
#include "AetherHealthComponent.h"
#include "AetherPrototypeLabels.h"
#include "AetherfallCharacter.h"

FAetherPrototypeRouteGuidanceViewData FAetherPrototypeRouteGuide::BuildGuidance(
	const AAetherGameModeBase* AetherGameMode,
	const AAetherfallCharacter* PlayerCharacter)
{
	FAetherPrototypeRouteGuidanceViewData Guidance;
	Guidance.ObjectiveLabel = BuildObjectiveLabel(AetherGameMode);
	Guidance.TutorialHintLabel = BuildTutorialHintLabel(AetherGameMode, PlayerCharacter);
	Guidance.bCriticalObjective = IsCriticalObjective(AetherGameMode);
	return Guidance;
}

/** 종료·보스전·활성 전투 등 우선순위에 따라 현재 목표 문구를 선택한다. */
FString FAetherPrototypeRouteGuide::BuildObjectiveLabel(const AAetherGameModeBase* AetherGameMode)
{
	if (!AetherGameMode)
	{
		return TEXT("");
	}

	if (AetherGameMode->IsPrototypeLevelComplete())
	{
		return AetherGameMode->IsCathedralEndingComplete()
			? TEXT("ROUTE SUMMARY: RIFT SEALED - ROUTE COMPLETE")
			: TEXT("ROUTE SUMMARY: LEVEL GOAL COMPLETE");
	}

	const FName ActiveEncounterLabel = AetherGameMode->GetActivePrototypeEncounterLabel();
	if (AetherGameMode->IsPrototypeBossEncounterActive())
	{
		return TEXT("ROUTE SUMMARY: AUREL HOLDS THE RIFT");
	}

	if (!ActiveEncounterLabel.IsNone())
	{
		if (ActiveEncounterLabel == AetherPrototypeLabels::LowerCryptElite())
		{
			return TEXT("ROUTE SUMMARY: CAEL GUARDS THE CATHEDRAL PATH");
		}

		if (ActiveEncounterLabel == AetherPrototypeLabels::HamletEntry())
		{
			return TEXT("ROUTE SUMMARY: FALLEN HAMLET IS STILL HOSTILE");
		}

		if (ActiveEncounterLabel == AetherPrototypeLabels::ForestIntro())
		{
			return TEXT("ROUTE SUMMARY: FIRST HOLLOW KNIGHT CONTACT");
		}

		return FString::Printf(TEXT("ROUTE SUMMARY: ENCOUNTER %s"), *ActiveEncounterLabel.ToString().ToUpper());
	}

	if (AetherGameMode->HasCompletedPrototypeBossEncounter())
	{
		return TEXT("ROUTE SUMMARY: THE RIFT CAN BE SEALED");
	}

	if (AetherGameMode->HasCompletedPrototypeEncounter(AetherPrototypeLabels::LowerCryptElite()) ||
		AetherGameMode->HasCollectedPrototypeReward(AetherPrototypeLabels::CryptEliteReward()) ||
		AetherGameMode->HasUnlockedPrototypeProgressGate(AetherPrototypeLabels::GateLowerCryptElite()))
	{
		return TEXT("ROUTE SUMMARY: CATHEDRAL PATH OPEN");
	}

	if (!AetherGameMode->HasCollectedPrototypeLore(AetherPrototypeLabels::LoreCrypt001()) &&
		AetherGameMode->GetActivePrototypeCheckpointProgressRank() >= 30)
	{
		return TEXT("ROUTE SUMMARY: CRYPT RECORD AND WARDEN SEAL");
	}

	if (AetherGameMode->HasCompletedPrototypeEncounter(AetherPrototypeLabels::HamletEntry()))
	{
		return TEXT("ROUTE SUMMARY: BROKEN WALL LEADS TO THE CRYPT");
	}

	if (AetherGameMode->HasUnlockedPrototypeKeyGate(AetherPrototypeLabels::HamletDoor()))
	{
		return TEXT("ROUTE SUMMARY: HAMLET GATE OPEN");
	}

	if (AetherGameMode->HasCollectedPrototypeKey(AetherPrototypeLabels::HamletKey()))
	{
		return TEXT("ROUTE SUMMARY: HAMLET KEY RECOVERED");
	}

	if (AetherGameMode->HasCompletedPrototypeEncounter(AetherPrototypeLabels::ForestIntro()))
	{
		if (!AetherGameMode->HasCollectedPrototypeLore(AetherPrototypeLabels::LoreHamlet001()))
		{
			return TEXT("ROUTE SUMMARY: FALLEN HAMLET HOLDS A KEY AND RECORD");
		}

		return TEXT("ROUTE SUMMARY: HAMLET DOOR NEEDS A KEY");
	}

	return TEXT("ROUTE SUMMARY: FOREST PATH TOWARD ELDRAN");
}

/** 플레이어 사망과 진행 구간을 기준으로 조작 및 재시도 안내를 선택한다. */
FString FAetherPrototypeRouteGuide::BuildTutorialHintLabel(
	const AAetherGameModeBase* AetherGameMode,
	const AAetherfallCharacter* PlayerCharacter)
{
	if (!AetherGameMode)
	{
		return TEXT("");
	}

	const UAetherCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	const UAetherHealthComponent* HealthComponent = PlayerCharacter ? PlayerCharacter->GetHealthComponent() : nullptr;
	if (HealthComponent && HealthComponent->IsDead())
	{
		return AetherGameMode->IsPrototypeDefeatRetryScheduled()
			? TEXT("AUTO RETRY ACTIVE / WAIT FOR CHECKPOINT RESTORE")
			: TEXT("PRESS Y TO RESET WHEN NO CHECKPOINT EXISTS");
	}

	if (AetherGameMode->IsPrototypeLevelComplete())
	{
		return TEXT("BACKSPACE CLEARS QA PROGRESS FOR A FRESH ROUTE RUN");
	}

	const bool bAetherSlashReady = CombatComponent && CombatComponent->IsAetherSlashReady();
	const bool bCombatActive = !AetherGameMode->GetActivePrototypeEncounterLabel().IsNone() && !AetherGameMode->IsPrototypeCombatRoundComplete();
	if (AetherGameMode->IsPrototypeBossEncounterActive())
	{
		return bAetherSlashReady
			? TEXT("TAB LOCK-ON / RMB GUARD / Q PARRY / X AETHER SLASH")
			: TEXT("TAB LOCK-ON / RMB GUARD / Q PARRY / F EXECUTION OPENINGS");
	}

	if (bCombatActive)
	{
		return bAetherSlashReady
			? TEXT("LMB COMBO / RMB GUARD / Q PARRY / X AETHER SLASH / H HEAL")
			: TEXT("LMB COMBO / TAB LOCK-ON / RMB GUARD / Q PARRY / H HEAL");
	}

	if (!AetherGameMode->HasCompletedPrototypeEncounter(AetherPrototypeLabels::ForestIntro()))
	{
		return TEXT("WASD MOVE / MOUSE LOOK / TAB LOCK-ON WHEN ENEMY APPEARS");
	}

	if (AetherGameMode->HasCompletedPrototypeBossEncounter())
	{
		return TEXT("FOLLOW THE CATHEDRAL ENDING MARKER / WATCH FOR THE FINAL PROMPT");
	}

	return TEXT("[V] INTERACT / H HEAL / BACKSPACE CLEAR QA PROGRESS");
}

/** 보스전, 보스 처치 이후 또는 레벨 종료를 강조 구간으로 분류한다. */
bool FAetherPrototypeRouteGuide::IsCriticalObjective(const AAetherGameModeBase* AetherGameMode)
{
	return AetherGameMode &&
		(AetherGameMode->IsPrototypeBossEncounterActive() ||
			AetherGameMode->HasCompletedPrototypeBossEncounter() ||
			AetherGameMode->IsPrototypeLevelComplete());
}
