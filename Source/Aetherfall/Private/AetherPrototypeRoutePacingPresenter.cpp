/** 경과 시간을 목표 범위와 비교해 HUD용 시간 및 진행 속도 문구를 구성한다. */
#include "AetherPrototypeRoutePacingPresenter.h"

/** 음수 시간을 보정하고 반올림한 초를 분과 초로 표시한다. */
FString FAetherPrototypeRoutePacingPresenter::FormatTime(float Seconds)
{
	const int32 TotalSeconds = FMath::Max(0, FMath::RoundToInt(Seconds));
	const int32 Minutes = TotalSeconds / 60;
	const int32 RemainingSeconds = TotalSeconds % 60;
	return FString::Printf(TEXT("%02d:%02d"), Minutes, RemainingSeconds);
}

/** 목표 최소·최대 시간의 바깥만 빠름 또는 느림으로 표시하고 경계값은 목표 범위에 포함한다. */
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

/** 현재 경과 시간과 목표 범위를 표시하며 종료된 경우에만 최종 비교 결과를 덧붙인다. */
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
