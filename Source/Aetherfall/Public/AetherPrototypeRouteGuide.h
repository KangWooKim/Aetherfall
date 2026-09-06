#pragma once

#include "CoreMinimal.h"

class AAetherGameModeBase;
class AAetherfallCharacter;

/** 현재 목표·조작 안내·중요 구간 여부를 HUD에 전달한다. */
struct FAetherPrototypeRouteGuidanceViewData
{
	FString ObjectiveLabel;
	FString TutorialHintLabel;
	bool bCriticalObjective = false;

	bool HasGuidance() const
	{
		return !ObjectiveLabel.IsEmpty() || !TutorialHintLabel.IsEmpty();
	}
};

/** 게임 모드 진행 정보에서 HUD용 목표·조작 안내·중요 구간 표시를 만드는 경로 안내 정책이다. */
class AETHERFALL_API FAetherPrototypeRouteGuide
{
public:
	static FAetherPrototypeRouteGuidanceViewData BuildGuidance(
		const AAetherGameModeBase* AetherGameMode,
		const AAetherfallCharacter* PlayerCharacter);

	/** 종료·보스전·활성 전투 등 우선순위에 따라 현재 목표 문구를 선택한다. */
	static FString BuildObjectiveLabel(const AAetherGameModeBase* AetherGameMode);
	/** 플레이어 사망과 진행 구간을 기준으로 조작 및 재시도 안내를 선택한다. */
	static FString BuildTutorialHintLabel(
		const AAetherGameModeBase* AetherGameMode,
		const AAetherfallCharacter* PlayerCharacter);
	/** 보스전, 보스 처치 이후 또는 레벨 종료를 강조 구간으로 분류한다. */
	static bool IsCriticalObjective(const AAetherGameModeBase* AetherGameMode);
};
