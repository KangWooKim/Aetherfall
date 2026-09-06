#pragma once

#include "CoreMinimal.h"
#include "AetherPrototypeEncounterConfig.h"
#include "Engine/DataAsset.h"
#include "AetherPrototypeEncounterDataAsset.generated.h"

/** 구간 전투 설정과 시작 안내 문구·색상을 재사용 가능한 데이터 에셋으로 제공한다. */
UCLASS(BlueprintType)
class AETHERFALL_API UAetherPrototypeEncounterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FAetherPrototypeEncounterConfig& GetEncounterConfig() const { return EncounterConfig; }
	const FString& GetEncounterStartFeedbackLabel() const { return EncounterStartFeedbackLabel; }
	const FLinearColor& GetEncounterStartFeedbackColor() const { return EncounterStartFeedbackColor; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter", meta = (AllowPrivateAccess = "true"))
	FAetherPrototypeEncounterConfig EncounterConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter|Feedback", meta = (AllowPrivateAccess = "true"))
	FString EncounterStartFeedbackLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Encounter|Feedback", meta = (AllowPrivateAccess = "true"))
	FLinearColor EncounterStartFeedbackColor = FLinearColor(0.62f, 0.36f, 1.0f, 1.0f);
};
