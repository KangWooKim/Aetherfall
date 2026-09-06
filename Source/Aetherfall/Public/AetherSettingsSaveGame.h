#pragma once

#include "CoreMinimal.h"
#include "AetherSettingsTypes.h"
#include "GameFramework/SaveGame.h"
#include "AetherSettingsSaveGame.generated.h"

/** 영상 설정과 분리하여 오디오·접근성·조작 설정을 버전 정보와 함께 저장한다. */
UCLASS()
class AETHERFALL_API UAetherSettingsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 SchemaVersion = 1;

	UPROPERTY()
	FName SchemaLabel = FName(TEXT("AetherSettingsV1"));

	UPROPERTY()
	FAetherCustomSettings Settings;
};
