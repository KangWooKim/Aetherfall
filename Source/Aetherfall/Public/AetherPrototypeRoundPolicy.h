#pragma once

#include "CoreMinimal.h"

/** 적 사망 직후 처치 수·목표·생존 적 수를 평가하는 입력이다. */
struct FAetherPrototypeRoundDeathInput
{
	bool bUseRoundGoal = false;
	bool bRoundComplete = false;
	int32 CurrentDefeatCount = 0;
	int32 KillGoal = 1;
	int32 LivingEnemiesAfterDeath = 0;
	bool bGoalMetCuePlayed = false;
};

/** 목표 도달과 전투 완료 안내에 사용할 피드백을 묶는다. */
struct FAetherPrototypeRoundFeedback
{
	FString Message;
	FColor Color = FColor::White;
};

/** 처치 수 변경, 목표 알림, 전투 완료와 재생성 요청을 분리해 반환한다. */
struct FAetherPrototypeRoundDeathResult
{
	int32 NewDefeatCount = 0;
	bool bBroadcastProgress = false;
	bool bClearRespawnTimer = false;
	bool bCompleteRound = false;
	bool bScheduleRespawn = false;
	bool bBroadcastGoalMet = false;
	bool bPlayGoalMetCue = false;
	bool bGoalMetCuePlayed = false;
	TArray<FAetherPrototypeRoundFeedback> FeedbackMessages;
};

/** 적 사망 이후 처치 목표와 생존 적 수를 평가해 전투 완료·목표 알림·재생성 계획을 반환한다. */
class AETHERFALL_API FAetherPrototypeRoundPolicy
{
public:
	/** 목표 수를 채워도 남은 적이 있으면 목표 달성만 알리고 마지막 적이 사망했을 때 전투를 완료한다. */
	static FAetherPrototypeRoundDeathResult EvaluateEnemyDeath(const FAetherPrototypeRoundDeathInput& Input);
};
