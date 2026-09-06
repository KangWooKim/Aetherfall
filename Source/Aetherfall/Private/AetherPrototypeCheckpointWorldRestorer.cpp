/** 월드 액터를 한 번 수집해 종류별로 분류하고 저장 라벨에 맞춰 아홉 종류의 상호작용·진행 액터 상태를 복원한다. */
#include "AetherPrototypeCheckpointWorldRestorer.h"

#include "AetherPrototypeChest.h"
#include "AetherPrototypeEncounterTrigger.h"
#include "AetherPrototypeKeyGate.h"
#include "AetherPrototypeKeyPickup.h"
#include "AetherPrototypeLevelGoal.h"
#include "AetherPrototypeLever.h"
#include "AetherPrototypeLorePickup.h"
#include "AetherPrototypeProgressGate.h"
#include "AetherPrototypeRewardPickup.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
	struct FAetherPrototypeRestoreActors
	{
		TArray<AAetherPrototypeKeyPickup*> KeyPickups;
		TArray<AAetherPrototypeRewardPickup*> RewardPickups;
		TArray<AAetherPrototypeLorePickup*> LorePickups;
		TArray<AAetherPrototypeLever*> Levers;
		TArray<AAetherPrototypeProgressGate*> ProgressGates;
		TArray<AAetherPrototypeKeyGate*> KeyGates;
		TArray<AAetherPrototypeChest*> Chests;
		TArray<AAetherPrototypeEncounterTrigger*> EncounterTriggers;
		TArray<AAetherPrototypeLevelGoal*> LevelGoals;

		void ReserveDefaults()
		{
			KeyPickups.Reserve(8);
			RewardPickups.Reserve(8);
			LorePickups.Reserve(8);
			Levers.Reserve(8);
			ProgressGates.Reserve(4);
			KeyGates.Reserve(4);
			Chests.Reserve(4);
			EncounterTriggers.Reserve(8);
			LevelGoals.Reserve(2);
		}
	};

	/** 한 번의 월드 순회로 복원 대상 목록을 만들고 종류별 배열에 보관한다. */
	static FAetherPrototypeRestoreActors CollectPrototypeRestoreActors(const UObject* WorldContextObject)
	{
		FAetherPrototypeRestoreActors RestoreActors;
		RestoreActors.ReserveDefaults();

		UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
		if (!World)
		{
			return RestoreActors;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!IsValid(Actor))
			{
				continue;
			}

			if (AAetherPrototypeKeyPickup* KeyPickup = Cast<AAetherPrototypeKeyPickup>(Actor))
			{
				RestoreActors.KeyPickups.Add(KeyPickup);
			}
			else if (AAetherPrototypeRewardPickup* RewardPickup = Cast<AAetherPrototypeRewardPickup>(Actor))
			{
				RestoreActors.RewardPickups.Add(RewardPickup);
			}
			else if (AAetherPrototypeLorePickup* LorePickup = Cast<AAetherPrototypeLorePickup>(Actor))
			{
				RestoreActors.LorePickups.Add(LorePickup);
			}
			else if (AAetherPrototypeLever* Lever = Cast<AAetherPrototypeLever>(Actor))
			{
				RestoreActors.Levers.Add(Lever);
			}
			else if (AAetherPrototypeProgressGate* ProgressGate = Cast<AAetherPrototypeProgressGate>(Actor))
			{
				RestoreActors.ProgressGates.Add(ProgressGate);
			}
			else if (AAetherPrototypeKeyGate* KeyGate = Cast<AAetherPrototypeKeyGate>(Actor))
			{
				RestoreActors.KeyGates.Add(KeyGate);
			}
			else if (AAetherPrototypeChest* Chest = Cast<AAetherPrototypeChest>(Actor))
			{
				RestoreActors.Chests.Add(Chest);
			}
			else if (AAetherPrototypeEncounterTrigger* EncounterTrigger = Cast<AAetherPrototypeEncounterTrigger>(Actor))
			{
				RestoreActors.EncounterTriggers.Add(EncounterTrigger);
			}
			else if (AAetherPrototypeLevelGoal* LevelGoal = Cast<AAetherPrototypeLevelGoal>(Actor))
			{
				RestoreActors.LevelGoals.Add(LevelGoal);
			}
		}

		return RestoreActors;
	}

	template <typename ActorType, typename ShouldRestoreStateType>
	static void RestorePrototypeActors(const TArray<ActorType*>& Actors, ShouldRestoreStateType&& ShouldRestoreState)
	{
		for (ActorType* TypedActor : Actors)
		{
			if (IsValid(TypedActor))
			{
				TypedActor->RestorePrototypeCheckpointState(ShouldRestoreState(*TypedActor));
			}
		}
	}
}

/** 수집한 액터의 안정적인 기능 라벨을 스냅샷 집합과 비교해 각 액터의 복원 API를 호출한다. */
void FAetherPrototypeCheckpointWorldRestorer::RestoreWorldState(
	const UObject* WorldContextObject,
	const FAetherPrototypeCheckpointSnapshotState& SnapshotState)
{
	const FAetherPrototypeRestoreActors RestoreActors = CollectPrototypeRestoreActors(WorldContextObject);

	RestorePrototypeActors(
		RestoreActors.KeyPickups,
		[&SnapshotState](const AAetherPrototypeKeyPickup& KeyPickup)
		{
			return SnapshotState.CollectedPrototypeKeyLabels.Contains(KeyPickup.GetKeyLabel());
		});

	RestorePrototypeActors(
		RestoreActors.RewardPickups,
		[&SnapshotState](const AAetherPrototypeRewardPickup& RewardPickup)
		{
			return SnapshotState.CollectedPrototypeRewardLabels.Contains(RewardPickup.GetRewardLabel());
		});

	RestorePrototypeActors(
		RestoreActors.LorePickups,
		[&SnapshotState](const AAetherPrototypeLorePickup& LorePickup)
		{
			return SnapshotState.CollectedPrototypeLoreLabels.Contains(LorePickup.GetLoreLabel());
		});

	RestorePrototypeActors(
		RestoreActors.Levers,
		[&SnapshotState](const AAetherPrototypeLever& Lever)
		{
			return SnapshotState.ActivatedPrototypeLeverLabels.Contains(Lever.GetLeverLabel());
		});

	RestorePrototypeActors(
		RestoreActors.ProgressGates,
		[&SnapshotState](const AAetherPrototypeProgressGate& ProgressGate)
		{
			return SnapshotState.UnlockedPrototypeProgressGateLabels.Contains(ProgressGate.GetGateLabel());
		});

	RestorePrototypeActors(
		RestoreActors.KeyGates,
		[&SnapshotState](const AAetherPrototypeKeyGate& KeyGate)
		{
			return SnapshotState.UnlockedPrototypeKeyGateLabels.Contains(KeyGate.GetGateLabel());
		});

	RestorePrototypeActors(
		RestoreActors.Chests,
		[&SnapshotState](const AAetherPrototypeChest& Chest)
		{
			return SnapshotState.OpenedPrototypeChestLabels.Contains(Chest.GetChestLabel());
		});

	RestorePrototypeActors(
		RestoreActors.EncounterTriggers,
		[&SnapshotState](const AAetherPrototypeEncounterTrigger& EncounterTrigger)
		{
			const FName EncounterLabel = EncounterTrigger.GetEncounterLabel();
			return !EncounterLabel.IsNone() &&
				(EncounterLabel == SnapshotState.ActivePrototypeEncounterLabel ||
				SnapshotState.CompletedPrototypeEncounterLabels.Contains(EncounterLabel));
		});

	RestorePrototypeActors(
		RestoreActors.LevelGoals,
		[&SnapshotState](const AAetherPrototypeLevelGoal& LevelGoal)
		{
			const FName GoalLabel = LevelGoal.GetGoalLabel();
			return SnapshotState.bPrototypeLevelComplete &&
				!GoalLabel.IsNone() &&
				GoalLabel == SnapshotState.CompletedPrototypeLevelGoalLabel;
		});
}
