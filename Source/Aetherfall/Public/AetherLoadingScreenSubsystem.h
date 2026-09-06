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

/** 맵 전환과 게임 준비 알림을 받아 로딩 화면의 표시·유지·페이드를 관리하고 Slate 및 선택적 위젯에 표시 모델을 전달한다. */
UCLASS()
class AETHERFALL_API UAetherLoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	/** 맵 전환 델리게이트 등록을 해제하고 남은 로딩 표시와 틱커를 정리한다. */
	virtual void Deinitialize() override;

	/** 새 요청의 시간·대기 조건을 초기화하고 기본 화면과 선택적 위젯을 생성한 뒤 코어 틱커를 등록한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Loading Screen")
	void BeginLoadingScreen(const FText& LoadingMessage, bool bWaitForGameplayReady);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Loading Screen")
	void NotifyMapLoadCompleted();

	/** 게임 모드가 준비되었음을 기록하고 맵 완료 알림이 아직 없으면 완료 기준 시간도 함께 채운다. */
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
	/** 실제 경과 시간으로 페이드를 진행하며 최소 표시 시간, 맵 로드 후 대기, 게임 준비 조건이 모두 충족되면 숨김을 시작한다. */
	bool TickLoadingScreen(float DeltaTime);
	void ShowViewportLoadingScreen();
	void RemoveViewportLoadingScreen();
	/** 명시 클래스 또는 소프트 클래스 참조를 해석하고 플레이어 컨트롤러가 있을 때 확장용 위젯을 생성한다. */
	void TryCreateLoadingWidget(UWorld* World);
	void RemoveLoadingWidget();
	void StartFadeOut();
	/** 상태를 숨김으로 먼저 바꾸고 위젯·뷰포트 콘텐츠·틱커를 정리한 뒤 숨김 이벤트를 발행한다. */
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
