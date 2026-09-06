#pragma once

#include "CoreMinimal.h"
#include "AetherSettingsTypes.h"
#include "Blueprint/UserWidget.h"
#include "AetherMainMenuWidget.generated.h"

class UBorder;
class UButton;
class UCheckBox;
class UComboBoxString;
class UHorizontalBox;
class UScrollBox;
class USpinBox;
class UTextBlock;
class UVerticalBox;
class UWidgetSwitcher;
enum class EAetherMenuFlowState : uint8;
#if !UE_BUILD_SHIPPING
struct FAetherMainMenuWidgetValidationAccess;
struct FAetherPauseMenuRuntimeValidationAccess;
#endif

/** 같은 메뉴 위젯이 메인 화면과 일시 정지 화면의 동작을 구분하도록 한다. */
UENUM(BlueprintType)
enum class EAetherMenuContext : uint8
{
	MainMenu,
	PauseMenu
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAetherMenuResumeRequestedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAetherMenuReturnRequestedSignature);

/** 메인 메뉴와 일시 정지 메뉴가 공유하는 UMG 화면을 구성하고 저장 선택·설정 편집·확인 팝업을 서브시스템에 연결한다. */
UCLASS(Blueprintable)
class AETHERFALL_API UAetherMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ConfigureMenuContext(EAetherMenuContext InContext) { MenuContext = InContext; }
	void FocusPrimaryAction();
	void RestorePauseMenuAfterTransitionFailure(const FText& Message);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Menu")
	EAetherMenuContext GetMenuContext() const { return MenuContext; }

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Menu")
	FAetherMenuResumeRequestedSignature OnResumeRequested;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Menu")
	FAetherMenuReturnRequestedSignature OnReturnToMainMenuRequested;

protected:
	/** 기존 루트 위젯이 없을 때만 C++ 기본 위젯 트리를 구성한다. */
	virtual TSharedRef<SWidget> RebuildWidget() override;
	/** 화면 생성 후 동작을 연결하고 저장 상태와 초기 포커스를 갱신하며 흐름·설정 이벤트를 구독한다. */
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
#if !UE_BUILD_SHIPPING
	friend struct FAetherMainMenuWidgetValidationAccess;
	friend struct FAetherPauseMenuRuntimeValidationAccess;
#endif

	enum class EPopupAction : uint8
	{
		None,
		OverwriteProgress,
		QuitGame,
		ReturnToMainMenu,
		VideoConfirmation,
		Information
	};

	/** 메뉴 문맥에 맞춰 탐색 버튼, 콘텐츠 전환기, 설정 제어 및 팝업·로딩 레이어를 생성한다. */
	void BuildWidgetTree();
	UButton* BuildButton(const FText& Label, UVerticalBox* Parent, bool bPrimary = false);
	UTextBlock* BuildText(const FText& Text, int32 Size, const FLinearColor& Color, bool bWrap = false);
	UHorizontalBox* BuildSettingRow(UVerticalBox* Parent, const FText& Label, UWidget* Control);
	UComboBoxString* BuildComboBox(const TArray<FString>& Options);
	USpinBox* BuildSpinBox(float MinValue, float MaxValue, float Delta, float MinFractionalDigits = 0.0f);
	UCheckBox* BuildCheckBox();
	UVerticalBox* BuildSettingsCategory(const FText& Heading);
	/** 중복 연결을 막고 버튼·품질 선택 이벤트를 각 처리 함수에 연결한다. */
	void BindActions();
	/** 슬롯 존재 여부와 실제 로드 가능 여부를 구분해 계속하기·불러오기 버튼 및 저장 요약을 갱신한다. */
	void RefreshSaveState();
	void ShowMainScreen();
	void ShowLoadScreen();
	/** 설정 편집 세션을 시작하고 보류 스냅샷을 화면 제어값에 복사한다. */
	void ShowSettingsScreen();
	void ShowCreditsScreen();
	void ShowSettingsCategory(int32 CategoryIndex);
	void ShowPopup(EPopupAction Action, const FText& Title, const FText& Message, const FText& ConfirmLabel, bool bShowCancel);
	void HidePopup();
	void SetLoadingState(bool bLoading, const FText& Message);
	/** 보류 설정을 UI에 반영하며 현재 해상도·프레임 제한값이 목록에 없으면 추가한다. */
	void SynchronizeSettingsControls();
	/** UI 제어값을 설정 스냅샷으로 모으고 백분율 표시 음량·화면 흔들림을 저장 단위로 환산한다. */
	FAetherSettingsSnapshot CollectSettingsControls() const;
	void ApplyOverallQualitySelection(int32 QualityLevel);
	/** 로딩·팝업·설정 편집 상태에 따라 뒤로 가기를 처리하고, 표시 설정 확인 중에는 변경을 되돌린다. */
	void HandleBack();

	UFUNCTION()
	void HandleContinueClicked();

	/** 기존 진행 슬롯이 있으면 덮어쓰기 팝업을 거치고, 없으면 새 게임을 직접 요청한다. */
	UFUNCTION()
	void HandleNewGameClicked();

	UFUNCTION()
	void HandleLoadClicked();

	UFUNCTION()
	void HandleLoadSelectedClicked();

	UFUNCTION()
	void HandleSettingsClicked();

	UFUNCTION()
	void HandleCreditsClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UFUNCTION()
	void HandleBackClicked();

	/** 현재 팝업의 목적에 따라 새 게임·종료·메뉴 복귀·표시 설정 확정을 수행한다. */
	UFUNCTION()
	void HandlePopupConfirmClicked();

	UFUNCTION()
	void HandlePopupCancelClicked();

	/** 화면 값을 보류 설정에 반영하고 표시 모드 변경이 있으면 유지 여부를 묻는 팝업을 연다. */
	UFUNCTION()
	void HandleSettingsApplyClicked();

	UFUNCTION()
	void HandleSettingsCancelClicked();

	UFUNCTION()
	void HandleSettingsDefaultsClicked();

	UFUNCTION()
	void HandleDisplayTabClicked();

	UFUNCTION()
	void HandleGraphicsTabClicked();

	UFUNCTION()
	void HandleAudioTabClicked();

	UFUNCTION()
	void HandleGameplayTabClicked();

	UFUNCTION()
	UWidget* HandleGenerateComboWidget(FString Item);

	/** 사용자가 변경한 선택만 프리셋으로 적용하여 프로그램에 의한 값 동기화가 재진입하지 않게 한다. */
	UFUNCTION()
	void HandleOverallQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleIndividualQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	/** 맵 전환 동안 입력을 잠그고 실패 시 입력을 복원해 오류 내용을 표시한다. */
	UFUNCTION()
	void HandleMenuFlowChanged(EAetherMenuFlowState State, FText Message);

	UFUNCTION()
	void HandleVideoConfirmationChanged(bool bAwaitingConfirmation);

	UPROPERTY(Transient)
	TObjectPtr<UWidgetSwitcher> ContentSwitcher;

	UPROPERTY(Transient)
	TObjectPtr<UWidgetSwitcher> SettingsCategorySwitcher;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> PopupOverlay;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> LoadingOverlay;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PopupTitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PopupMessageText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PopupConfirmText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LoadingMessageText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SaveSummaryText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ContinueButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> NewGameButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> LoadButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CreditsButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> LoadSelectedButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PopupConfirmButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> PopupCancelButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> LoadBackButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> CreditsBackButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SettingsApplyButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SettingsCancelButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> SettingsDefaultsButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> DisplayTabButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> GraphicsTabButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> AudioTabButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> GameplayTabButton;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> WindowModeCombo;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> ResolutionCombo;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> VSyncCheckBox;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> FrameLimitCombo;

	UPROPERTY(Transient)
	TObjectPtr<USpinBox> ResolutionScaleSpinBox;

	UPROPERTY(Transient)
	TObjectPtr<USpinBox> GammaSpinBox;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> OverallQualityCombo;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UComboBoxString>> IndividualQualityCombos;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> MotionBlurCheckBox;

	UPROPERTY(Transient)
	TObjectPtr<USpinBox> MasterVolumeSpinBox;

	UPROPERTY(Transient)
	TObjectPtr<USpinBox> MusicVolumeSpinBox;

	UPROPERTY(Transient)
	TObjectPtr<USpinBox> SfxVolumeSpinBox;

	UPROPERTY(Transient)
	TObjectPtr<USpinBox> VoiceVolumeSpinBox;

	UPROPERTY(Transient)
	TObjectPtr<USpinBox> UiVolumeSpinBox;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> MuteCheckBox;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> SubtitlesCheckBox;

	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> SubtitleSizeCombo;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> DialogueAutoAdvanceCheckBox;

	UPROPERTY(Transient)
	TObjectPtr<USpinBox> CameraSensitivityXSpinBox;

	UPROPERTY(Transient)
	TObjectPtr<USpinBox> CameraSensitivityYSpinBox;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> InvertCameraYCheckBox;

	UPROPERTY(Transient)
	TObjectPtr<UCheckBox> VibrationCheckBox;

	UPROPERTY(Transient)
	TObjectPtr<USpinBox> ScreenShakeSpinBox;

	EPopupAction ActivePopupAction = EPopupAction::None;
	EAetherMenuContext MenuContext = EAetherMenuContext::MainMenu;
	int32 ActiveContentIndex = 0;
	bool bActionsBound = false;
};
