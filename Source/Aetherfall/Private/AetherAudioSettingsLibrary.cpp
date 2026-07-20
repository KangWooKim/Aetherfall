#include "AetherAudioSettingsLibrary.h"

#include "AetherSettingsSubsystem.h"
#include "Components/AudioComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

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
