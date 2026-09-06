#pragma once

#include "CoreMinimal.h"
#include "AetherEnemyBase.h"

/** 플레이어 기준 축과 슬롯 번호·적 수·간격으로 생성 배치를 계산하는 입력이다. */
struct FAetherPrototypeEnemySpawnRequest
{
	FVector PlayerLocation = FVector::ZeroVector;
	FVector PlayerForward = FVector::ForwardVector;
	FVector PlayerRight = FVector::RightVector;
	int32 SlotIndex = 0;
	int32 TargetSpawnCount = 1;
	float SpawnDistance = 650.0f;
	float SpawnRightOffset = 0.0f;
	float SpawnSpacing = 420.0f;
};

/** 해당 슬롯에 적용할 적 원형과 생성 변환을 반환한다. */
struct FAetherPrototypeEnemySpawnPlan
{
	FTransform SpawnTransform = FTransform::Identity;
	EAetherEnemyArchetype EnemyArchetype = EAetherEnemyArchetype::Vanguard;
};

/** 플레이어 기준의 생성 배치와 슬롯별 적 원형을 계산하며 실제 액터 생성은 게임 모드에 맡긴다. */
class AETHERFALL_API FAetherPrototypeEnemySpawnPolicy
{
public:
	static FAetherPrototypeEnemySpawnPlan BuildSpawnPlan(
		const FAetherPrototypeEnemySpawnRequest& SpawnRequest,
		const TArray<EAetherEnemyArchetype>& ArchetypeSequence);

	/** 생성 슬롯을 중앙 기준으로 배치해 전방 거리와 좌우 간격을 적용하고 플레이어 쪽을 향하게 한다. */
	static FTransform BuildSpawnTransform(const FAetherPrototypeEnemySpawnRequest& SpawnRequest);
	/** 원형 목록을 슬롯 인덱스로 순환 선택하고 목록이 없으면 기본 선봉형 적을 사용한다. */
	static EAetherEnemyArchetype ResolveArchetypeForSlot(int32 SlotIndex, const TArray<EAetherEnemyArchetype>& ArchetypeSequence);
};
