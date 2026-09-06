/** 저장 버전을 비교해 불러오기 허용 여부와 이전 형식 보정 필요성을 계산한다. */
#include "AetherPrototypeSaveSchemaPolicy.h"

#include "AetherPrototypeSaveGame.h"

FName FAetherPrototypeSaveSchemaPolicy::GetCurrentSchemaLabel()
{
	return FName(TEXT("PrototypeCheckpointV2"));
}

/** 저장 객체에 현재 버전과 형식 라벨을 기록한다. */
void FAetherPrototypeSaveSchemaPolicy::StampCurrentSchema(UAetherPrototypeSaveGame& SaveGameObject)
{
	SaveGameObject.SaveSchemaVersion = CurrentSchemaVersion;
	SaveGameObject.SaveSchemaLabel = GetCurrentSchemaLabel();
}

/** 미래 버전은 거부하고 이전 버전은 마이그레이션 대상으로 표시한다. 실제 필드 복원은 스냅샷 정책이 수행한다. */
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

/** 호환성 판단 결과를 로그와 진단에 사용할 요약 문자열로 구성한다. */
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
