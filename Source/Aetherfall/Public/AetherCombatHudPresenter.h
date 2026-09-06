#pragma once

#include "CoreMinimal.h"

class AAetherGameModeBase;
class UAetherCombatComponent;
class UAetherHealthComponent;

/** 체력 위험 상태를 HUD 문구와 색상으로 표현하는 결과다. */
struct FAetherPlayerDangerViewData
{
	bool bShouldDisplay = false;
	FString Label;
	FLinearColor Color = FLinearColor::White;
};

/** 전투·체력·게임 진행 상태를 화면에 표시할 문구와 색상 데이터로 변환한다. */
class AETHERFALL_API FAetherCombatHudPresenter
{
public:
	/** 사망과 피격 회복을 우선 표시한 뒤 현재 전투 행동을 상태 문구로 변환한다. */
	static FString BuildCombatStateLabel(
		const UAetherCombatComponent* CombatComponent,
		const UAetherHealthComponent* HealthComponent,
		const AAetherGameModeBase* AetherGameMode);

	/** 남은 재사용 시간, 부족한 게이지, 사용 가능 여부 순으로 검기 상태를 표시한다. */
	static FString BuildAetherSlashStatusLabel(const UAetherCombatComponent* CombatComponent);

	/** 전투 완료 시 경고를 숨기고 사망·위급·저체력 상태에 맞는 문구와 색상을 반환한다. */
	static FAetherPlayerDangerViewData BuildPlayerDangerViewData(
		const UAetherHealthComponent* HealthComponent,
		const AAetherGameModeBase* AetherGameMode);
};
