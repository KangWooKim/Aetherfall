#pragma once

#include "CoreMinimal.h"
#include "AetherEnemyBase.h"

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

struct FAetherPrototypeEnemySpawnPlan
{
	FTransform SpawnTransform = FTransform::Identity;
	EAetherEnemyArchetype EnemyArchetype = EAetherEnemyArchetype::Vanguard;
};

class AETHERFALL_API FAetherPrototypeEnemySpawnPolicy
{
public:
	static FAetherPrototypeEnemySpawnPlan BuildSpawnPlan(
		const FAetherPrototypeEnemySpawnRequest& SpawnRequest,
		const TArray<EAetherEnemyArchetype>& ArchetypeSequence);

	static FTransform BuildSpawnTransform(const FAetherPrototypeEnemySpawnRequest& SpawnRequest);
	static EAetherEnemyArchetype ResolveArchetypeForSlot(int32 SlotIndex, const TArray<EAetherEnemyArchetype>& ArchetypeSequence);
};
