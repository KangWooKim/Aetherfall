#pragma once

#include "CoreMinimal.h"
#include "AetherDialogueTypes.h"
#include "Engine/DataAsset.h"
#include "AetherDialogueDataAsset.generated.h"

UCLASS(BlueprintType)
class AETHERFALL_API UAetherDialogueDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aetherfall|Dialogue")
	TArray<FAetherDialogueSequence> DialogueSequences;
};
