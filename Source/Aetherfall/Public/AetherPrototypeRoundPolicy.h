#pragma once

#include "CoreMinimal.h"

struct FAetherPrototypeRoundDeathInput
{
	bool bUseRoundGoal = false;
	bool bRoundComplete = false;
	int32 CurrentDefeatCount = 0;
	int32 KillGoal = 1;
	int32 LivingEnemiesAfterDeath = 0;
	bool bGoalMetCuePlayed = false;
};

struct FAetherPrototypeRoundFeedback
{
	FString Message;
	FColor Color = FColor::White;
};

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

class AETHERFALL_API FAetherPrototypeRoundPolicy
{
public:
	static FAetherPrototypeRoundDeathResult EvaluateEnemyDeath(const FAetherPrototypeRoundDeathInput& Input);
};
