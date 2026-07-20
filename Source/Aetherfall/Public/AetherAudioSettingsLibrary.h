#pragma once

#include "CoreMinimal.h"
#include "AetherSettingsTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AetherAudioSettingsLibrary.generated.h"

class USoundBase;
class UAudioComponent;

UCLASS()
class AETHERFALL_API UAetherAudioSettingsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Audio", meta = (WorldContext = "WorldContextObject"))
	static UAudioComponent* SpawnSound2DForCategory(
		const UObject* WorldContextObject,
		USoundBase* Sound,
		EAetherAudioCategory Category,
		float VolumeMultiplier = 1.0f,
		float PitchMultiplier = 1.0f,
		bool bPersistAcrossLevelTransition = false);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Audio", meta = (WorldContext = "WorldContextObject"))
	static UAudioComponent* SpawnSoundAtLocationForCategory(
		const UObject* WorldContextObject,
		USoundBase* Sound,
		FVector Location,
		EAetherAudioCategory Category,
		float VolumeMultiplier = 1.0f,
		float PitchMultiplier = 1.0f);
};
