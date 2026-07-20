#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AetherPrototypeSaveGame.generated.h"

UCLASS()
class AETHERFALL_API UAetherPrototypeSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 SaveSchemaVersion = 0;

	UPROPERTY()
	FName SaveSchemaLabel = NAME_None;

	UPROPERTY()
	FDateTime SavedAtUtc;

	UPROPERTY()
	FName SavedMapAsset = NAME_None;

	UPROPERTY()
	bool bHasActiveCheckpoint = false;

	UPROPERTY()
	FTransform ActiveCheckpointTransform = FTransform::Identity;

	UPROPERTY()
	FName ActiveCheckpointLabel = NAME_None;

	UPROPERTY()
	int32 ActiveCheckpointProgressRank = 0;

	UPROPERTY()
	float PlayerCurrentHealth = 100.0f;

	UPROPERTY()
	int32 PrototypeHealingItemCount = 0;

	UPROPERTY()
	FName ActivePrototypeEncounterLabel = NAME_None;

	UPROPERTY()
	FName LastCompletedPrototypeEncounterLabel = NAME_None;

	UPROPERTY()
	TArray<FName> CompletedPrototypeEncounterLabels;

	UPROPERTY()
	bool bPrototypeCombatRoundComplete = false;

	UPROPERTY()
	int32 PrototypeRoundDefeatCount = 0;

	UPROPERTY()
	int32 PrototypeRoundKillGoal = 1;

	UPROPERTY()
	bool bPrototypeLevelComplete = false;

	UPROPERTY()
	FName CompletedPrototypeLevelGoalLabel = NAME_None;

	UPROPERTY()
	bool bCathedralEndingComplete = false;

	UPROPERTY()
	TArray<FName> CollectedPrototypeKeyLabels;

	UPROPERTY()
	TArray<FName> CollectedPrototypeRewardLabels;

	UPROPERTY()
	TArray<FName> CollectedPrototypeLoreLabels;

	UPROPERTY()
	TArray<FName> ActivatedPrototypeLeverLabels;

	UPROPERTY()
	TArray<FName> UnlockedPrototypeProgressGateLabels;

	UPROPERTY()
	TArray<FName> UnlockedPrototypeKeyGateLabels;

	UPROPERTY()
	TArray<FName> OpenedPrototypeChestLabels;

	UPROPERTY()
	TArray<FName> PlayedPrototypeDialogueLabels;
};
