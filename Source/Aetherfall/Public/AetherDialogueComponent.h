#pragma once

#include "CoreMinimal.h"
#include "AetherDialogueTypes.h"
#include "Components/ActorComponent.h"
#include "AetherDialogueComponent.generated.h"

class UAetherDialogueDataAsset;
class UAetherDialogueTtsService;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherDialogueGameplayEventSignature, FName, EventLabel);

UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherDialogueComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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
	void SetPlayedDialogueLabels(const TSet<FName>& DialogueLabels);
	const TSet<FName>& GetPlayedDialogueLabels() const { return PlayedDialogueLabels; }

private:
	void RebuildRuntimeSequences();
	void EnsureTtsService();
	void BuildPrototypeFallbackDialogue(TArray<FAetherDialogueSequence>& OutSequences) const;
	bool StartSequence(const FAetherDialogueSequence& Sequence);
	void StartCurrentLine();
	void AdvanceToNextLine();
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
