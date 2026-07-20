#include "AetherDialogueTtsService.h"

void UAetherDialogueTtsService::StartLine(const FAetherDialogueLine& DialogueLine, float FallbackDurationSeconds)
{
}

void UAetherDialogueTtsService::TickService(float DeltaTime)
{
}

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
