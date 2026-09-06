#pragma once

#include "CoreMinimal.h"
#include "AetherDialogueTypes.h"
#include "Components/ActorComponent.h"
#include "AetherDialogueComponent.generated.h"

class UAetherDialogueDataAsset;
class UAetherDialogueTtsService;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherDialogueGameplayEventSignature, FName, EventLabel);

/** 이벤트 라벨로 대사를 선택하고 재생 이력, 대기열, 자동 진행, 입력 차단 및 대사 완료 이벤트를 관리한다. */
UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherDialogueComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	/** 활성 대사의 시간과 음성 상태를 갱신하고 사용자 자동 진행 설정 및 대사 정책에 따라 다음 줄로 넘긴다. */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 재생 이력과 중복 요청을 확인하고, 다른 대사가 진행 중이면 트리거와 강제 재생 여부를 대기열에 보관한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Dialogue")
	bool TryStartDialogue(FName TriggerLabel, bool bForceReplay = false);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Dialogue")
	void SkipOrAdvanceDialogue();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Dialogue")
	void ResetPlayedDialogueLabels();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Dialogue")
	bool IsDialogueActive() const { return ActiveLineIndex != INDEX_NONE; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Dialogue")
	bool ShouldBlockGameplayInput() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Dialogue")
	FText GetCurrentSpeakerName() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Dialogue")
	FText GetCurrentDialogueText() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Dialogue")
	FText GetCurrentObjectiveHint() const;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Dialogue")
	FAetherDialogueGameplayEventSignature OnDialogueGameplayEvent;

	bool HasPlayedDialogueLabel(FName DialogueLabel) const;
	/** 저장 이력을 복원하기 전에 대기 요청과 활성 대화를 정리한다. */
	void SetPlayedDialogueLabels(const TSet<FName>& DialogueLabels);
	const TSet<FName>& GetPlayedDialogueLabels() const { return PlayedDialogueLabels; }

private:
	/** 대화 에셋과 선택적 기본 대사를 최초 한 번 결합하여 실행용 목록을 구성한다. */
	void RebuildRuntimeSequences();
	/** 설정된 음성 서비스 클래스를 생성하며, 미지정 시 시간 기반 모의 서비스를 사용한다. */
	void EnsureTtsService();
	void BuildPrototypeFallbackDialogue(TArray<FAetherDialogueSequence>& OutSequences) const;
	/** 대사 복사본을 활성화하고 저장 대상 시퀀스는 시작 시점에 재생 이력에 기록한다. */
	bool StartSequence(const FAetherDialogueSequence& Sequence);
	void StartCurrentLine();
	/** 현재 대사의 완료 이벤트를 보낸 뒤 음성을 정리하고 다음 대사 또는 대기 시퀀스로 이동한다. */
	void AdvanceToNextLine();
	/** 현재 대사 상태와 입력 차단을 해제한 뒤 대기 중인 다음 대화를 시작한다. */
	void FinishActiveSequence();
	void StartNextPendingDialogue();
	float EstimateLineDurationSeconds(const FAetherDialogueLine& DialogueLine) const;
	const FAetherDialogueLine* GetActiveLine() const;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Dialogue")
	TObjectPtr<UAetherDialogueDataAsset> DialogueDataAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Dialogue")
	bool bUsePrototypeFallbackDialogue = true;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Dialogue|TTS", meta = (ClampMin = "1.0"))
	float MockTtsCharactersPerSecond = 16.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Dialogue|TTS")
	TSubclassOf<UAetherDialogueTtsService> TtsServiceClass;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Dialogue|TTS", meta = (ClampMin = "0.1"))
	float MockTtsMinimumLineDuration = 1.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Dialogue|TTS", meta = (ClampMin = "0.1"))
	float MockTtsMaximumLineDuration = 5.0f;

	UPROPERTY(Transient)
	TObjectPtr<UAetherDialogueTtsService> TtsService;

	TArray<FAetherDialogueSequence> RuntimeSequences;
	TArray<FAetherDialogueLine> ActiveLines;
	TArray<FName> PendingDialogueTriggerLabels;
	TSet<FName> ForceReplayPendingTriggerLabels;
	TSet<FName> PlayedDialogueLabels;
	FName ActiveSequenceLabel = NAME_None;
	int32 ActiveLineIndex = INDEX_NONE;
	float ActiveLineElapsedSeconds = 0.0f;
	float ActiveLineDurationSeconds = 0.0f;
	bool bRuntimeSequencesBuilt = false;
	bool bActiveSequenceBlocksGameplayInput = false;
};
