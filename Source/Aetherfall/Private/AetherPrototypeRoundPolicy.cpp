/** 적 사망 이후 처치 목표와 생존 적 수를 평가해 전투 완료·목표 알림·재생성 계획을 반환한다. */
#include "AetherPrototypeRoundPolicy.h"

namespace
{
void AddFeedback(FAetherPrototypeRoundDeathResult& Result, const FString& Message, const FColor& Color)
{
	FAetherPrototypeRoundFeedback Feedback;
	Feedback.Message = Message;
	Feedback.Color = Color;
	Result.FeedbackMessages.Add(Feedback);
}
}

/** 목표 수를 채워도 남은 적이 있으면 목표 달성만 알리고 마지막 적이 사망했을 때 전투를 완료한다. */
FAetherPrototypeRoundDeathResult FAetherPrototypeRoundPolicy::EvaluateEnemyDeath(const FAetherPrototypeRoundDeathInput& Input)
{
	FAetherPrototypeRoundDeathResult Result;
	const int32 SafeKillGoal = FMath::Max(1, Input.KillGoal);
	Result.NewDefeatCount = FMath::Max(0, Input.CurrentDefeatCount);
	Result.bGoalMetCuePlayed = Input.bGoalMetCuePlayed;

	if (Input.bUseRoundGoal && !Input.bRoundComplete)
	{
		const bool bRoundGoalAlreadyMet = Result.NewDefeatCount >= SafeKillGoal;
		if (!bRoundGoalAlreadyMet)
		{
			Result.NewDefeatCount = FMath::Min(Result.NewDefeatCount + 1, SafeKillGoal);
			Result.bBroadcastProgress = true;
			AddFeedback(
				Result,
				FString::Printf(TEXT("Round defeated enemies %d / %d"), Result.NewDefeatCount, SafeKillGoal),
				FColor::Cyan);
		}

		if (Result.NewDefeatCount >= SafeKillGoal)
		{
			Result.bClearRespawnTimer = true;

			if (bRoundGoalAlreadyMet)
			{
				AddFeedback(
					Result,
					FString::Printf(TEXT("Round cleanup enemy defeated / remaining %d"), Input.LivingEnemiesAfterDeath),
					FColor::Silver);
			}

			if (Input.LivingEnemiesAfterDeath <= 0)
			{
				Result.bCompleteRound = true;
				return Result;
			}

			if (!Input.bGoalMetCuePlayed)
			{
				AddFeedback(Result, TEXT("Round goal reached / clear remaining enemies"), FColor::Yellow);
				Result.bBroadcastGoalMet = true;
				Result.bPlayGoalMetCue = true;
				Result.bGoalMetCuePlayed = true;
			}
			else
			{
				AddFeedback(Result, TEXT("Round cleanup continuing"), FColor::Silver);
			}

			return Result;
		}
	}

	Result.bScheduleRespawn = true;
	return Result;
}
