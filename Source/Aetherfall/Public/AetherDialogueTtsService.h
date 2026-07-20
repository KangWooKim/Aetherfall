#pragma once

#include "CoreMinimal.h"
#include "AetherDialogueTypes.h"
#include "UObject/Object.h"
#include "AetherDialogueTtsService.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class AETHERFALL_API UAetherDialogueTtsService : public UObject
{
	GENERATED_BODY()

public:
	virtual void StartLine(const FAetherDialogueLine& DialogueLine, float FallbackDurationSeconds);
	virtual void TickService(float DeltaTime);
	virtual void SkipLine();
	virtual bool IsLinePlaying() const;
	virtual bool HasLineFinished() const;
};

UCLASS(BlueprintType)
class AETHERFALL_API UAetherDialogueMockTtsService : public UAetherDialogueTtsService
{
	GENERATED_BODY()

public:
	virtual void StartLine(const FAetherDialogueLine& DialogueLine, float FallbackDurationSeconds) override;
	virtual void TickService(float DeltaTime) override;
	virtual void SkipLine() override;
	virtual bool IsLinePlaying() const override;
	virtual bool HasLineFinished() const override;

private:
	float RemainingDurationSeconds = 0.0f;
	bool bPlaying = false;
};
