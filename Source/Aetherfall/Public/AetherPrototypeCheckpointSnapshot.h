#pragma once

#include "CoreMinimal.h"

class UAetherPrototypeSaveGame;

/** 게임 모드의 진행 집합과 플레이어 상태를 저장 데이터로 변환하기 위한 스냅샷이다. */
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

/** 실행 중 진행 집합과 SaveGame 배열을 상호 변환하고 호환 상태 보정 및 재현 가능한 QA 요약을 제공한다. */
class AETHERFALL_API FAetherPrototypeCheckpointSnapshot
{
public:
	/** 명시된 양수 진행 등급을 우선하고, 없으면 라벨 마지막 숫자 또는 한 글자 알파벳으로 등급을 추론한다. */
	static int32 ResolveProgressRank(FName CheckpointLabel, int32 ExplicitCheckpointProgressRank);

	/** 현재 스키마를 기록하고 실행 상태의 라벨 집합을 저장 가능한 배열로 복사한다. */
	static void WriteSaveGame(UAetherPrototypeSaveGame& SaveGameObject, const FAetherPrototypeCheckpointSnapshotState& SnapshotState);
	/** 호출자가 스키마 허용 여부를 확인한 저장을 실행 상태로 변환하며 마지막 완료 구간과 결말 상태의 호환값을 보완한다. */
	static FAetherPrototypeCheckpointSnapshotState ReadSaveGame(const UAetherPrototypeSaveGame& SaveGameObject, FName CathedralEndingGoalLabel);

	static FString BuildQASummary(const FString& Context, const FAetherPrototypeCheckpointSnapshotState& SnapshotState);
	/** 비어 있지 않은 라벨을 정렬해 집합 순서에 영향받지 않는 QA 로그 문자열을 만든다. */
	static FString FormatNameSetForQA(const TSet<FName>& Labels);
};
