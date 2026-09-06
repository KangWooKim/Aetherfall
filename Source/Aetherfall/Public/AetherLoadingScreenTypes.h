/** 로딩 화면 단계와 편집 가능한 표시 설정, UI에 전달할 읽기 전용 표시 모델을 정의한다. */
#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"
#include "AetherLoadingScreenTypes.generated.h"

class UAetherLoadingScreenWidget;
class UTexture2D;

/** 로딩 표시의 등장·유지·퇴장 단계를 구분한다. */
UENUM(BlueprintType)
enum class EAetherLoadingScreenState : uint8
{
	Hidden,
	FadingIn,
	Holding,
	FadingOut
};

/** 로딩 화면의 표시 시간과 페이드·문구 등을 조정한다. */
USTRUCT(BlueprintType)
struct FAetherLoadingScreenSettings
{
	GENERATED_BODY()

	FAetherLoadingScreenSettings()
		: LoadingText(NSLOCTEXT("AetherLoadingScreen", "DefaultLoadingText", "Loading Aetherfall..."))
	{
		LoadingTips.Add(NSLOCTEXT("AetherLoadingScreen", "DefaultTip", "Let the world settle before the blade is drawn."));
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen")
	bool bEnableViewportLoadingScreen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen")
	TSubclassOf<UAetherLoadingScreenWidget> LoadingWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen")
	TSoftClassPtr<UAetherLoadingScreenWidget> LoadingWidgetBlueprintClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen")
	TSoftObjectPtr<UTexture2D> BackgroundImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen")
	FLinearColor BackgroundColor = FLinearColor(0.004f, 0.007f, 0.010f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen")
	FLinearColor AccentColor = FLinearColor(0.38f, 0.78f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen")
	FText LoadingText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen")
	TArray<FText> LoadingTips;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen", meta = (ClampMin = "0.0"))
	float MinimumDisplayTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen", meta = (ClampMin = "0.0"))
	float PostLoadHoldTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen", meta = (ClampMin = "0.0"))
	float FadeInTime = 0.12f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen", meta = (ClampMin = "0.0"))
	float FadeOutTime = 0.28f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aetherfall|Loading Screen")
	int32 ViewportZOrder = 10000;
};

/** 현재 로딩 상태를 화면 위젯이 그릴 값으로 전달한다. */
USTRUCT(BlueprintType)
struct FAetherLoadingScreenViewModel
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Loading Screen")
	EAetherLoadingScreenState State = EAetherLoadingScreenState::Hidden;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Loading Screen")
	FText LoadingText;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Loading Screen")
	FText LoadingTip;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Loading Screen")
	float Opacity = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Loading Screen")
	bool bWaitingForMapLoad = false;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Loading Screen")
	bool bWaitingForGameplayReady = false;
};
