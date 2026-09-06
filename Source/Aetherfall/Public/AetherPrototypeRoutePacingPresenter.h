#pragma once

#include "CoreMinimal.h"

/** 경과 시간을 목표 범위와 비교해 HUD용 시간 및 진행 속도 문구를 구성한다. */
class AETHERFALL_API FAetherPrototypeRoutePacingPresenter
{
public:
	/** 음수 시간을 보정하고 반올림한 초를 분과 초로 표시한다. */
	static FString FormatTime(float Seconds);
	/** 목표 최소·최대 시간의 바깥만 빠름 또는 느림으로 표시하고 경계값은 목표 범위에 포함한다. */
	static FString BuildResultLabel(float ElapsedSeconds, float TargetMinSeconds, float TargetMaxSeconds);
	/** 현재 경과 시간과 목표 범위를 표시하며 종료된 경우에만 최종 비교 결과를 덧붙인다. */
	static FString BuildStatusLabel(float ElapsedSeconds, float TargetMinSeconds, float TargetMaxSeconds, bool bCompleted);
};
