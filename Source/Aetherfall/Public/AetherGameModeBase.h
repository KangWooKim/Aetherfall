#pragma once

#include "CoreMinimal.h"
#include "AetherCinematicTypes.h"
#include "AetherEnemyBase.h"
#include "AetherPrototypeEnemyAttackSlotCoordinator.h"
#include "AetherPrototypeCheckpointRetryCoordinator.h"
#include "AetherPrototypeCheckpointSnapshot.h"
#include "AetherPrototypeEncounterConfig.h"
#include "AetherPrototypeEncounterConfigPolicy.h"
#include "GameFramework/GameModeBase.h"
#include "AetherGameModeBase.generated.h"

class AAetherPrototypeDamageTarget;
class AAetherEnemyBase;
class UAetherDialogueComponent;
class UAetherPrototypeSaveGame;
class UAetherHealthComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAetherPrototypeRoundEventSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAetherPrototypeRoundProgressSignature, int32, DefeatedEnemies, int32, KillGoal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherPrototypeCheckpointEventSignature, FName, CheckpointLabel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherPrototypeEncounterEventSignature, FName, EncounterLabel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherPrototypeLevelGoalEventSignature, FName, GoalLabel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherPrototypeKeyEventSignature, FName, KeyLabel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherPrototypeRewardEventSignature, FName, RewardLabel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherPrototypeLoreEventSignature, FName, LoreLabel);

/** 전투 구간과 레벨 진행을 조정하고, 체크포인트 저장·복원 및 대화·연출·적 생성 서비스를 연결한다. */
UCLASS()
class AETHERFALL_API AAetherGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAetherGameModeBase();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype")
	void StartPrototypeCombatRound();

	/** 저장된 보스 처치 이력을 확인하고, 보스 도입 연출이 필요하면 완료 후 전투 시작을 재개한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype")
	void StartPrototypeCombatRoundForEncounter(FName EncounterLabel);

	/** 구간 설정 정책이 계산한 실행값을 반영하고 처치 목표 표시를 갱신한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype")
	void ApplyPrototypeEncounterConfig(const FAetherPrototypeEncounterConfig& EncounterConfig);

	/** 적과 공격 슬롯·재생성 타이머를 정리하고 라운드 상태 및 플레이어 전투 자원을 초기화한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype")
	void ResetPrototypeCombatRound();

	/** 진행 등급 하락 방지 조건을 통과하면 위치·라벨을 갱신하고 체크포인트 이벤트와 저장을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Checkpoint")
	bool ActivatePrototypeCheckpoint(const FTransform& CheckpointTransform, FName CheckpointLabel, int32 CheckpointProgressRank = 0);

	/** 저장 복원 계획에 따라 보스 실행 상태를 정리하고 플레이어 이동, 구간 재시작, 월드·자원 복원을 순서대로 수행한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Checkpoint")
	void ResetPrototypePlayerAtCheckpoint();

	/** 체크포인트에 따른 예약 계획을 받은 뒤 기존 재시작 타이머를 지우고 새 지연 시간으로 예약한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Checkpoint")
	void SchedulePrototypePlayerRetryAfterDefeat();

	/** 저장 슬롯 삭제가 성공한 경우에만 실행 중 진행 이력과 체크포인트 표시를 초기화한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Checkpoint")
	bool ClearPrototypeCheckpointProgress();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Checkpoint")
	bool IsPrototypeDefeatRetryScheduled() const { return PrototypeCheckpointRetry.IsRetryScheduled(); }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Checkpoint")
	float GetPrototypeDefeatRetryRemainingTime() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Checkpoint")
	bool HasActivePrototypeCheckpoint() const { return bHasActivePrototypeCheckpoint; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Checkpoint")
	FName GetActivePrototypeCheckpointLabel() const { return ActivePrototypeCheckpointLabel; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Checkpoint")
	int32 GetActivePrototypeCheckpointProgressRank() const { return ActivePrototypeCheckpointProgressRank; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Checkpoint")
	bool HasPrototypeCheckpointFeedback() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Checkpoint")
	FString GetPrototypeCheckpointFeedbackLabel() const { return PrototypeCheckpointFeedbackLabel; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Checkpoint")
	FLinearColor GetPrototypeCheckpointFeedbackColor() const { return PrototypeCheckpointFeedbackColor; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Checkpoint")
	float GetPrototypeCheckpointFeedbackRemainingTime() const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Progress")
	void RegisterPrototypeProgressFeedback(const FString& FeedbackLabel, const FLinearColor& FeedbackColor);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Progress")
	bool HasPrototypeProgressFeedback() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Progress")
	FString GetPrototypeProgressFeedbackLabel() const { return PrototypeProgressFeedbackLabel; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Progress")
	FLinearColor GetPrototypeProgressFeedbackColor() const { return PrototypeProgressFeedbackColor; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Progress")
	float GetPrototypeProgressFeedbackRemainingTime() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Encounter")
	FName GetActivePrototypeEncounterLabel() const { return ActivePrototypeEncounterLabel; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Encounter")
	FName GetLastCompletedPrototypeEncounterLabel() const { return LastCompletedPrototypeEncounterLabel; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Encounter")
	bool HasCompletedPrototypeEncounter(FName EncounterLabel) const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Boss")
	bool IsPrototypeBossEncounterActive() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Boss")
	bool HasCompletedPrototypeBossEncounter() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Boss")
	FName GetPrototypeBossEncounterLabel() const;

	/** 완료 상태와 최초 경과 시간을 기록하고 완료 이벤트·대사·저장을 연결한다. 이전 완료를 대성당 결말로 승격하는 경로를 허용한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Level")
	void CompletePrototypeLevel(FName GoalLabel);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Level")
	bool IsPrototypeLevelComplete() const { return bPrototypeLevelComplete; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Level")
	FName GetCompletedPrototypeLevelGoalLabel() const { return CompletedPrototypeLevelGoalLabel; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Level")
	bool IsCathedralEndingComplete() const { return bCathedralEndingComplete; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Level")
	FName GetPrototypeCathedralEndingGoalLabel() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Pacing")
	float GetPrototypeRouteElapsedSeconds() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Pacing")
	float GetPrototypeRouteTargetMinSeconds() const { return PrototypeRouteTargetMinSeconds; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Pacing")
	float GetPrototypeRouteTargetMaxSeconds() const { return PrototypeRouteTargetMaxSeconds; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Pacing")
	FString GetPrototypeRoutePacingStatusLabel() const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Dialogue")
	bool TryStartPrototypeDialogue(FName DialogueTriggerLabel, bool bForceReplay = false);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Dialogue")
	void AdvancePrototypeDialogue();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Dialogue")
	bool IsPrototypeDialogueActive() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Dialogue")
	bool ShouldPrototypeDialogueBlockGameplayInput() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Dialogue")
	FText GetPrototypeDialogueSpeakerName() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Dialogue")
	FText GetPrototypeDialogueText() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Dialogue")
	FText GetPrototypeDialogueObjectiveHint() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Dialogue")
	bool HasPlayedPrototypeDialogueLabel(FName DialogueLabel) const;

	bool ShouldShowPrototypeLevelCompleteBanner() const;
	float GetPrototypeLevelCompleteBannerRemainingTime() const;

	/** 중복되지 않은 열쇠 라벨을 진행 집합에 추가하고 획득 이벤트와 대응 대화를 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Interaction")
	void CollectPrototypeKey(FName KeyLabel);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Interaction")
	bool HasCollectedPrototypeKey(FName KeyLabel) const;

	/** 보상 라벨의 중복 수집을 막고 진행 이벤트를 발행한다. 실제 보상 수량 지급은 호출 측이 담당한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Interaction")
	void CollectPrototypeReward(FName RewardLabel);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Interaction")
	bool HasCollectedPrototypeReward(FName RewardLabel) const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Interaction")
	void CollectPrototypeLore(FName LoreLabel);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Interaction")
	bool HasCollectedPrototypeLore(FName LoreLabel) const;

	bool TryAcquirePrototypeEnemyAttackSlot(AAetherEnemyBase* RequestingEnemy);
	void ReleasePrototypeEnemyAttackSlot(AAetherEnemyBase* ReleasingEnemy);
	void DelayPrototypeEnemyAttacks(float DelayDuration);
	bool IsPrototypeCombatRoundGoalEnabled() const { return bUsePrototypeCombatRoundGoal; }
	bool IsPrototypeCombatRoundComplete() const { return bPrototypeCombatRoundComplete; }
	bool ShouldShowPrototypeRoundClearBanner() const;
	float GetPrototypeRoundClearBannerRemainingTime() const;
	int32 GetPrototypeRoundDefeatCount() const { return PrototypeRoundDefeatCount; }
	int32 GetPrototypeRoundKillGoal() const { return PrototypeRoundKillGoal; }
	int32 GetLivingPrototypeEnemyCountForHUD() const { return GetLivingPrototypeEnemyCount(); }

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Prototype|Round Goal")
	FAetherPrototypeRoundEventSignature OnPrototypeCombatRoundReset;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Prototype|Round Goal")
	FAetherPrototypeRoundProgressSignature OnPrototypeRoundProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Prototype|Round Goal")
	FAetherPrototypeRoundEventSignature OnPrototypeRoundGoalMet;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Prototype|Round Goal")
	FAetherPrototypeRoundEventSignature OnPrototypeRoundCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Prototype|Checkpoint")
	FAetherPrototypeCheckpointEventSignature OnPrototypeCheckpointActivated;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Prototype|Encounter")
	FAetherPrototypeEncounterEventSignature OnPrototypeEncounterReset;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Prototype|Encounter")
	FAetherPrototypeEncounterEventSignature OnPrototypeEncounterCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Prototype|Level")
	FAetherPrototypeLevelGoalEventSignature OnPrototypeLevelCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Prototype|Interaction")
	FAetherPrototypeKeyEventSignature OnPrototypeKeyCollected;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Prototype|Interaction")
	FAetherPrototypeRewardEventSignature OnPrototypeRewardCollected;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Prototype|Interaction")
	FAetherPrototypeLoreEventSignature OnPrototypeLoreCollected;

protected:
	/** 액터의 시작 처리 전에 저장된 체크포인트 진행 상태를 읽는다. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SpawnPrototypeDamageTarget();
	void DestroyPrototypeDamageTarget();
	/** 생성 정책으로 위치·원형을 결정하고 지연 생성을 통해 원형을 먼저 적용한 뒤 사망 이벤트를 연결한다. */
	void SpawnPrototypeEnemy();
	void DestroyPrototypeEnemies();
	void CleanPrototypeEnemyReferences();
	int32 GetLivingPrototypeEnemyCount() const;
	FAetherPrototypeEncounterRuntimeConfig BuildActivePrototypeEncounterRuntimeConfig() const;
	void CommitPrototypeEncounterRuntimeConfig(const FAetherPrototypeEncounterRuntimeConfig& RuntimeConfig);
	void SchedulePrototypeEnemyRespawn();
	/** 완료 처리를 중복 실행하지 않도록 상태를 먼저 기록하고, 보상·구간 이벤트·저장·보스 후속 연출을 처리한다. */
	void CompletePrototypeCombatRound();
	void PlayPrototypeRoundCueSound(USoundBase* CueSound, FName CueName) const;
	void ShowPrototypeMessage(const FString& Message, const FColor& Color) const;
	FAetherPrototypeCheckpointSnapshotState BuildPrototypeCheckpointSnapshotState(const AAetherfallCharacter* PlayerCharacter) const;
	void ApplyPrototypeCheckpointSnapshotState(const FAetherPrototypeCheckpointSnapshotState& SnapshotState);
	void LogPrototypeCheckpointSnapshotQAState(const FString& Context) const;
	/** 저장 스키마를 검증한 후 진행 상태를 채우고, 플레이어 적용 전까지 저장을 지연한다. */
	void LoadPrototypeCheckpointSnapshot();
	/** 유효한 체크포인트와 플레이어가 있고 복원 지연이 해제된 경우에만 현재 상태를 저장 슬롯에 기록한다. */
	void SavePrototypeCheckpointSnapshot();
	/** 체크포인트 위치·시점과 체력·회복 아이템을 적용한 뒤 저장 지연을 해제한다. */
	void ApplyLoadedPrototypePlayerState();
	/** 지원하는 저장 스키마인지 확인한 후 스냅샷 변환 결과를 실행 상태에 반영한다. */
	bool PopulatePrototypeCheckpointStateFromSaveGame(const UAetherPrototypeSaveGame* SaveGameObject);
	/** 사망 후 재시도를 위해 디스크 스냅샷을 다시 읽고 복원 중 덮어쓰기를 막는다. */
	bool PreparePrototypeCheckpointRetryRestore();
	/** 스냅샷의 라벨 집합을 월드 복원기에 전달해 상호작용 액터 상태를 맞춘다. */
	void RefreshPrototypeCheckpointWorldState();
	void RegisterPrototypeCheckpointFeedback(const FString& FeedbackLabel, const FLinearColor& FeedbackColor);
	/** 보스 구간 복원 전에 공격 예약과 락온, 이전 아우렐 인스턴스를 정리한다. */
	void ResetPrototypeBossEncounterRuntimeForCheckpointRestore(FName RuntimeEncounterLabel);
	int32 DestroyPrototypeEnemiesByArchetype(EAetherEnemyArchetype EnemyArchetype);
	void ResetPrototypeRoutePacingTimer();
	void LogPrototypeRoutePacingComplete(FName GoalLabel) const;
	void TryStartPrototypeDialogueForLabeledEvent(const TCHAR* EventPrefix, FName EventLabel, bool bForceReplay = false);
	void ContinuePrototypeBeginPlayAfterIntro();
	void SignalGameplayWorldReady();
	bool TryRequestPrototypeCinematic(EAetherCinematicTrigger Trigger, FName EventLabel);
	/** 대기 중이던 시작 또는 보스 도입 연출이 끝나면 해당 게임 흐름을 이어간다. */
	void HandlePrototypeCinematicFinished(const FAetherCinematicRuntimeState& RuntimeState);

	/** 사망한 적의 공격 슬롯을 반환하고 라운드 정책 결과에 따라 목표 알림, 완료 또는 보충 생성을 수행한다. */
	UFUNCTION()
	void HandlePrototypeEnemyDeath(UAetherHealthComponent* DeadHealthComponent, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|QA")
	bool bAutoSpawnPrototypeDamageTarget = false;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|QA")
	bool bClearPrototypeDamageTargetOnCombatRoundReset = true;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|QA")
	bool bShowPrototypeScreenDebugMessages = false;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|QA", meta = (ClampMin = "100.0", EditCondition = "bAutoSpawnPrototypeDamageTarget"))
	float PrototypeDamageTargetSpawnDistance = 240.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|QA", meta = (EditCondition = "bAutoSpawnPrototypeDamageTarget"))
	TSubclassOf<AAetherPrototypeDamageTarget> PrototypeDamageTargetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype")
	bool bAutoSpawnPrototypeEnemy = true;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype")
	bool bRespawnPrototypeEnemiesDuringRound = true;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Checkpoint")
	bool bRestartActiveEncounterOnCheckpointReset = true;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Checkpoint")
	bool bPreventPrototypeCheckpointDowngrade = true;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Checkpoint", meta = (ClampMin = "0.0"))
	float PrototypeDefeatRetryDelay = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Checkpoint", meta = (ClampMin = "0.1"))
	float PrototypeCheckpointFeedbackDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Progress", meta = (ClampMin = "0.1"))
	float PrototypeProgressFeedbackDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype", meta = (ClampMin = "200.0"))
	float PrototypeEnemySpawnDistance = 650.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype", meta = (ClampMin = "0.0"))
	float PrototypeEnemySpawnRightOffset = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype")
	TSubclassOf<AAetherEnemyBase> PrototypeEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype", meta = (ClampMin = "1", ClampMax = "4"))
	int32 PrototypeEnemySpawnCount = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype", meta = (ClampMin = "100.0"))
	float PrototypeEnemySpawnSpacing = 420.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype", meta = (ClampMin = "0.0"))
	float PrototypeEnemyRespawnDelay = 2.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Round Goal")
	bool bUsePrototypeCombatRoundGoal = true;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Round Goal", meta = (ClampMin = "1", EditCondition = "bUsePrototypeCombatRoundGoal"))
	int32 PrototypeRoundKillGoal = 6;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Round Goal", meta = (ClampMin = "0.1"))
	float PrototypeRoundClearBannerDuration = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Level", meta = (ClampMin = "0.1"))
	float PrototypeLevelCompleteBannerDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Pacing", meta = (ClampMin = "60.0"))
	float PrototypeRouteTargetMinSeconds = 1200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Pacing", meta = (ClampMin = "60.0"))
	float PrototypeRouteTargetMaxSeconds = 1800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Enemy AI")
	bool bCoordinatePrototypeEnemyAttacks = true;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Prototype|Enemy AI")
	TArray<EAetherEnemyArchetype> PrototypeEnemyArchetypeSequence;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Assets|Audio")
	bool bAutoPlayPrototypeCombatMusic = false;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Assets|Audio")
	TObjectPtr<USoundBase> PrototypeCombatMusic;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Assets|Audio", meta = (ClampMin = "0.0"))
	float PrototypeCombatMusicVolume = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Assets|Audio|Round Cue")
	bool bEnablePrototypeRoundCueSounds = true;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Assets|Audio|Round Cue", meta = (EditCondition = "bEnablePrototypeRoundCueSounds"))
	TObjectPtr<USoundBase> PrototypeRoundGoalMetSound;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Assets|Audio|Round Cue", meta = (EditCondition = "bEnablePrototypeRoundCueSounds"))
	TObjectPtr<USoundBase> PrototypeRoundClearSound;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Assets|Audio|Round Cue", meta = (ClampMin = "0.0", EditCondition = "bEnablePrototypeRoundCueSounds"))
	float PrototypeRoundCueSoundVolume = 0.85f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Assets|Audio|Round Cue", meta = (ClampMin = "0.0", EditCondition = "bEnablePrototypeRoundCueSounds"))
	float PrototypeRoundCueSoundVolumeVariance = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Assets|Audio|Round Cue", meta = (ClampMin = "0.1", EditCondition = "bEnablePrototypeRoundCueSounds"))
	float PrototypeRoundCueSoundPitchMin = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Assets|Audio|Round Cue", meta = (ClampMin = "0.1", EditCondition = "bEnablePrototypeRoundCueSounds"))
	float PrototypeRoundCueSoundPitchMax = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Prototype|Dialogue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherDialogueComponent> DialogueComponent;

	/** 게임 모드가 생성한 적을 약한 참조로 추적하여 소멸한 액터를 소유하지 않는다. */
	TArray<TWeakObjectPtr<AAetherEnemyBase>> CurrentPrototypeEnemies;
	TWeakObjectPtr<AAetherPrototypeDamageTarget> CurrentPrototypeDamageTarget;
	FAetherPrototypeEnemyAttackSlotCoordinator PrototypeEnemyAttackSlots;
	int32 PrototypeRoundDefeatCount = 0;
	bool bPrototypeCombatRoundComplete = false;
	bool bPrototypeRoundGoalMetCuePlayed = false;
	double PrototypeRoundClearBannerExpireTime = -1.0;
	double PrototypeLevelCompleteBannerExpireTime = -1.0;
	double PrototypeRouteStartTime = -1.0;
	double PrototypeRouteCompletedElapsedSeconds = -1.0;
	FName ActivePrototypeEncounterLabel = NAME_None;
	FName LastCompletedPrototypeEncounterLabel = NAME_None;
	FName ActivePrototypeEncounterCompletionRewardLabel = NAME_None;
	float ActivePrototypeEncounterCompletionAetherReward = 0.0f;
	FString ActivePrototypeEncounterCompletionFeedbackLabel;
	TSet<FName> CompletedPrototypeEncounterLabels;
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
	bool bHasActivePrototypeCheckpoint = false;
	FTransform ActivePrototypeCheckpointTransform = FTransform::Identity;
	FName ActivePrototypeCheckpointLabel = NAME_None;
	int32 ActivePrototypeCheckpointProgressRank = 0;
	bool bHasLoadedPrototypeCheckpointSnapshot = false;
	/** 복원 도중 기본 플레이어 값이 기존 저장 데이터를 덮어쓰지 않도록 저장을 잠시 미룬다. */
	bool bDeferPrototypeCheckpointSaveUntilLoadedStateApplied = false;
	FAetherPrototypeCheckpointRetryCoordinator PrototypeCheckpointRetry;
	FString PrototypeCheckpointFeedbackLabel;
	FLinearColor PrototypeCheckpointFeedbackColor = FLinearColor::White;
	double PrototypeCheckpointFeedbackExpireTime = -1.0;
	FString PrototypeProgressFeedbackLabel;
	FLinearColor PrototypeProgressFeedbackColor = FLinearColor::White;
	double PrototypeProgressFeedbackExpireTime = -1.0;
	float LoadedPrototypePlayerCurrentHealth = 100.0f;
	int32 LoadedPrototypeHealingItemCount = 0;
	FTimerHandle PrototypeEnemyRespawnTimerHandle;
	FTimerHandle PrototypeDefeatRetryTimerHandle;
	FDelegateHandle PrototypeCinematicFinishedHandle;
	FName PendingPrototypeEncounterAfterCinematic = NAME_None;
	bool bPrototypeBeginPlayWaitingForIntroCinematic = false;
	bool bBypassNextBossIntroCinematic = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Interaction")
	void RecordPrototypeLeverActivated(FName LeverLabel);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Interaction")
	bool HasActivatedPrototypeLever(FName LeverLabel) const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Interaction")
	void RecordPrototypeProgressGateUnlocked(FName GateLabel);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Interaction")
	bool HasUnlockedPrototypeProgressGate(FName GateLabel) const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Interaction")
	void RecordPrototypeKeyGateUnlocked(FName GateLabel);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Interaction")
	bool HasUnlockedPrototypeKeyGate(FName GateLabel) const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Prototype|Interaction")
	void RecordPrototypeChestOpened(FName ChestLabel);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Prototype|Interaction")
	bool HasOpenedPrototypeChest(FName ChestLabel) const;
};
