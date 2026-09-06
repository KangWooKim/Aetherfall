#pragma once

#include "CoreMinimal.h"
#include "AetherSettingsTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AetherAudioSettingsLibrary.generated.h"

class USoundBase;
class UAudioComponent;

/** 설정 서브시스템의 오디오 분류를 적용해 2D·월드 위치 사운드를 재생하는 Blueprint 공용 진입점이다. */
UCLASS()
class AETHERFALL_API UAetherAudioSettingsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 유효한 사운드에 현재 SoundMix와 분류별 SoundClass를 적용한 뒤 재생한다. 사운드가 없으면 null을 반환한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Audio", meta = (WorldContext = "WorldContextObject"))
	static UAudioComponent* SpawnSound2DForCategory(
		const UObject* WorldContextObject,
		USoundBase* Sound,
		EAetherAudioCategory Category,
		float VolumeMultiplier = 1.0f,
		float PitchMultiplier = 1.0f,
		bool bPersistAcrossLevelTransition = false);

	/** 월드 위치에서 사운드를 생성하고 설정된 오디오 분류를 연결한다. 볼륨과 피치는 재생 가능한 하한으로 보정한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Audio", meta = (WorldContext = "WorldContextObject"))
	static UAudioComponent* SpawnSoundAtLocationForCategory(
		const UObject* WorldContextObject,
		USoundBase* Sound,
		FVector Location,
		EAetherAudioCategory Category,
		float VolumeMultiplier = 1.0f,
		float PitchMultiplier = 1.0f);
};
