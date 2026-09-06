#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AetherPrototypeSaveGame.generated.h"

/** 체크포인트, 플레이어 상태, 전투 진행과 수집 라벨을 직렬화하는 저장 데이터다. 액터 포인터 대신 라벨을 보관한다. */
UCLASS()
class AETHERFALL_API UAetherPrototypeSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** 저장 형식의 호환성 판단에 사용하는 버전이다. 현재 버전보다 높으면 불러오기를 거부한다. */
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

	/** 체크포인트 진행 순서를 비교해 이전 구간으로 기록이 낮아지는 것을 막는 기준이다. */
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

	/** 이미 시작한 대화 라벨을 저장해 일반 트리거의 반복 재생을 제어한다. */
	UPROPERTY()
	TArray<FName> PlayedPrototypeDialogueLabels;
};
