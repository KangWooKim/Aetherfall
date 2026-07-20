#pragma once

#include "CoreMinimal.h"
#include "AetherLoadingScreenTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Styling/SlateBrush.h"
#include "AetherLoadingScreenSubsystem.generated.h"

class SBorder;
class SImage;
class STextBlock;
class SWidget;
class UAetherLoadingScreenWidget;
class UWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherLoadingScreenChangedSignature, const FAetherLoadingScreenViewModel&, ViewModel);

UCLASS()
class AETHERFALL_API UAetherLoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Loading Screen")
	void BeginLoadingScreen(const FText& LoadingMessage, bool bWaitForGameplayReady);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Loading Screen")
	void NotifyMapLoadCompleted();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Loading Screen")
	void NotifyGameplayWorldReady();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Loading Screen")
	void RequestHideLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Loading Screen")
	void SetLoadingScreenSettings(const FAetherLoadingScreenSettings& InSettings);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Loading Screen")
	const FAetherLoadingScreenSettings& GetLoadingScreenSettings() const { return ActiveSettings; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Loading Screen")
	EAetherLoadingScreenState GetLoadingScreenState() const { return LoadingState; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Loading Screen")
	bool IsLoadingScreenVisible() const { return LoadingState != EAetherLoadingScreenState::Hidden; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Loading Screen")
	FAetherLoadingScreenViewModel GetCurrentViewModel() const;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Loading Screen")
	FAetherLoadingScreenChangedSignature OnLoadingScreenShown;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Loading Screen")
	FAetherLoadingScreenChangedSignature OnLoadingScreenUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Loading Screen")
	FAetherLoadingScreenChangedSignature OnLoadingScreenHidden;

private:
	void HandlePreLoadMap(const FString& MapName);
	void HandlePostLoadMap(UWorld* LoadedWorld);
	bool TickLoadingScreen(float DeltaTime);
	void ShowViewportLoadingScreen();
	void RemoveViewportLoadingScreen();
	void TryCreateLoadingWidget(UWorld* World);
	void RemoveLoadingWidget();
	void StartFadeOut();
	void CompleteHide();
	void EnsureTicker();
	void RemoveTicker();
	void RefreshVisuals();
	void ApplyOpacity(float NewOpacity);
	FText ChooseLoadingTip() const;
	UWorld* ResolveRuntimeWorld() const;

	FAetherLoadingScreenSettings ActiveSettings;
	EAetherLoadingScreenState LoadingState = EAetherLoadingScreenState::Hidden;
	FText ActiveLoadingText;
	FText ActiveLoadingTip;
	float CurrentOpacity = 0.0f;
	double DisplayStartTime = 0.0;
	double MapLoadCompletedTime = 0.0;
	double FadeStartTime = 0.0;
	bool bMapLoadCompleted = false;
	bool bRequireGameplayReady = false;
	bool bGameplayReady = false;

	FDelegateHandle TickerHandle;
	TSharedPtr<SWidget> ViewportLoadingWidget;
	TSharedPtr<SBorder> LoadingRootBorder;
	TSharedPtr<SImage> LoadingBackgroundImage;
	TSharedPtr<STextBlock> LoadingTextBlock;
	TSharedPtr<STextBlock> LoadingTipTextBlock;
	TSharedPtr<FSlateBrush> BackgroundBrush;

	UPROPERTY(Transient)
	TObjectPtr<UAetherLoadingScreenWidget> LoadingWidget;
};
