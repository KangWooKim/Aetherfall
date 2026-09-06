#pragma once

#include "CoreMinimal.h"
#include "AetherDialogueTypes.h"
#include "UObject/Object.h"
#include "AetherDialogueTtsService.generated.h"

/** 대화 진행에서 사용할 음성 서비스 경계를 정의하고, 실제 음성 대신 경과 시간으로 완료를 알리는 모의 구현을 제공한다. */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class AETHERFALL_API UAetherDialogueTtsService : public UObject
{
	GENERATED_BODY()

public:
	/** 서비스 구현에 현재 대사와 대체 재생 시간을 전달한다. 모의 구현은 시간을 세며 실제 음성을 생성하지 않는다. */
	virtual void StartLine(const FAetherDialogueLine& DialogueLine, float FallbackDurationSeconds);
	virtual void TickService(float DeltaTime);
	/** 현재 재생을 중단한다. 모의 구현은 남은 시간을 비워 즉시 완료 상태로 전환한다. */
	virtual void SkipLine();
	virtual bool IsLinePlaying() const;
	virtual bool HasLineFinished() const;
};

/** 남은 시간으로 음성 재생·완료 상태를 흉내내는 대체 서비스다. 실제 음성 생성 구현은 아니다. */
UCLASS(BlueprintType)
class AETHERFALL_API UAetherDialogueMockTtsService : public UAetherDialogueTtsService
{
	GENERATED_BODY()

public:
	/** 서비스 구현에 현재 대사와 대체 재생 시간을 전달한다. 모의 구현은 시간을 세며 실제 음성을 생성하지 않는다. */
	virtual void StartLine(const FAetherDialogueLine& DialogueLine, float FallbackDurationSeconds) override;
	virtual void TickService(float DeltaTime) override;
	/** 현재 재생을 중단한다. 모의 구현은 남은 시간을 비워 즉시 완료 상태로 전환한다. */
	virtual void SkipLine() override;
	virtual bool IsLinePlaying() const override;
	virtual bool HasLineFinished() const override;

private:
	float RemainingDurationSeconds = 0.0f;
	bool bPlaying = false;
};
