#pragma once

#include "CoreMinimal.h"

class UAetherPrototypeSaveGame;

/** 저장 버전의 호환성 및 이전 형식 보정 필요성을 호출자에게 전달한다. */
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

/** 저장 버전을 비교해 불러오기 허용 여부와 이전 형식 보정 필요성을 계산한다. */
class AETHERFALL_API FAetherPrototypeSaveSchemaPolicy
{
public:
	static constexpr int32 LegacyUnversionedSchemaVersion = 0;
	static constexpr int32 CurrentSchemaVersion = 2;

	static FName GetCurrentSchemaLabel();
	/** 저장 객체에 현재 버전과 형식 라벨을 기록한다. */
	static void StampCurrentSchema(UAetherPrototypeSaveGame& SaveGameObject);
	/** 미래 버전은 거부하고 이전 버전은 마이그레이션 대상으로 표시한다. 실제 필드 복원은 스냅샷 정책이 수행한다. */
	static FAetherPrototypeSaveSchemaLoadPlan BuildLoadPlan(const UAetherPrototypeSaveGame& SaveGameObject);
	/** 호환성 판단 결과를 로그와 진단에 사용할 요약 문자열로 구성한다. */
	static FString BuildSchemaSummary(const FAetherPrototypeSaveSchemaLoadPlan& LoadPlan);
};
