/** 컷신 트리거, 디자이너가 조정하는 연출 정의와 UI에 전달할 실행 상태를 담는다. 시퀀스 참조는 필요할 때 연결할 수 있는 소프트 참조다. */
#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "AetherCinematicTypes.generated.h"

/** 게임 진행 사건과 컷신 정의를 연결하는 트리거 종류다. */
UENUM(BlueprintType)
enum class EAetherCinematicTrigger : uint8
{
	GameIntro,
	BossIntro,
	BossDefeated,
	Custom
};

/** 컷신 자산과 잠금·건너뛰기·대체 종료 조건을 디자이너가 설정한다. */
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

/** 현재 컷신의 활성 상태와 요청 정보를 외부 표시 계층에 전달한다. */
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
