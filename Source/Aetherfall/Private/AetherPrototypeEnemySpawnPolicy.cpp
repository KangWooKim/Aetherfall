/** 플레이어 기준의 생성 배치와 슬롯별 적 원형을 계산하며 실제 액터 생성은 게임 모드에 맡긴다. */
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

/** 생성 슬롯을 중앙 기준으로 배치해 전방 거리와 좌우 간격을 적용하고 플레이어 쪽을 향하게 한다. */
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

/** 원형 목록을 슬롯 인덱스로 순환 선택하고 목록이 없으면 기본 선봉형 적을 사용한다. */
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
