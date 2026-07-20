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

UENUM(BlueprintType)
enum class EAetherMenuContext : uint8
{
	MainMenu,
	PauseMenu
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAetherMenuResumeRequestedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAetherMenuReturnRequestedSignature);

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
	virtual TSharedRef<SWidget> RebuildWidget() override;
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

	void BuildWidgetTree();
	UButton* BuildButton(const FText& Label, UVerticalBox* Parent, bool bPrimary = false);
	UTextBlock* BuildText(const FText& Text, int32 Size, const FLinearColor& Color, bool bWrap = false);
	UHorizontalBox* BuildSettingRow(UVerticalBox* Parent, const FText& Label, UWidget* Control);
	UComboBoxString* BuildComboBox(const TArray<FString>& Options);
	USpinBox* BuildSpinBox(float MinValue, float MaxValue, float Delta, float MinFractionalDigits = 0.0f);
	UCheckBox* BuildCheckBox();
	UVerticalBox* BuildSettingsCategory(const FText& Heading);
	void BindActions();
	void RefreshSaveState();
	void ShowMainScreen();
	void ShowLoadScreen();
	void ShowSettingsScreen();
	void ShowCreditsScreen();
	void ShowSettingsCategory(int32 CategoryIndex);
	void ShowPopup(EPopupAction Action, const FText& Title, const FText& Message, const FText& ConfirmLabel, bool bShowCancel);
	void HidePopup();
	void SetLoadingState(bool bLoading, const FText& Message);
	void SynchronizeSettingsControls();
	FAetherSettingsSnapshot CollectSettingsControls() const;
	void ApplyOverallQualitySelection(int32 QualityLevel);
	void HandleBack();

	UFUNCTION()
	void HandleContinueClicked();

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

	UFUNCTION()
	void HandlePopupConfirmClicked();

	UFUNCTION()
	void HandlePopupCancelClicked();

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

	UFUNCTION()
	void HandleOverallQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleIndividualQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

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
