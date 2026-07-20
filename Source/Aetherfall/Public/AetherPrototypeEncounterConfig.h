#pragma once

#include "CoreMinimal.h"
#include "AetherEnemyBase.h"
#include "AetherPrototypeEncounterConfig.generated.h"

USTRUCT(BlueprintType)
struct AETHERFALL_API FAetherPrototypeEncounterConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	bool bOverrideEnemySpawnCount = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter", meta = (ClampMin = "1", ClampMax = "4", EditCondition = "bOverrideEnemySpawnCount"))
	int32 EnemySpawnCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	bool bOverrideRoundKillGoal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter", meta = (ClampMin = "1", EditCondition = "bOverrideRoundKillGoal"))
	int32 RoundKillGoal = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	bool bOverrideEnemyArchetypeSequence = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter", meta = (EditCondition = "bOverrideEnemyArchetypeSequence"))
	TArray<EAetherEnemyArchetype> EnemyArchetypeSequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter|Completion")
	bool bGrantCompletionReward = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter|Completion", meta = (EditCondition = "bGrantCompletionReward"))
	FName CompletionRewardLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter|Completion", meta = (ClampMin = "0.0", EditCondition = "bGrantCompletionReward"))
	float CompletionAetherGaugeAmount = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter|Completion", meta = (EditCondition = "bGrantCompletionReward"))
	FString CompletionFeedbackLabel;
};
