#pragma once

#include "CoreMinimal.h"
#include "AetherDialogueTypes.generated.h"

UENUM(BlueprintType)
enum class EAetherDialogueAutoAdvancePolicy : uint8
{
	Manual UMETA(DisplayName = "Manual"),
	Timed UMETA(DisplayName = "Timed"),
	TtsComplete UMETA(DisplayName = "TTS Complete")
};

USTRUCT(BlueprintType)
struct FAetherDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue")
	FName LineLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue")
	FName SpeakerLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue")
	FText SpeakerDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue", meta = (MultiLine = "true"))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue|Voice")
	FName VoiceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue|Voice")
	FName TtsId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue|Flow")
	EAetherDialogueAutoAdvancePolicy AutoAdvancePolicy = EAetherDialogueAutoAdvancePolicy::TtsComplete;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue|Flow", meta = (ClampMin = "0.0"))
	float AutoAdvanceDelay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue|Gameplay")
	FName GameplayLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue|Gameplay")
	FText ObjectiveHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue|Gameplay")
	FName CompletionEventLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue|Input")
	bool bBlocksGameplayInput = false;
};

USTRUCT(BlueprintType)
struct FAetherDialogueSequence
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue")
	FName SequenceLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue")
	FName TriggerLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue")
	TArray<FAetherDialogueLine> Lines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue|Flow")
	bool bPlayOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue|Flow")
	bool bSaveWhenPlayed = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Dialogue|Input")
	bool bBlocksGameplayInput = false;
};
