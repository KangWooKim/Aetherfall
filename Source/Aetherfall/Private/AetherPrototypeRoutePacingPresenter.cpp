#include "AetherPrototypeRoutePacingPresenter.h"

FString FAetherPrototypeRoutePacingPresenter::FormatTime(float Seconds)
{
	const int32 TotalSeconds = FMath::Max(0, FMath::RoundToInt(Seconds));
	const int32 Minutes = TotalSeconds / 60;
	const int32 RemainingSeconds = TotalSeconds % 60;
	return FString::Printf(TEXT("%02d:%02d"), Minutes, RemainingSeconds);
}

FString FAetherPrototypeRoutePacingPresenter::BuildResultLabel(float ElapsedSeconds, float TargetMinSeconds, float TargetMaxSeconds)
{
	if (ElapsedSeconds < TargetMinSeconds)
	{
		return TEXT("FAST");
	}

	if (ElapsedSeconds > TargetMaxSeconds)
	{
		return TEXT("SLOW");
	}

	return TEXT("ON TARGET");
}

FString FAetherPrototypeRoutePacingPresenter::BuildStatusLabel(float ElapsedSeconds, float TargetMinSeconds, float TargetMaxSeconds, bool bCompleted)
{
	const FString ElapsedLabel = FormatTime(ElapsedSeconds);
	const FString MinLabel = FormatTime(TargetMinSeconds);
	const FString MaxLabel = FormatTime(TargetMaxSeconds);

	if (bCompleted)
	{
		return FString::Printf(
			TEXT("ROUTE TIME %s / TARGET %s-%s %s"),
			*ElapsedLabel,
			*MinLabel,
			*MaxLabel,
			*BuildResultLabel(ElapsedSeconds, TargetMinSeconds, TargetMaxSeconds));
	}

	return FString::Printf(
		TEXT("ROUTE TIME %s / TARGET %s-%s"),
		*ElapsedLabel,
		*MinLabel,
		*MaxLabel);
}
