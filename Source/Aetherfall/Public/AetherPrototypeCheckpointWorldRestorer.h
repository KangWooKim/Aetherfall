#pragma once

#include "CoreMinimal.h"
#include "AetherPrototypeCheckpointSnapshot.h"

class UObject;

/** 월드 액터를 한 번 수집해 종류별로 분류하고 저장 라벨에 맞춰 아홉 종류의 상호작용·진행 액터 상태를 복원한다. */
class AETHERFALL_API FAetherPrototypeCheckpointWorldRestorer
{
public:
	/** 수집한 액터의 안정적인 기능 라벨을 스냅샷 집합과 비교해 각 액터의 복원 API를 호출한다. */
	static void RestoreWorldState(const UObject* WorldContextObject, const FAetherPrototypeCheckpointSnapshotState& SnapshotState);
};
