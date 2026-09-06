/** 설정 서브시스템의 오디오 분류를 적용해 2D·월드 위치 사운드를 재생하는 Blueprint 공용 진입점이다. */
#include "AetherAudioSettingsLibrary.h"

#include "AetherSettingsSubsystem.h"
#include "Components/AudioComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

/** 유효한 사운드에 현재 SoundMix와 분류별 SoundClass를 적용한 뒤 재생한다. 사운드가 없으면 null을 반환한다. */
UAudioComponent* UAetherAudioSettingsLibrary::SpawnSound2DForCategory(
	const UObject* WorldContextObject,
	USoundBase* Sound,
	EAetherAudioCategory Category,
	float VolumeMultiplier,
	float PitchMultiplier,
	bool bPersistAcrossLevelTransition)
{
	if (!Sound)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UAetherSettingsSubsystem* SettingsSubsystem = GameInstance ? GameInstance->GetSubsystem<UAetherSettingsSubsystem>() : nullptr;
	if (SettingsSubsystem)
	{
		SettingsSubsystem->EnsureSoundMixApplied();
	}

	UAudioComponent* AudioComponent = UGameplayStatics::CreateSound2D(
		WorldContextObject,
		Sound,
		FMath::Max(0.0f, VolumeMultiplier),
		FMath::Max(0.01f, PitchMultiplier),
		0.0f,
		nullptr,
		bPersistAcrossLevelTransition,
		true);
	if (AudioComponent)
	{
		AudioComponent->SoundClassOverride = SettingsSubsystem ? SettingsSubsystem->GetSoundClassForCategory(Category) : nullptr;
		AudioComponent->Play();
	}
	return AudioComponent;
}

/** 월드 위치에서 사운드를 생성하고 설정된 오디오 분류를 연결한다. 볼륨과 피치는 재생 가능한 하한으로 보정한다. */
UAudioComponent* UAetherAudioSettingsLibrary::SpawnSoundAtLocationForCategory(
	const UObject* WorldContextObject,
	USoundBase* Sound,
	FVector Location,
	EAetherAudioCategory Category,
	float VolumeMultiplier,
	float PitchMultiplier)
{
	if (!Sound)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	UAetherSettingsSubsystem* SettingsSubsystem = GameInstance ? GameInstance->GetSubsystem<UAetherSettingsSubsystem>() : nullptr;
	if (SettingsSubsystem)
	{
		SettingsSubsystem->EnsureSoundMixApplied();
	}

	UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
		WorldContextObject,
		Sound,
		Location,
		FRotator::ZeroRotator,
		FMath::Max(0.0f, VolumeMultiplier),
		FMath::Max(0.01f, PitchMultiplier));
	if (AudioComponent)
	{
		AudioComponent->SoundClassOverride = SettingsSubsystem ? SettingsSubsystem->GetSoundClassForCategory(Category) : nullptr;
	}
	return AudioComponent;
}
