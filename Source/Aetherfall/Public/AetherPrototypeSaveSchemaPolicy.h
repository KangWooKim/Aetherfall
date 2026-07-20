#pragma once

#include "CoreMinimal.h"

class UAetherPrototypeSaveGame;

struct FAetherPrototypeSaveSchemaLoadPlan
{
	int32 SourceSchemaVersion = 0;
	int32 TargetSchemaVersion = 0;
	bool bCanLoad = false;
	bool bNeedsMigration = false;
	bool bIsLegacyUnversioned = false;
	bool bIsFutureVersion = false;
	FString SummaryMessage;
};

class AETHERFALL_API FAetherPrototypeSaveSchemaPolicy
{
public:
	static constexpr int32 LegacyUnversionedSchemaVersion = 0;
	static constexpr int32 CurrentSchemaVersion = 2;

	static FName GetCurrentSchemaLabel();
	static void StampCurrentSchema(UAetherPrototypeSaveGame& SaveGameObject);
	static FAetherPrototypeSaveSchemaLoadPlan BuildLoadPlan(const UAetherPrototypeSaveGame& SaveGameObject);
	static FString BuildSchemaSummary(const FAetherPrototypeSaveSchemaLoadPlan& LoadPlan);
};
