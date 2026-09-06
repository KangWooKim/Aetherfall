/** 대사 내용, 화자·음성 식별자, 완료 이벤트, 자동 진행 및 입력 차단 정책을 정의한다. */
#pragma once

#include "CoreMinimal.h"
#include "AetherDialogueTypes.generated.h"

/** 대사 진행을 수동 입력·지정 시간·음성 완료 상태 중 하나에 연결한다. */
UENUM(BlueprintType)
enum class EAetherDialogueAutoAdvancePolicy : uint8
{
	Manual UMETA(DisplayName = "Manual"),
	Timed UMETA(DisplayName = "Timed"),
	TtsComplete UMETA(DisplayName = "TTS Complete")
};

/** 한 줄의 화자·본문·음성 및 진행 시간을 정의한다. */
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

/** 트리거로 시작할 여러 대사와 반복 재생 정책을 묶는다. */
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
