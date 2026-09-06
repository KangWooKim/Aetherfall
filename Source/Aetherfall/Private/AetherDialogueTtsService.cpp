/** 대화 진행에서 사용할 음성 서비스 경계를 정의하고, 실제 음성 대신 경과 시간으로 완료를 알리는 모의 구현을 제공한다. */
#include "AetherDialogueTtsService.h"

/** 서비스 구현에 현재 대사와 대체 재생 시간을 전달한다. 모의 구현은 시간을 세며 실제 음성을 생성하지 않는다. */
void UAetherDialogueTtsService::StartLine(const FAetherDialogueLine& DialogueLine, float FallbackDurationSeconds)
{
}

void UAetherDialogueTtsService::TickService(float DeltaTime)
{
}

/** 현재 재생을 중단한다. 모의 구현은 남은 시간을 비워 즉시 완료 상태로 전환한다. */
void UAetherDialogueTtsService::SkipLine()
{
}

bool UAetherDialogueTtsService::IsLinePlaying() const
{
	return false;
}

bool UAetherDialogueTtsService::HasLineFinished() const
{
	return true;
}

/** 서비스 구현에 현재 대사와 대체 재생 시간을 전달한다. 모의 구현은 시간을 세며 실제 음성을 생성하지 않는다. */
void UAetherDialogueMockTtsService::StartLine(const FAetherDialogueLine& DialogueLine, float FallbackDurationSeconds)
{
	RemainingDurationSeconds = FMath::Max(0.0f, FallbackDurationSeconds);
	bPlaying = RemainingDurationSeconds > 0.0f;
}

void UAetherDialogueMockTtsService::TickService(float DeltaTime)
{
	if (!bPlaying)
	{
		return;
	}

	RemainingDurationSeconds = FMath::Max(0.0f, RemainingDurationSeconds - FMath::Max(0.0f, DeltaTime));
	if (RemainingDurationSeconds <= 0.0f)
	{
		bPlaying = false;
	}
}

/** 현재 재생을 중단한다. 모의 구현은 남은 시간을 비워 즉시 완료 상태로 전환한다. */
void UAetherDialogueMockTtsService::SkipLine()
{
	RemainingDurationSeconds = 0.0f;
	bPlaying = false;
}

bool UAetherDialogueMockTtsService::IsLinePlaying() const
{
	return bPlaying;
}

bool UAetherDialogueMockTtsService::HasLineFinished() const
{
	return !bPlaying;
}
