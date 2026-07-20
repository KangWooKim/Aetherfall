#include "AetherMainMenuWidget.h"

#include "AetherMenuFlowSubsystem.h"
#include "AetherSaveSubsystem.h"
#include "AetherSettingsSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/SpinBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Engine/GameInstance.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

namespace
{
	const FLinearColor AetherGold(0.78f, 0.60f, 0.24f, 1.0f);
	const FLinearColor AetherIvory(0.90f, 0.88f, 0.82f, 1.0f);
	const FLinearColor AetherMuted(0.56f, 0.58f, 0.60f, 1.0f);
	const FLinearColor AetherPanel(0.018f, 0.024f, 0.030f, 0.78f);
	const FLinearColor AetherButton(0.08f, 0.095f, 0.11f, 0.96f);
	const FLinearColor AetherControl(0.045f, 0.055f, 0.065f, 1.0f);
	const FLinearColor AetherControlHover(0.13f, 0.11f, 0.065f, 1.0f);
	const FLinearColor AetherControlPressed(0.20f, 0.15f, 0.07f, 1.0f);

	FString FormatResolution(const FIntPoint& Resolution)
	{
		return FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y);
	}

	FIntPoint ParseResolution(const FString& Value, const FIntPoint& Fallback)
	{
		FString Left;
		FString Right;
		if (!Value.Split(TEXT("x"), &Left, &Right))
		{
			return Fallback;
		}
		return FIntPoint(FCString::Atoi(*Left.TrimStartAndEnd()), FCString::Atoi(*Right.TrimStartAndEnd()));
	}

	int32 QualityIndex(const UComboBoxString* Combo)
	{
		return Combo ? FMath::Max(0, Combo->GetSelectedIndex()) : 3;
	}
}

TSharedRef<SWidget> UAetherMainMenuWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildWidgetTree();
	}
	return Super::RebuildWidget();
}

void UAetherMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	BindActions();
	RefreshSaveState();
	ShowMainScreen();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UAetherMenuFlowSubsystem* Flow = GameInstance->GetSubsystem<UAetherMenuFlowSubsystem>())
		{
			Flow->OnMenuFlowChanged.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleMenuFlowChanged);
		}
		if (UAetherSettingsSubsystem* Settings = GameInstance->GetSubsystem<UAetherSettingsSubsystem>())
		{
			Settings->OnVideoConfirmationChanged.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleVideoConfirmationChanged);
		}
	}
}

void UAetherMainMenuWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (ActivePopupAction != EPopupAction::VideoConfirmation || !PopupMessageText)
	{
		return;
	}

	const UAetherSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr;
	if (Settings && Settings->IsAwaitingVideoConfirmation())
	{
		PopupMessageText->SetText(FText::Format(
			NSLOCTEXT("AetherMenu", "VideoConfirmCountdown", "Keep these display settings?\nReverting in {0} seconds."),
			FText::AsNumber(FMath::CeilToInt(Settings->GetVideoConfirmationSecondsRemaining()))));
	}
}

FReply UAetherMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Gamepad_Special_Right)
	{
		HandleBack();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UAetherMainMenuWidget::BuildWidgetTree()
{
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UBorder* ScreenTint = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ScreenTint"));
	ScreenTint->SetBrushColor(FLinearColor(0.005f, 0.008f, 0.012f, 0.60f));
	Root->AddChildToOverlay(ScreenTint);

	UHorizontalBox* Shell = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MenuShell"));
	if (UOverlaySlot* ShellSlot = Root->AddChildToOverlay(Shell))
	{
		ShellSlot->SetPadding(FMargin(64.0f, 48.0f));
		ShellSlot->SetHorizontalAlignment(HAlign_Fill);
		ShellSlot->SetVerticalAlignment(VAlign_Fill);
	}

	USizeBox* NavigationSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("NavigationSize"));
	NavigationSize->SetWidthOverride(420.0f);
	UVerticalBox* NavigationPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Navigation"));
	NavigationSize->SetContent(NavigationPanel);
	if (UHorizontalBoxSlot* NavigationSlot = Shell->AddChildToHorizontalBox(NavigationSize))
	{
		NavigationSlot->SetPadding(FMargin(0.0f, 0.0f, 48.0f, 0.0f));
		NavigationSlot->SetVerticalAlignment(VAlign_Fill);
	}

	const bool bPauseMenu = MenuContext == EAetherMenuContext::PauseMenu;
	NavigationPanel->AddChildToVerticalBox(BuildText(NSLOCTEXT("AetherMenu", "Title", "AETHERFALL"), 48, AetherIvory));
	NavigationPanel->AddChildToVerticalBox(BuildText(
		bPauseMenu ? NSLOCTEXT("AetherMenu", "PauseTitleRule", "JOURNEY PAUSED") : NSLOCTEXT("AetherMenu", "TitleRule", "THE LAST RESONANCE"),
		13,
		AetherGold));
	USpacer* TitleSpace = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
	TitleSpace->SetSize(FVector2D(1.0f, 72.0f));
	NavigationPanel->AddChildToVerticalBox(TitleSpace);

	ContinueButton = BuildButton(
		bPauseMenu ? NSLOCTEXT("AetherMenu", "Resume", "RESUME") : NSLOCTEXT("AetherMenu", "Continue", "CONTINUE"),
		NavigationPanel,
		true);
	NewGameButton = BuildButton(NSLOCTEXT("AetherMenu", "NewGame", "NEW GAME"), NavigationPanel);
	LoadButton = BuildButton(NSLOCTEXT("AetherMenu", "Load", "LOAD"), NavigationPanel);
	SettingsButton = BuildButton(NSLOCTEXT("AetherMenu", "Settings", "SETTINGS"), NavigationPanel);
	CreditsButton = BuildButton(NSLOCTEXT("AetherMenu", "Credits", "CREDITS"), NavigationPanel);
	QuitButton = BuildButton(
		bPauseMenu ? NSLOCTEXT("AetherMenu", "ReturnToMenu", "RETURN TO MAIN MENU") : NSLOCTEXT("AetherMenu", "Quit", "QUIT"),
		NavigationPanel);
	if (bPauseMenu)
	{
		NewGameButton->SetVisibility(ESlateVisibility::Collapsed);
		LoadButton->SetVisibility(ESlateVisibility::Collapsed);
		CreditsButton->SetVisibility(ESlateVisibility::Collapsed);
		ContinueButton->SetNavigationRuleExplicit(EUINavigation::Down, SettingsButton);
		SettingsButton->SetNavigationRuleExplicit(EUINavigation::Up, ContinueButton);
		SettingsButton->SetNavigationRuleExplicit(EUINavigation::Down, QuitButton);
		QuitButton->SetNavigationRuleExplicit(EUINavigation::Up, SettingsButton);
	}

	UBorder* ContentPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ContentPanel"));
	ContentPanel->SetBrushColor(AetherPanel);
	ContentPanel->SetPadding(FMargin(40.0f));
	if (UHorizontalBoxSlot* ContentSlot = Shell->AddChildToHorizontalBox(ContentPanel))
	{
		ContentSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		ContentSlot->SetVerticalAlignment(VAlign_Fill);
	}

	ContentSwitcher = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("ContentSwitcher"));
	ContentPanel->SetContent(ContentSwitcher);

	UVerticalBox* MainScreen = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainScreen"));
	MainScreen->AddChildToVerticalBox(BuildText(
		bPauseMenu ? NSLOCTEXT("AetherMenu", "PauseHeading", "THE WORLD HOLDS ITS BREATH") : NSLOCTEXT("AetherMenu", "MainHeading", "THE VEIL IS THINNING"),
		28,
		AetherIvory));
	MainScreen->AddChildToVerticalBox(BuildText(
		bPauseMenu
			? NSLOCTEXT("AetherMenu", "PauseCopy", "Resume the journey, adjust your settings, or return to the title.")
			: NSLOCTEXT("AetherMenu", "MainCopy", "Beyond the fallen hamlet, the Cathedral of Aether waits in silence."),
		16,
		AetherMuted,
		true));
	USpacer* MainScreenFill = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
	if (UVerticalBoxSlot* FillSlot = MainScreen->AddChildToVerticalBox(MainScreenFill))
	{
		FillSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	MainScreen->AddChildToVerticalBox(BuildText(NSLOCTEXT("AetherMenu", "RouteHeading", "THE LAST RESONANCE"), 13, AetherGold));
	MainScreen->AddChildToVerticalBox(BuildText(
		NSLOCTEXT("AetherMenu", "RouteCopy", "FALLEN HAMLET  /  BROKEN WALL  /  LOWER CRYPT  /  CATHEDRAL OF AETHER"),
		13,
		AetherMuted,
		true));
	ContentSwitcher->AddChild(MainScreen);

	UVerticalBox* LoadScreen = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("LoadScreen"));
	LoadScreen->AddChildToVerticalBox(BuildText(NSLOCTEXT("AetherMenu", "LoadHeading", "RECORDED JOURNEY"), 28, AetherIvory));
	SaveSummaryText = BuildText(FText::GetEmpty(), 16, AetherMuted, true);
	if (UVerticalBoxSlot* SummarySlot = LoadScreen->AddChildToVerticalBox(SaveSummaryText))
	{
		SummarySlot->SetPadding(FMargin(0.0f, 24.0f, 0.0f, 32.0f));
	}
	LoadSelectedButton = BuildButton(NSLOCTEXT("AetherMenu", "LoadSelected", "LOAD JOURNEY"), LoadScreen, true);
	LoadBackButton = BuildButton(NSLOCTEXT("AetherMenu", "Back", "BACK"), LoadScreen);
	ContentSwitcher->AddChild(LoadScreen);

	UVerticalBox* SettingsScreen = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SettingsScreen"));
	SettingsScreen->AddChildToVerticalBox(BuildText(NSLOCTEXT("AetherMenu", "SettingsHeading", "SETTINGS"), 28, AetherIvory));
	UHorizontalBox* Tabs = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SettingsTabs"));
	if (UVerticalBoxSlot* TabsSlot = SettingsScreen->AddChildToVerticalBox(Tabs))
	{
		TabsSlot->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 16.0f));
	}
	UVerticalBox* DisplayTabHolder = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	DisplayTabButton = BuildButton(NSLOCTEXT("AetherMenu", "DisplayTab", "DISPLAY"), DisplayTabHolder);
	Tabs->AddChildToHorizontalBox(DisplayTabHolder);
	UVerticalBox* GraphicsTabHolder = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	GraphicsTabButton = BuildButton(NSLOCTEXT("AetherMenu", "GraphicsTab", "GRAPHICS"), GraphicsTabHolder);
	Tabs->AddChildToHorizontalBox(GraphicsTabHolder);
	UVerticalBox* AudioTabHolder = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	AudioTabButton = BuildButton(NSLOCTEXT("AetherMenu", "AudioTab", "AUDIO"), AudioTabHolder);
	Tabs->AddChildToHorizontalBox(AudioTabHolder);
	UVerticalBox* GameplayTabHolder = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	GameplayTabButton = BuildButton(NSLOCTEXT("AetherMenu", "GameplayTab", "GAMEPLAY"), GameplayTabHolder);
	Tabs->AddChildToHorizontalBox(GameplayTabHolder);

	SettingsCategorySwitcher = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("SettingsCategories"));
	if (UVerticalBoxSlot* CategorySlot = SettingsScreen->AddChildToVerticalBox(SettingsCategorySwitcher))
	{
		CategorySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

	UVerticalBox* DisplayCategory = BuildSettingsCategory(NSLOCTEXT("AetherMenu", "DisplayCategory", "DISPLAY"));
	WindowModeCombo = BuildComboBox({TEXT("Fullscreen"), TEXT("Borderless"), TEXT("Windowed")});
	BuildSettingRow(DisplayCategory, NSLOCTEXT("AetherMenu", "WindowMode", "Window Mode"), WindowModeCombo);
	ResolutionCombo = BuildComboBox({});
	BuildSettingRow(DisplayCategory, NSLOCTEXT("AetherMenu", "Resolution", "Resolution"), ResolutionCombo);
	VSyncCheckBox = BuildCheckBox();
	BuildSettingRow(DisplayCategory, NSLOCTEXT("AetherMenu", "VSync", "Vertical Sync"), VSyncCheckBox);
	FrameLimitCombo = BuildComboBox({TEXT("Unlimited"), TEXT("30"), TEXT("60"), TEXT("90"), TEXT("120"), TEXT("144"), TEXT("165"), TEXT("240")});
	BuildSettingRow(DisplayCategory, NSLOCTEXT("AetherMenu", "FrameLimit", "Frame Limit"), FrameLimitCombo);
	ResolutionScaleSpinBox = BuildSpinBox(25.0f, 100.0f, 1.0f);
	BuildSettingRow(DisplayCategory, NSLOCTEXT("AetherMenu", "ResolutionScale", "Resolution Scale (%)"), ResolutionScaleSpinBox);
	GammaSpinBox = BuildSpinBox(1.6f, 2.8f, 0.05f, 2.0f);
	BuildSettingRow(DisplayCategory, NSLOCTEXT("AetherMenu", "Gamma", "Gamma"), GammaSpinBox);

	UVerticalBox* GraphicsCategory = BuildSettingsCategory(NSLOCTEXT("AetherMenu", "GraphicsCategory", "GRAPHICS"));
	OverallQualityCombo = BuildComboBox({TEXT("Custom"), TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic"), TEXT("Cinematic")});
	BuildSettingRow(GraphicsCategory, NSLOCTEXT("AetherMenu", "OverallQuality", "Overall Quality"), OverallQualityCombo);
	const TArray<FText> QualityLabels = {
		NSLOCTEXT("AetherMenu", "ViewDistance", "View Distance"),
		NSLOCTEXT("AetherMenu", "AntiAliasing", "Anti-Aliasing"),
		NSLOCTEXT("AetherMenu", "Shadows", "Shadows"),
		NSLOCTEXT("AetherMenu", "GlobalIllumination", "Global Illumination"),
		NSLOCTEXT("AetherMenu", "Reflections", "Reflections"),
		NSLOCTEXT("AetherMenu", "Textures", "Textures"),
		NSLOCTEXT("AetherMenu", "Effects", "Effects"),
		NSLOCTEXT("AetherMenu", "PostProcessing", "Post Processing"),
		NSLOCTEXT("AetherMenu", "Foliage", "Foliage"),
		NSLOCTEXT("AetherMenu", "Shading", "Shading")};
	for (const FText& Label : QualityLabels)
	{
		UComboBoxString* Combo = BuildComboBox({TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic"), TEXT("Cinematic")});
		IndividualQualityCombos.Add(Combo);
		BuildSettingRow(GraphicsCategory, Label, Combo);
	}
	MotionBlurCheckBox = BuildCheckBox();
	BuildSettingRow(GraphicsCategory, NSLOCTEXT("AetherMenu", "MotionBlur", "Motion Blur"), MotionBlurCheckBox);

	UVerticalBox* AudioCategory = BuildSettingsCategory(NSLOCTEXT("AetherMenu", "AudioCategory", "AUDIO"));
	MasterVolumeSpinBox = BuildSpinBox(0.0f, 100.0f, 1.0f);
	MusicVolumeSpinBox = BuildSpinBox(0.0f, 100.0f, 1.0f);
	SfxVolumeSpinBox = BuildSpinBox(0.0f, 100.0f, 1.0f);
	VoiceVolumeSpinBox = BuildSpinBox(0.0f, 100.0f, 1.0f);
	UiVolumeSpinBox = BuildSpinBox(0.0f, 100.0f, 1.0f);
	MuteCheckBox = BuildCheckBox();
	BuildSettingRow(AudioCategory, NSLOCTEXT("AetherMenu", "MasterVolume", "Master"), MasterVolumeSpinBox);
	BuildSettingRow(AudioCategory, NSLOCTEXT("AetherMenu", "MusicVolume", "BGM"), MusicVolumeSpinBox);
	BuildSettingRow(AudioCategory, NSLOCTEXT("AetherMenu", "SfxVolume", "SFX"), SfxVolumeSpinBox);
	BuildSettingRow(AudioCategory, NSLOCTEXT("AetherMenu", "VoiceVolume", "Voice / TTS"), VoiceVolumeSpinBox);
	BuildSettingRow(AudioCategory, NSLOCTEXT("AetherMenu", "UiVolume", "UI"), UiVolumeSpinBox);
	BuildSettingRow(AudioCategory, NSLOCTEXT("AetherMenu", "MuteAll", "Mute All"), MuteCheckBox);

	UVerticalBox* GameplayCategory = BuildSettingsCategory(NSLOCTEXT("AetherMenu", "GameplayCategory", "GAMEPLAY & ACCESSIBILITY"));
	SubtitlesCheckBox = BuildCheckBox();
	SubtitleSizeCombo = BuildComboBox({TEXT("Small"), TEXT("Medium"), TEXT("Large")});
	DialogueAutoAdvanceCheckBox = BuildCheckBox();
	CameraSensitivityXSpinBox = BuildSpinBox(0.1f, 4.0f, 0.1f, 1.0f);
	CameraSensitivityYSpinBox = BuildSpinBox(0.1f, 4.0f, 0.1f, 1.0f);
	InvertCameraYCheckBox = BuildCheckBox();
	VibrationCheckBox = BuildCheckBox();
	ScreenShakeSpinBox = BuildSpinBox(0.0f, 100.0f, 5.0f);
	BuildSettingRow(GameplayCategory, NSLOCTEXT("AetherMenu", "Subtitles", "Subtitles"), SubtitlesCheckBox);
	BuildSettingRow(GameplayCategory, NSLOCTEXT("AetherMenu", "SubtitleSize", "Subtitle Size"), SubtitleSizeCombo);
	BuildSettingRow(GameplayCategory, NSLOCTEXT("AetherMenu", "AutoAdvance", "Dialogue Auto Advance"), DialogueAutoAdvanceCheckBox);
	BuildSettingRow(GameplayCategory, NSLOCTEXT("AetherMenu", "SensitivityX", "Camera Sensitivity X"), CameraSensitivityXSpinBox);
	BuildSettingRow(GameplayCategory, NSLOCTEXT("AetherMenu", "SensitivityY", "Camera Sensitivity Y"), CameraSensitivityYSpinBox);
	BuildSettingRow(GameplayCategory, NSLOCTEXT("AetherMenu", "InvertY", "Invert Camera Y"), InvertCameraYCheckBox);
	BuildSettingRow(GameplayCategory, NSLOCTEXT("AetherMenu", "Vibration", "Controller Vibration"), VibrationCheckBox);
	BuildSettingRow(GameplayCategory, NSLOCTEXT("AetherMenu", "ScreenShake", "Screen Shake (%)"), ScreenShakeSpinBox);

	UHorizontalBox* SettingsActions = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("SettingsActions"));
	if (UVerticalBoxSlot* ActionsSlot = SettingsScreen->AddChildToVerticalBox(SettingsActions))
	{
		ActionsSlot->SetPadding(FMargin(0.0f, 16.0f, 0.0f, 0.0f));
	}
	UVerticalBox* DefaultsHolder = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	SettingsDefaultsButton = BuildButton(NSLOCTEXT("AetherMenu", "Defaults", "RESTORE DEFAULTS"), DefaultsHolder);
	SettingsActions->AddChildToHorizontalBox(DefaultsHolder);
	UVerticalBox* CancelHolder = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	SettingsCancelButton = BuildButton(NSLOCTEXT("AetherMenu", "Cancel", "CANCEL"), CancelHolder);
	SettingsActions->AddChildToHorizontalBox(CancelHolder);
	UVerticalBox* ApplyHolder = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	SettingsApplyButton = BuildButton(NSLOCTEXT("AetherMenu", "Apply", "APPLY"), ApplyHolder, true);
	SettingsActions->AddChildToHorizontalBox(ApplyHolder);
	ContentSwitcher->AddChild(SettingsScreen);

	UVerticalBox* CreditsScreen = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CreditsScreen"));
	CreditsScreen->AddChildToVerticalBox(BuildText(NSLOCTEXT("AetherMenu", "CreditsHeading", "CREDITS"), 28, AetherIvory));
	CreditsScreen->AddChildToVerticalBox(BuildText(
		NSLOCTEXT("AetherMenu", "CreditsBody", "AETHERFALL\n\nCreated by Kim DongBeen\n\nBuilt with Unreal Engine 5\nThird-party assets retain their respective licenses."),
		16,
		AetherMuted,
		true));
	CreditsBackButton = BuildButton(NSLOCTEXT("AetherMenu", "CreditsBack", "BACK"), CreditsScreen);
	ContentSwitcher->AddChild(CreditsScreen);

	PopupOverlay = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PopupOverlay"));
	PopupOverlay->SetBrushColor(FLinearColor(0.005f, 0.008f, 0.012f, 0.82f));
	PopupOverlay->SetVisibility(ESlateVisibility::Collapsed);
	if (UOverlaySlot* PopupSlot = Root->AddChildToOverlay(PopupOverlay))
	{
		PopupSlot->SetHorizontalAlignment(HAlign_Fill);
		PopupSlot->SetVerticalAlignment(VAlign_Fill);
	}
	UOverlay* PopupLayer = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	PopupOverlay->SetContent(PopupLayer);
	UBorder* PopupPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	PopupPanel->SetBrushColor(FLinearColor(0.01f, 0.012f, 0.016f, 0.99f));
	PopupPanel->SetPadding(FMargin(36.0f));
	if (UOverlaySlot* PopupPanelSlot = PopupLayer->AddChildToOverlay(PopupPanel))
	{
		PopupPanelSlot->SetHorizontalAlignment(HAlign_Center);
		PopupPanelSlot->SetVerticalAlignment(VAlign_Center);
	}
	USizeBox* PopupSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	PopupSize->SetWidthOverride(520.0f);
	UVerticalBox* PopupContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	PopupSize->SetContent(PopupContent);
	PopupPanel->SetContent(PopupSize);
	PopupTitleText = BuildText(FText::GetEmpty(), 24, AetherIvory);
	PopupMessageText = BuildText(FText::GetEmpty(), 15, AetherMuted, true);
	PopupContent->AddChildToVerticalBox(PopupTitleText);
	if (UVerticalBoxSlot* PopupMessageSlot = PopupContent->AddChildToVerticalBox(PopupMessageText))
	{
		PopupMessageSlot->SetPadding(FMargin(0.0f, 16.0f, 0.0f, 28.0f));
	}
	UHorizontalBox* PopupActions = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	PopupContent->AddChildToVerticalBox(PopupActions);
	UVerticalBox* PopupCancelHolder = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	PopupCancelButton = BuildButton(NSLOCTEXT("AetherMenu", "PopupCancel", "CANCEL"), PopupCancelHolder);
	PopupActions->AddChildToHorizontalBox(PopupCancelHolder);
	UVerticalBox* PopupConfirmHolder = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	PopupConfirmButton = BuildButton(FText::GetEmpty(), PopupConfirmHolder, true);
	PopupConfirmText = Cast<UTextBlock>(PopupConfirmButton->GetContent());
	PopupActions->AddChildToHorizontalBox(PopupConfirmHolder);

	LoadingOverlay = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("LoadingOverlay"));
	LoadingOverlay->SetBrushColor(FLinearColor(0.005f, 0.008f, 0.012f, 0.96f));
	LoadingOverlay->SetVisibility(ESlateVisibility::Collapsed);
	if (UOverlaySlot* LoadingSlot = Root->AddChildToOverlay(LoadingOverlay))
	{
		LoadingSlot->SetHorizontalAlignment(HAlign_Fill);
		LoadingSlot->SetVerticalAlignment(VAlign_Fill);
	}
	LoadingMessageText = BuildText(NSLOCTEXT("AetherMenu", "Loading", "LOADING..."), 20, AetherGold, true);
	LoadingMessageText->SetJustification(ETextJustify::Center);
	LoadingOverlay->SetContent(LoadingMessageText);
}

UButton* UAetherMainMenuWidget::BuildButton(const FText& Label, UVerticalBox* Parent, bool bPrimary)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	Button->SetBackgroundColor(bPrimary ? FLinearColor(0.20f, 0.16f, 0.08f, 1.0f) : AetherButton);
	UTextBlock* LabelText = BuildText(Label, 15, bPrimary ? AetherGold : AetherIvory);
	LabelText->SetJustification(ETextJustify::Left);
	Button->SetContent(LabelText);
	if (Parent)
	{
		if (UVerticalBoxSlot* ButtonSlot = Parent->AddChildToVerticalBox(Button))
		{
			ButtonSlot->SetPadding(FMargin(0.0f, 3.0f));
			ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
		}
	}
	return Button;
}

UTextBlock* UAetherMainMenuWidget::BuildText(const FText& Text, int32 Size, const FLinearColor& Color, bool bWrap)
{
	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TextBlock->SetText(Text);
	TextBlock->SetColorAndOpacity(Color);
	TextBlock->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), Size));
	TextBlock->SetAutoWrapText(bWrap);
	return TextBlock;
}

UHorizontalBox* UAetherMainMenuWidget::BuildSettingRow(UVerticalBox* Parent, const FText& Label, UWidget* Control)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	if (Parent)
	{
		if (UVerticalBoxSlot* RowSlot = Parent->AddChildToVerticalBox(Row))
		{
			RowSlot->SetPadding(FMargin(0.0f, 5.0f));
		}
	}
	UTextBlock* LabelText = BuildText(Label, 14, AetherIvory);
	if (UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(LabelText))
	{
		LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LabelSlot->SetVerticalAlignment(VAlign_Center);
	}
	USizeBox* ControlSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	ControlSize->SetWidthOverride(260.0f);
	ControlSize->SetContent(Control);
	if (UHorizontalBoxSlot* ControlSlot = Row->AddChildToHorizontalBox(ControlSize))
	{
		ControlSlot->SetHorizontalAlignment(HAlign_Right);
		ControlSlot->SetVerticalAlignment(VAlign_Center);
	}
	return Row;
}

UComboBoxString* UAetherMainMenuWidget::BuildComboBox(const TArray<FString>& Options)
{
	UComboBoxString* Combo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass());
	FComboBoxStyle ComboStyle = Combo->GetWidgetStyle();
	FComboButtonStyle ComboButtonStyle = ComboStyle.ComboButtonStyle;
	FButtonStyle ButtonStyle = ComboButtonStyle.ButtonStyle;
	ButtonStyle.Normal.TintColor = FSlateColor(AetherControl);
	ButtonStyle.Hovered.TintColor = FSlateColor(AetherControlHover);
	ButtonStyle.Pressed.TintColor = FSlateColor(AetherControlPressed);
	ButtonStyle.Disabled.TintColor = FSlateColor(AetherControl.CopyWithNewOpacity(0.45f));
	ButtonStyle.NormalForeground = FSlateColor(AetherIvory);
	ButtonStyle.HoveredForeground = FSlateColor(AetherIvory);
	ButtonStyle.PressedForeground = FSlateColor(AetherGold);
	ButtonStyle.DisabledForeground = FSlateColor(AetherMuted);
	ComboButtonStyle.SetButtonStyle(ButtonStyle);
	ComboButtonStyle.DownArrowImage.TintColor = FSlateColor(AetherGold);
	ComboButtonStyle.MenuBorderBrush.TintColor = FSlateColor(AetherControl);
	ComboStyle.SetComboButtonStyle(ComboButtonStyle).SetContentPadding(FMargin(10.0f, 4.0f));
	Combo->SetWidgetStyle(ComboStyle);

	FTableRowStyle ItemStyle = Combo->GetItemStyle();
	ItemStyle.EvenRowBackgroundBrush.TintColor = FSlateColor(AetherControl);
	ItemStyle.OddRowBackgroundBrush.TintColor = FSlateColor(AetherControl);
	ItemStyle.EvenRowBackgroundHoveredBrush.TintColor = FSlateColor(AetherControlHover);
	ItemStyle.OddRowBackgroundHoveredBrush.TintColor = FSlateColor(AetherControlHover);
	ItemStyle.ActiveBrush.TintColor = FSlateColor(AetherControlPressed);
	ItemStyle.ActiveHoveredBrush.TintColor = FSlateColor(AetherControlHover);
	ItemStyle.InactiveBrush.TintColor = FSlateColor(AetherControl);
	ItemStyle.InactiveHoveredBrush.TintColor = FSlateColor(AetherControlHover);
	ItemStyle.SetTextColor(FSlateColor(AetherIvory)).SetSelectedTextColor(FSlateColor(AetherIvory));
	Combo->SetItemStyle(ItemStyle);
	Combo->OnGenerateWidgetEvent.BindDynamic(this, &UAetherMainMenuWidget::HandleGenerateComboWidget);
	for (const FString& Option : Options)
	{
		Combo->AddOption(Option);
	}
	if (Options.Num() > 0)
	{
		Combo->SetSelectedIndex(0);
	}
	return Combo;
}

USpinBox* UAetherMainMenuWidget::BuildSpinBox(float MinValue, float MaxValue, float Delta, float MinFractionalDigits)
{
	USpinBox* SpinBox = WidgetTree->ConstructWidget<USpinBox>(USpinBox::StaticClass());
	FSpinBoxStyle SpinStyle = SpinBox->GetWidgetStyle();
	SpinStyle.BackgroundBrush.TintColor = FSlateColor(AetherControl);
	SpinStyle.ActiveBackgroundBrush.TintColor = FSlateColor(AetherControl);
	SpinStyle.HoveredBackgroundBrush.TintColor = FSlateColor(AetherControlHover);
	SpinStyle.ActiveFillBrush.TintColor = FSlateColor(AetherGold.CopyWithNewOpacity(0.55f));
	SpinStyle.HoveredFillBrush.TintColor = FSlateColor(AetherGold.CopyWithNewOpacity(0.35f));
	SpinStyle.InactiveFillBrush.TintColor = FSlateColor(AetherMuted.CopyWithNewOpacity(0.30f));
	SpinStyle.ArrowsImage.TintColor = FSlateColor(AetherGold);
	SpinStyle.SetForegroundColor(FSlateColor(AetherIvory)).SetTextPadding(FMargin(8.0f, 3.0f));
	SpinBox->SetWidgetStyle(SpinStyle);
	SpinBox->SetForegroundColor(FSlateColor(AetherIvory));
	SpinBox->SetMinValue(MinValue);
	SpinBox->SetMaxValue(MaxValue);
	SpinBox->SetMinSliderValue(MinValue);
	SpinBox->SetMaxSliderValue(MaxValue);
	SpinBox->SetDelta(Delta);
	SpinBox->SetMinFractionalDigits(FMath::RoundToInt(MinFractionalDigits));
	SpinBox->SetMaxFractionalDigits(FMath::Max(0, FMath::RoundToInt(MinFractionalDigits)));
	return SpinBox;
}

UCheckBox* UAetherMainMenuWidget::BuildCheckBox()
{
	UCheckBox* CheckBox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
	FCheckBoxStyle CheckStyle = CheckBox->GetWidgetStyle();
	CheckStyle.UncheckedImage.TintColor = FSlateColor(AetherMuted);
	CheckStyle.UncheckedHoveredImage.TintColor = FSlateColor(AetherGold.CopyWithNewOpacity(0.70f));
	CheckStyle.UncheckedPressedImage.TintColor = FSlateColor(AetherControlPressed);
	CheckStyle.CheckedImage.TintColor = FSlateColor(AetherGold);
	CheckStyle.CheckedHoveredImage.TintColor = FSlateColor(AetherGold);
	CheckStyle.CheckedPressedImage.TintColor = FSlateColor(AetherIvory);
	CheckStyle.SetForegroundColor(FSlateColor(AetherIvory))
		.SetHoveredForegroundColor(FSlateColor(AetherGold))
		.SetCheckedForegroundColor(FSlateColor(AetherGold));
	CheckBox->SetWidgetStyle(CheckStyle);
	return CheckBox;
}

UVerticalBox* UAetherMainMenuWidget::BuildSettingsCategory(const FText& Heading)
{
	UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass());
	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Content->AddChildToVerticalBox(BuildText(Heading, 16, AetherGold));
	Scroll->AddChild(Content);
	SettingsCategorySwitcher->AddChild(Scroll);
	return Content;
}

void UAetherMainMenuWidget::BindActions()
{
	if (bActionsBound)
	{
		return;
	}
	bActionsBound = true;
	ContinueButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleContinueClicked);
	NewGameButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleNewGameClicked);
	LoadButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleLoadClicked);
	SettingsButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleSettingsClicked);
	CreditsButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleCreditsClicked);
	QuitButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleQuitClicked);
	LoadSelectedButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleLoadSelectedClicked);
	LoadBackButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleBackClicked);
	CreditsBackButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleBackClicked);
	PopupConfirmButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandlePopupConfirmClicked);
	PopupCancelButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandlePopupCancelClicked);
	SettingsApplyButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleSettingsApplyClicked);
	SettingsCancelButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleSettingsCancelClicked);
	SettingsDefaultsButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleSettingsDefaultsClicked);
	DisplayTabButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleDisplayTabClicked);
	GraphicsTabButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleGraphicsTabClicked);
	AudioTabButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleAudioTabClicked);
	GameplayTabButton->OnClicked.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleGameplayTabClicked);
	OverallQualityCombo->OnSelectionChanged.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleOverallQualityChanged);
	for (UComboBoxString* QualityCombo : IndividualQualityCombos)
	{
		if (QualityCombo)
		{
			QualityCombo->OnSelectionChanged.AddUniqueDynamic(this, &UAetherMainMenuWidget::HandleIndividualQualityChanged);
		}
	}
}

void UAetherMainMenuWidget::RefreshSaveState()
{
	if (MenuContext == EAetherMenuContext::PauseMenu)
	{
		ContinueButton->SetIsEnabled(true);
		return;
	}

	const UAetherSaveSubsystem* SaveSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	const FAetherSaveSlotSummary Summary = SaveSubsystem ? SaveSubsystem->GetPrototypeCheckpointSummary() : FAetherSaveSlotSummary();
	ContinueButton->SetIsEnabled(Summary.bLoadable);
	LoadButton->SetIsEnabled(Summary.bExists);
	LoadSelectedButton->SetIsEnabled(Summary.bLoadable);
	if (SaveSummaryText)
	{
		FText Detail = Summary.StatusText;
		if (Summary.bExists && Summary.SavedAtUtc.GetTicks() > 0)
		{
			Detail = FText::Format(
				NSLOCTEXT("AetherMenu", "SaveSummaryWithTime", "{0}\nSaved {1} UTC\nSchema {2}"),
				Summary.StatusText,
				FText::FromString(Summary.SavedAtUtc.ToString(TEXT("%Y-%m-%d %H:%M"))),
				FText::AsNumber(Summary.SaveSchemaVersion));
		}
		SaveSummaryText->SetText(Detail);
	}
}

void UAetherMainMenuWidget::FocusPrimaryAction()
{
	ShowMainScreen();
}

void UAetherMainMenuWidget::ShowMainScreen()
{
	ActiveContentIndex = 0;
	ContentSwitcher->SetActiveWidgetIndex(ActiveContentIndex);
	RefreshSaveState();
	if (MenuContext == EAetherMenuContext::PauseMenu)
	{
		ContinueButton->SetKeyboardFocus();
	}
	else
	{
		(ContinueButton->GetIsEnabled() ? ContinueButton.Get() : NewGameButton.Get())->SetKeyboardFocus();
	}
}

void UAetherMainMenuWidget::ShowLoadScreen()
{
	ActiveContentIndex = 1;
	ContentSwitcher->SetActiveWidgetIndex(ActiveContentIndex);
	RefreshSaveState();
	(LoadSelectedButton->GetIsEnabled() ? LoadSelectedButton.Get() : LoadBackButton.Get())->SetKeyboardFocus();
}

void UAetherMainMenuWidget::ShowSettingsScreen()
{
	ActiveContentIndex = 2;
	ContentSwitcher->SetActiveWidgetIndex(ActiveContentIndex);
	if (UAetherSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr)
	{
		Settings->BeginSettingsEdit();
	}
	SynchronizeSettingsControls();
	ShowSettingsCategory(0);
}

void UAetherMainMenuWidget::ShowCreditsScreen()
{
	ActiveContentIndex = 3;
	ContentSwitcher->SetActiveWidgetIndex(ActiveContentIndex);
	CreditsBackButton->SetKeyboardFocus();
}

void UAetherMainMenuWidget::ShowSettingsCategory(int32 CategoryIndex)
{
	SettingsCategorySwitcher->SetActiveWidgetIndex(FMath::Clamp(CategoryIndex, 0, 3));
	UButton* FocusButton = DisplayTabButton;
	if (CategoryIndex == 1) FocusButton = GraphicsTabButton;
	if (CategoryIndex == 2) FocusButton = AudioTabButton;
	if (CategoryIndex == 3) FocusButton = GameplayTabButton;
	FocusButton->SetKeyboardFocus();
}

void UAetherMainMenuWidget::ShowPopup(EPopupAction Action, const FText& Title, const FText& Message, const FText& ConfirmLabel, bool bShowCancel)
{
	ActivePopupAction = Action;
	PopupTitleText->SetText(Title);
	PopupMessageText->SetText(Message);
	PopupConfirmText->SetText(ConfirmLabel);
	PopupCancelButton->SetVisibility(bShowCancel ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	PopupOverlay->SetVisibility(ESlateVisibility::Visible);
	PopupConfirmButton->SetKeyboardFocus();
}

void UAetherMainMenuWidget::HidePopup()
{
	PopupOverlay->SetVisibility(ESlateVisibility::Collapsed);
	ActivePopupAction = EPopupAction::None;
	if (ActiveContentIndex == 2)
	{
		SettingsApplyButton->SetKeyboardFocus();
	}
	else
	{
		ShowMainScreen();
	}
}

void UAetherMainMenuWidget::RestorePauseMenuAfterTransitionFailure(const FText& Message)
{
	SetLoadingState(false, FText::GetEmpty());
	ShowPopup(
		EPopupAction::Information,
		NSLOCTEXT("AetherMenu", "ReturnFailedTitle", "UNABLE TO RETURN"),
		Message,
		NSLOCTEXT("AetherMenu", "Okay", "OK"),
		false);
}

void UAetherMainMenuWidget::SetLoadingState(bool bLoading, const FText& Message)
{
	LoadingMessageText->SetText(Message);
	LoadingOverlay->SetVisibility(bLoading ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	SetIsEnabled(!bLoading);
}

void UAetherMainMenuWidget::SynchronizeSettingsControls()
{
	UAetherSettingsSubsystem* SettingsSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr;
	if (!SettingsSubsystem)
	{
		return;
	}

	const FAetherSettingsSnapshot Settings = SettingsSubsystem->GetPendingSettings();
	WindowModeCombo->SetSelectedIndex(static_cast<int32>(Settings.Video.WindowMode));
	ResolutionCombo->ClearOptions();
	TArray<FIntPoint> Resolutions = SettingsSubsystem->GetSupportedResolutions();
	if (!Resolutions.Contains(Settings.Video.Resolution))
	{
		Resolutions.Add(Settings.Video.Resolution);
	}
	for (const FIntPoint& Resolution : Resolutions)
	{
		ResolutionCombo->AddOption(FormatResolution(Resolution));
	}
	ResolutionCombo->SetSelectedOption(FormatResolution(Settings.Video.Resolution));
	VSyncCheckBox->SetIsChecked(Settings.Video.bVSyncEnabled);
	const FString FrameLimit = Settings.Video.FrameRateLimit <= 0.0f ? TEXT("Unlimited") : FString::FromInt(FMath::RoundToInt(Settings.Video.FrameRateLimit));
	if (FrameLimitCombo->FindOptionIndex(FrameLimit) == INDEX_NONE)
	{
		FrameLimitCombo->AddOption(FrameLimit);
	}
	FrameLimitCombo->SetSelectedOption(FrameLimit);
	ResolutionScaleSpinBox->SetValue(Settings.Video.ResolutionScale);
	GammaSpinBox->SetValue(Settings.Custom.Gamma);
	OverallQualityCombo->SetSelectedIndex(Settings.Video.OverallQuality + 1);
	const int32 QualityValues[] = {
		Settings.Video.ViewDistanceQuality,
		Settings.Video.AntiAliasingQuality,
		Settings.Video.ShadowQuality,
		Settings.Video.GlobalIlluminationQuality,
		Settings.Video.ReflectionQuality,
		Settings.Video.TextureQuality,
		Settings.Video.EffectsQuality,
		Settings.Video.PostProcessQuality,
		Settings.Video.FoliageQuality,
		Settings.Video.ShadingQuality};
	for (int32 Index = 0; Index < IndividualQualityCombos.Num(); ++Index)
	{
		IndividualQualityCombos[Index]->SetSelectedIndex(QualityValues[Index]);
	}
	MotionBlurCheckBox->SetIsChecked(Settings.Custom.bMotionBlurEnabled);
	MasterVolumeSpinBox->SetValue(Settings.Custom.MasterVolume * 100.0f);
	MusicVolumeSpinBox->SetValue(Settings.Custom.MusicVolume * 100.0f);
	SfxVolumeSpinBox->SetValue(Settings.Custom.SfxVolume * 100.0f);
	VoiceVolumeSpinBox->SetValue(Settings.Custom.VoiceVolume * 100.0f);
	UiVolumeSpinBox->SetValue(Settings.Custom.UiVolume * 100.0f);
	MuteCheckBox->SetIsChecked(Settings.Custom.bMuteAll);
	SubtitlesCheckBox->SetIsChecked(Settings.Custom.bSubtitlesEnabled);
	SubtitleSizeCombo->SetSelectedIndex(static_cast<int32>(Settings.Custom.SubtitleSize));
	DialogueAutoAdvanceCheckBox->SetIsChecked(Settings.Custom.bDialogueAutoAdvance);
	CameraSensitivityXSpinBox->SetValue(Settings.Custom.CameraSensitivityX);
	CameraSensitivityYSpinBox->SetValue(Settings.Custom.CameraSensitivityY);
	InvertCameraYCheckBox->SetIsChecked(Settings.Custom.bInvertCameraY);
	VibrationCheckBox->SetIsChecked(Settings.Custom.bVibrationEnabled);
	ScreenShakeSpinBox->SetValue(Settings.Custom.ScreenShakeScale * 100.0f);
}

FAetherSettingsSnapshot UAetherMainMenuWidget::CollectSettingsControls() const
{
	FAetherSettingsSnapshot Settings = GetGameInstance() && GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>()
		? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>()->GetPendingSettings()
		: FAetherSettingsSnapshot();
	Settings.Video.WindowMode = static_cast<EAetherWindowMode>(FMath::Clamp(WindowModeCombo->GetSelectedIndex(), 0, 2));
	Settings.Video.Resolution = ParseResolution(ResolutionCombo->GetSelectedOption(), Settings.Video.Resolution);
	Settings.Video.bVSyncEnabled = VSyncCheckBox->IsChecked();
	Settings.Video.FrameRateLimit = FrameLimitCombo->GetSelectedOption() == TEXT("Unlimited") ? 0.0f : FCString::Atof(*FrameLimitCombo->GetSelectedOption());
	Settings.Video.ResolutionScale = ResolutionScaleSpinBox->GetValue();
	Settings.Custom.Gamma = GammaSpinBox->GetValue();
	Settings.Video.OverallQuality = OverallQualityCombo->GetSelectedIndex() - 1;
	if (IndividualQualityCombos.Num() == 10)
	{
		Settings.Video.ViewDistanceQuality = QualityIndex(IndividualQualityCombos[0]);
		Settings.Video.AntiAliasingQuality = QualityIndex(IndividualQualityCombos[1]);
		Settings.Video.ShadowQuality = QualityIndex(IndividualQualityCombos[2]);
		Settings.Video.GlobalIlluminationQuality = QualityIndex(IndividualQualityCombos[3]);
		Settings.Video.ReflectionQuality = QualityIndex(IndividualQualityCombos[4]);
		Settings.Video.TextureQuality = QualityIndex(IndividualQualityCombos[5]);
		Settings.Video.EffectsQuality = QualityIndex(IndividualQualityCombos[6]);
		Settings.Video.PostProcessQuality = QualityIndex(IndividualQualityCombos[7]);
		Settings.Video.FoliageQuality = QualityIndex(IndividualQualityCombos[8]);
		Settings.Video.ShadingQuality = QualityIndex(IndividualQualityCombos[9]);
	}
	Settings.Custom.bMotionBlurEnabled = MotionBlurCheckBox->IsChecked();
	Settings.Custom.MasterVolume = MasterVolumeSpinBox->GetValue() / 100.0f;
	Settings.Custom.MusicVolume = MusicVolumeSpinBox->GetValue() / 100.0f;
	Settings.Custom.SfxVolume = SfxVolumeSpinBox->GetValue() / 100.0f;
	Settings.Custom.VoiceVolume = VoiceVolumeSpinBox->GetValue() / 100.0f;
	Settings.Custom.UiVolume = UiVolumeSpinBox->GetValue() / 100.0f;
	Settings.Custom.bMuteAll = MuteCheckBox->IsChecked();
	Settings.Custom.bSubtitlesEnabled = SubtitlesCheckBox->IsChecked();
	Settings.Custom.SubtitleSize = static_cast<EAetherSubtitleSize>(FMath::Clamp(SubtitleSizeCombo->GetSelectedIndex(), 0, 2));
	Settings.Custom.bDialogueAutoAdvance = DialogueAutoAdvanceCheckBox->IsChecked();
	Settings.Custom.CameraSensitivityX = CameraSensitivityXSpinBox->GetValue();
	Settings.Custom.CameraSensitivityY = CameraSensitivityYSpinBox->GetValue();
	Settings.Custom.bInvertCameraY = InvertCameraYCheckBox->IsChecked();
	Settings.Custom.bVibrationEnabled = VibrationCheckBox->IsChecked();
	Settings.Custom.ScreenShakeScale = ScreenShakeSpinBox->GetValue() / 100.0f;
	return Settings;
}

void UAetherMainMenuWidget::ApplyOverallQualitySelection(int32 QualityLevel)
{
	if (QualityLevel < 0)
	{
		return;
	}

	if (UAetherSettingsSubsystem* SettingsSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr)
	{
		const FAetherSettingsSnapshot PresetSettings = SettingsSubsystem->BuildPerformancePresetSettings(QualityLevel, CollectSettingsControls());
		SettingsSubsystem->SetPendingSettings(PresetSettings);
		SynchronizeSettingsControls();
		return;
	}

	for (UComboBoxString* Combo : IndividualQualityCombos)
	{
		Combo->SetSelectedIndex(QualityLevel);
	}
}

void UAetherMainMenuWidget::HandleBack()
{
	if (LoadingOverlay->GetVisibility() == ESlateVisibility::Visible)
	{
		return;
	}
	if (PopupOverlay->GetVisibility() == ESlateVisibility::Visible)
	{
		if (ActivePopupAction == EPopupAction::VideoConfirmation)
		{
			if (UAetherSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr)
			{
				Settings->RevertVideoSettings();
			}
		}
		else
		{
			HidePopup();
		}
		return;
	}
	if (ActiveContentIndex == 2)
	{
		HandleSettingsCancelClicked();
	}
	else if (ActiveContentIndex != 0)
	{
		ShowMainScreen();
	}
	else if (MenuContext == EAetherMenuContext::PauseMenu)
	{
		OnResumeRequested.Broadcast();
	}
	else
	{
		HandleQuitClicked();
	}
}

void UAetherMainMenuWidget::HandleContinueClicked()
{
	if (MenuContext == EAetherMenuContext::PauseMenu)
	{
		OnResumeRequested.Broadcast();
		return;
	}

	if (UAetherMenuFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr)
	{
		Flow->ContinueGame();
	}
}

void UAetherMainMenuWidget::HandleNewGameClicked()
{
	const UAetherSaveSubsystem* Save = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSaveSubsystem>() : nullptr;
	if (Save && Save->HasPrototypeCheckpointSnapshot())
	{
		ShowPopup(
			EPopupAction::OverwriteProgress,
			NSLOCTEXT("AetherMenu", "OverwriteTitle", "BEGIN A NEW JOURNEY?"),
			NSLOCTEXT("AetherMenu", "OverwriteMessage", "Existing checkpoint progress will be permanently replaced."),
			NSLOCTEXT("AetherMenu", "OverwriteConfirm", "BEGIN NEW GAME"),
			true);
		return;
	}
	if (UAetherMenuFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr)
	{
		Flow->StartNewGame(false);
	}
}

void UAetherMainMenuWidget::HandleLoadClicked() { ShowLoadScreen(); }
void UAetherMainMenuWidget::HandleLoadSelectedClicked()
{
	if (UAetherMenuFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr)
	{
		Flow->LoadGame();
	}
}
void UAetherMainMenuWidget::HandleSettingsClicked() { ShowSettingsScreen(); }
void UAetherMainMenuWidget::HandleCreditsClicked() { ShowCreditsScreen(); }

void UAetherMainMenuWidget::HandleQuitClicked()
{
	if (MenuContext == EAetherMenuContext::PauseMenu)
	{
		ShowPopup(
			EPopupAction::ReturnToMainMenu,
			NSLOCTEXT("AetherMenu", "ReturnTitle", "RETURN TO THE MAIN MENU?"),
			NSLOCTEXT("AetherMenu", "ReturnMessage", "Progress since the last checkpoint will be lost. Your recorded journey will be preserved."),
			NSLOCTEXT("AetherMenu", "ReturnConfirm", "RETURN TO MAIN MENU"),
			true);
		return;
	}

	ShowPopup(
		EPopupAction::QuitGame,
		NSLOCTEXT("AetherMenu", "QuitTitle", "LEAVE AETHERFALL?"),
		NSLOCTEXT("AetherMenu", "QuitMessage", "Unsaved progress since the last checkpoint will be lost."),
		NSLOCTEXT("AetherMenu", "QuitConfirm", "QUIT GAME"),
		true);
}

void UAetherMainMenuWidget::HandleBackClicked() { HandleBack(); }

void UAetherMainMenuWidget::HandlePopupConfirmClicked()
{
	const EPopupAction Action = ActivePopupAction;
	if (Action == EPopupAction::OverwriteProgress)
	{
		HidePopup();
		if (UAetherMenuFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr)
		{
			Flow->StartNewGame(true);
		}
	}
	else if (Action == EPopupAction::QuitGame)
	{
		HidePopup();
		if (UAetherMenuFlowSubsystem* Flow = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherMenuFlowSubsystem>() : nullptr)
		{
			Flow->QuitGame(GetOwningPlayer());
		}
	}
	else if (Action == EPopupAction::ReturnToMainMenu)
	{
		HidePopup();
		SetLoadingState(true, NSLOCTEXT("AetherMenu", "PauseMenuLoading", "RETURNING TO THE TITLE..."));
		OnReturnToMainMenuRequested.Broadcast();
	}
	else if (Action == EPopupAction::VideoConfirmation)
	{
		if (UAetherSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr)
		{
			Settings->ConfirmVideoSettings();
		}
		HidePopup();
	}
	else
	{
		HidePopup();
	}
}

void UAetherMainMenuWidget::HandlePopupCancelClicked()
{
	if (ActivePopupAction == EPopupAction::VideoConfirmation)
	{
		if (UAetherSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr)
		{
			Settings->RevertVideoSettings();
		}
	}
	HidePopup();
}

void UAetherMainMenuWidget::HandleSettingsApplyClicked()
{
	if (UAetherSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr)
	{
		Settings->SetPendingSettings(CollectSettingsControls());
		if (Settings->ApplyPendingSettings() && Settings->IsAwaitingVideoConfirmation())
		{
			ShowPopup(
				EPopupAction::VideoConfirmation,
				NSLOCTEXT("AetherMenu", "VideoConfirmTitle", "CONFIRM DISPLAY SETTINGS"),
				FText::GetEmpty(),
				NSLOCTEXT("AetherMenu", "KeepSettings", "KEEP SETTINGS"),
				true);
		}
	}
}

void UAetherMainMenuWidget::HandleSettingsCancelClicked()
{
	if (UAetherSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr)
	{
		Settings->CancelPendingSettings();
	}
	ShowMainScreen();
}

void UAetherMainMenuWidget::HandleSettingsDefaultsClicked()
{
	if (UAetherSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr)
	{
		Settings->RestorePendingDefaults();
		SynchronizeSettingsControls();
	}
}

void UAetherMainMenuWidget::HandleDisplayTabClicked() { ShowSettingsCategory(0); }
void UAetherMainMenuWidget::HandleGraphicsTabClicked() { ShowSettingsCategory(1); }
void UAetherMainMenuWidget::HandleAudioTabClicked() { ShowSettingsCategory(2); }
void UAetherMainMenuWidget::HandleGameplayTabClicked() { ShowSettingsCategory(3); }

UWidget* UAetherMainMenuWidget::HandleGenerateComboWidget(FString Item)
{
	return BuildText(FText::FromString(Item), 14, AetherIvory);
}

void UAetherMainMenuWidget::HandleOverallQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType != ESelectInfo::Direct)
	{
		ApplyOverallQualitySelection(OverallQualityCombo->GetSelectedIndex() - 1);
	}
}

void UAetherMainMenuWidget::HandleIndividualQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType != ESelectInfo::Direct && OverallQualityCombo)
	{
		OverallQualityCombo->SetSelectedIndex(0);
	}
}

void UAetherMainMenuWidget::HandleMenuFlowChanged(EAetherMenuFlowState State, FText Message)
{
	if (State == EAetherMenuFlowState::Transitioning)
	{
		SetLoadingState(true, Message);
	}
	else if (State == EAetherMenuFlowState::Failed)
	{
		SetLoadingState(false, FText::GetEmpty());
		ShowPopup(EPopupAction::Information, NSLOCTEXT("AetherMenu", "UnableTitle", "UNABLE TO CONTINUE"), Message, NSLOCTEXT("AetherMenu", "Okay", "OK"), false);
	}
}

void UAetherMainMenuWidget::HandleVideoConfirmationChanged(bool bAwaitingConfirmation)
{
	if (!bAwaitingConfirmation && ActivePopupAction == EPopupAction::VideoConfirmation)
	{
		HidePopup();
		SynchronizeSettingsControls();
	}
}
