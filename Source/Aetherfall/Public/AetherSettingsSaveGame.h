#pragma once

#include "CoreMinimal.h"
#include "AetherSettingsTypes.h"
#include "GameFramework/SaveGame.h"
#include "AetherSettingsSaveGame.generated.h"

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
