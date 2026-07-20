#include "AetherPrototypeEnemySpawnPolicy.h"

FAetherPrototypeEnemySpawnPlan FAetherPrototypeEnemySpawnPolicy::BuildSpawnPlan(
	const FAetherPrototypeEnemySpawnRequest& SpawnRequest,
	const TArray<EAetherEnemyArchetype>& ArchetypeSequence)
{
	FAetherPrototypeEnemySpawnPlan SpawnPlan;
	SpawnPlan.SpawnTransform = BuildSpawnTransform(SpawnRequest);
	SpawnPlan.EnemyArchetype = ResolveArchetypeForSlot(SpawnRequest.SlotIndex, ArchetypeSequence);
	return SpawnPlan;
}

FTransform FAetherPrototypeEnemySpawnPolicy::BuildSpawnTransform(const FAetherPrototypeEnemySpawnRequest& SpawnRequest)
{
	const int32 TargetSpawnCount = FMath::Max(1, SpawnRequest.TargetSpawnCount);
	const float CenteredSlot = SpawnRequest.SlotIndex - (TargetSpawnCount - 1) * 0.5f;
	const float RightOffset = SpawnRequest.SpawnRightOffset + CenteredSlot * SpawnRequest.SpawnSpacing;
	const FVector SpawnLocation =
		SpawnRequest.PlayerLocation +
		SpawnRequest.PlayerForward * SpawnRequest.SpawnDistance +
		SpawnRequest.PlayerRight * RightOffset;
	const FRotator SpawnRotation = (-SpawnRequest.PlayerForward).Rotation();

	return FTransform(SpawnRotation, SpawnLocation);
}

EAetherEnemyArchetype FAetherPrototypeEnemySpawnPolicy::ResolveArchetypeForSlot(
	int32 SlotIndex,
	const TArray<EAetherEnemyArchetype>& ArchetypeSequence)
{
	if (ArchetypeSequence.Num() <= 0)
	{
		return EAetherEnemyArchetype::Vanguard;
	}

	const int32 WrappedIndex = FMath::Abs(SlotIndex) % ArchetypeSequence.Num();
	return ArchetypeSequence[WrappedIndex];
}
