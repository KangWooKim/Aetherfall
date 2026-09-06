#pragma once

#include "CoreMinimal.h"
#include "AetherDialogueTypes.h"
#include "Engine/DataAsset.h"
#include "AetherDialogueDataAsset.generated.h"

/** 트리거와 대사 묶음을 편집 가능한 데이터 에셋으로 제공한다. */
UCLASS(BlueprintType)
class AETHERFALL_API UAetherDialogueDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aetherfall|Dialogue")
	TArray<FAetherDialogueSequence> DialogueSequences;
};
