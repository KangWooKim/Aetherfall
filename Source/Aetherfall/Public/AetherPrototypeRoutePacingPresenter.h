#pragma once

#include "CoreMinimal.h"

class AETHERFALL_API FAetherPrototypeRoutePacingPresenter
{
public:
	static FString FormatTime(float Seconds);
	static FString BuildResultLabel(float ElapsedSeconds, float TargetMinSeconds, float TargetMaxSeconds);
	static FString BuildStatusLabel(float ElapsedSeconds, float TargetMinSeconds, float TargetMaxSeconds, bool bCompleted);
};
