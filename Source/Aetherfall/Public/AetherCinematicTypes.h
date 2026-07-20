#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "AetherCinematicTypes.generated.h"

UENUM(BlueprintType)
enum class EAetherCinematicTrigger : uint8
{
	GameIntro,
	BossIntro,
	BossDefeated,
	Custom
};

USTRUCT(BlueprintType)
struct FAetherCinematicDefinition
{
	GENERATED_BODY()

	FAetherCinematicDefinition()
		: DisplayName(NSLOCTEXT("AetherCinematic", "DefaultDisplayName", "Cinematic"))
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic")
	EAetherCinematicTrigger Trigger = EAetherCinematicTrigger::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic")
	FName EventLabel = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic", meta = (AllowedClasses = "/Script/LevelSequence.LevelSequence"))
	TSoftObjectPtr<UObject> SequenceAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic")
	bool bSkippable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic")
	bool bLockPlayerInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic")
	bool bLockCameraControl = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic")
	bool bHideHud = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic")
	bool bUseFade = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic", meta = (ClampMin = "0.0"))
	float FadeInTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic", meta = (ClampMin = "0.0"))
	float FadeOutTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic", meta = (ClampMin = "0.0"))
	float FallbackDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Cinematic")
	bool bAutoFinishWhenNoSequenceAsset = true;
};

USTRUCT(BlueprintType)
struct FAetherCinematicRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Cinematic")
	bool bActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Cinematic")
	bool bSkipped = false;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Cinematic")
	FAetherCinematicDefinition Definition;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Cinematic")
	float ElapsedSeconds = 0.0f;
};
