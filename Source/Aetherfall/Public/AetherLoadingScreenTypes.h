#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"
#include "AetherLoadingScreenTypes.generated.h"

class UAetherLoadingScreenWidget;
class UTexture2D;

UENUM(BlueprintType)
enum class EAetherLoadingScreenState : uint8
{
	Hidden,
	FadingIn,
	Holding,
	FadingOut
};

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
