#pragma once

#include "CoreMinimal.h"

class UAetherPrototypeSaveGame;

struct FAetherPrototypeCheckpointSnapshotState
{
	int32 SaveSchemaVersion = 0;
	FName SaveSchemaLabel = NAME_None;
	bool bLoadedFromLegacySaveSchema = false;

	bool bHasActiveCheckpoint = false;
	FTransform ActiveCheckpointTransform = FTransform::Identity;
	FName ActiveCheckpointLabel = NAME_None;
	int32 ActiveCheckpointProgressRank = 0;

	float PlayerCurrentHealth = 100.0f;
	int32 PrototypeHealingItemCount = 0;

	FName ActivePrototypeEncounterLabel = NAME_None;
	FName LastCompletedPrototypeEncounterLabel = NAME_None;
	TSet<FName> CompletedPrototypeEncounterLabels;

	bool bPrototypeCombatRoundComplete = false;
	int32 PrototypeRoundDefeatCount = 0;
	int32 PrototypeRoundKillGoal = 1;
	bool bPrototypeRoundGoalMetCuePlayed = false;

	bool bPrototypeLevelComplete = false;
	FName CompletedPrototypeLevelGoalLabel = NAME_None;
	bool bCathedralEndingComplete = false;

	TSet<FName> CollectedPrototypeKeyLabels;
	TSet<FName> CollectedPrototypeRewardLabels;
	TSet<FName> CollectedPrototypeLoreLabels;
	TSet<FName> ActivatedPrototypeLeverLabels;
	TSet<FName> UnlockedPrototypeProgressGateLabels;
	TSet<FName> UnlockedPrototypeKeyGateLabels;
	TSet<FName> OpenedPrototypeChestLabels;
	TSet<FName> PlayedPrototypeDialogueLabels;
};

class AETHERFALL_API FAetherPrototypeCheckpointSnapshot
{
public:
	static int32 ResolveProgressRank(FName CheckpointLabel, int32 ExplicitCheckpointProgressRank);

	static void WriteSaveGame(UAetherPrototypeSaveGame& SaveGameObject, const FAetherPrototypeCheckpointSnapshotState& SnapshotState);
	static FAetherPrototypeCheckpointSnapshotState ReadSaveGame(const UAetherPrototypeSaveGame& SaveGameObject, FName CathedralEndingGoalLabel);

	static FString BuildQASummary(const FString& Context, const FAetherPrototypeCheckpointSnapshotState& SnapshotState);
	static FString FormatNameSetForQA(const TSet<FName>& Labels);
};
