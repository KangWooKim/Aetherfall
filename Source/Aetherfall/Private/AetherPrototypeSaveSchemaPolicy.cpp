#include "AetherPrototypeSaveSchemaPolicy.h"

#include "AetherPrototypeSaveGame.h"

FName FAetherPrototypeSaveSchemaPolicy::GetCurrentSchemaLabel()
{
	return FName(TEXT("PrototypeCheckpointV2"));
}

void FAetherPrototypeSaveSchemaPolicy::StampCurrentSchema(UAetherPrototypeSaveGame& SaveGameObject)
{
	SaveGameObject.SaveSchemaVersion = CurrentSchemaVersion;
	SaveGameObject.SaveSchemaLabel = GetCurrentSchemaLabel();
}

FAetherPrototypeSaveSchemaLoadPlan FAetherPrototypeSaveSchemaPolicy::BuildLoadPlan(const UAetherPrototypeSaveGame& SaveGameObject)
{
	FAetherPrototypeSaveSchemaLoadPlan LoadPlan;
	LoadPlan.SourceSchemaVersion = SaveGameObject.SaveSchemaVersion;
	LoadPlan.TargetSchemaVersion = CurrentSchemaVersion;
	LoadPlan.bIsLegacyUnversioned = LoadPlan.SourceSchemaVersion <= LegacyUnversionedSchemaVersion;
	LoadPlan.bIsFutureVersion = LoadPlan.SourceSchemaVersion > CurrentSchemaVersion;
	LoadPlan.bNeedsMigration = LoadPlan.SourceSchemaVersion < CurrentSchemaVersion;
	LoadPlan.bCanLoad = !LoadPlan.bIsFutureVersion;
	LoadPlan.SummaryMessage = BuildSchemaSummary(LoadPlan);
	return LoadPlan;
}

FString FAetherPrototypeSaveSchemaPolicy::BuildSchemaSummary(const FAetherPrototypeSaveSchemaLoadPlan& LoadPlan)
{
	if (LoadPlan.bIsFutureVersion)
	{
		return FString::Printf(
			TEXT("Save schema unsupported / source %d / current %d"),
			LoadPlan.SourceSchemaVersion,
			LoadPlan.TargetSchemaVersion);
	}

	if (LoadPlan.bIsLegacyUnversioned)
	{
		return FString::Printf(
			TEXT("Save schema legacy migration / source %d / target %d"),
			LoadPlan.SourceSchemaVersion,
			LoadPlan.TargetSchemaVersion);
	}

	if (LoadPlan.bNeedsMigration)
	{
		return FString::Printf(
			TEXT("Save schema migration / source %d / target %d"),
			LoadPlan.SourceSchemaVersion,
			LoadPlan.TargetSchemaVersion);
	}

	return FString::Printf(
		TEXT("Save schema current / version %d"),
		LoadPlan.SourceSchemaVersion);
}
