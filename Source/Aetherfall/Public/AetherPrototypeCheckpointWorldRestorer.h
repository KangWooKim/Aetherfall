#pragma once

#include "CoreMinimal.h"
#include "AetherPrototypeCheckpointSnapshot.h"

class UObject;

class AETHERFALL_API FAetherPrototypeCheckpointWorldRestorer
{
public:
	static void RestoreWorldState(const UObject* WorldContextObject, const FAetherPrototypeCheckpointSnapshotState& SnapshotState);
};
