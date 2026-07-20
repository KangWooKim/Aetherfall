#include "AetherLoadingScreenSubsystem.h"

#include "AetherLoadingScreenWidget.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SOverlay.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SVerticalBox.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	FText ResolveLoadingText(const FText& RequestedText, const FAetherLoadingScreenSettings& Settings)
	{
		return RequestedText.IsEmpty() ? Settings.LoadingText : RequestedText;
	}
}

void UAetherLoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UAetherLoadingScreenSubsystem::HandlePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UAetherLoadingScreenSubsystem::HandlePostLoadMap);
}

void UAetherLoadingScreenSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	CompleteHide();

	Super::Deinitialize();
}

void UAetherLoadingScreenSubsystem::BeginLoadingScreen(const FText& LoadingMessage, bool bWaitForGameplayReady)
{
	if (!ActiveSettings.bEnableViewportLoadingScreen)
	{
		return;
	}

	ActiveLoadingText = ResolveLoadingText(LoadingMessage, ActiveSettings);
	ActiveLoadingTip = ChooseLoadingTip();
	bMapLoadCompleted = false;
	bRequireGameplayReady = bWaitForGameplayReady;
	bGameplayReady = !bRequireGameplayReady;
	DisplayStartTime = FPlatformTime::Seconds();
	MapLoadCompletedTime = 0.0;
	FadeStartTime = DisplayStartTime;
	CurrentOpacity = ActiveSettings.FadeInTime <= 0.0f ? 1.0f : 0.0f;
	LoadingState = ActiveSettings.FadeInTime <= 0.0f ? EAetherLoadingScreenState::Holding : EAetherLoadingScreenState::FadingIn;

	ShowViewportLoadingScreen();
	TryCreateLoadingWidget(ResolveRuntimeWorld());
	RefreshVisuals();
	EnsureTicker();

	OnLoadingScreenShown.Broadcast(GetCurrentViewModel());
}

void UAetherLoadingScreenSubsystem::NotifyMapLoadCompleted()
{
	if (LoadingState == EAetherLoadingScreenState::Hidden)
	{
		return;
	}

	bMapLoadCompleted = true;
	if (!bRequireGameplayReady)
	{
		bGameplayReady = true;
	}
	MapLoadCompletedTime = FPlatformTime::Seconds();
	RefreshVisuals();
}

void UAetherLoadingScreenSubsystem::NotifyGameplayWorldReady()
{
	if (LoadingState == EAetherLoadingScreenState::Hidden)
	{
		return;
	}

	bGameplayReady = true;
	if (!bMapLoadCompleted)
	{
		bMapLoadCompleted = true;
		MapLoadCompletedTime = FPlatformTime::Seconds();
	}
	RefreshVisuals();
}

void UAetherLoadingScreenSubsystem::RequestHideLoadingScreen()
{
	if (LoadingState == EAetherLoadingScreenState::Hidden)
	{
		return;
	}

	bMapLoadCompleted = true;
	bGameplayReady = true;
	if (MapLoadCompletedTime <= 0.0)
	{
		MapLoadCompletedTime = FPlatformTime::Seconds();
	}
}

void UAetherLoadingScreenSubsystem::SetLoadingScreenSettings(const FAetherLoadingScreenSettings& InSettings)
{
	ActiveSettings = InSettings;
	if (LoadingState != EAetherLoadingScreenState::Hidden)
	{
		RefreshVisuals();
	}
}

FAetherLoadingScreenViewModel UAetherLoadingScreenSubsystem::GetCurrentViewModel() const
{
	FAetherLoadingScreenViewModel ViewModel;
	ViewModel.State = LoadingState;
	ViewModel.LoadingText = ActiveLoadingText.IsEmpty() ? ActiveSettings.LoadingText : ActiveLoadingText;
	ViewModel.LoadingTip = ActiveLoadingTip;
	ViewModel.Opacity = CurrentOpacity;
	ViewModel.bWaitingForMapLoad = !bMapLoadCompleted;
	ViewModel.bWaitingForGameplayReady = bRequireGameplayReady && !bGameplayReady;
	return ViewModel;
}

void UAetherLoadingScreenSubsystem::HandlePreLoadMap(const FString& MapName)
{
	if (LoadingState == EAetherLoadingScreenState::Hidden)
	{
		BeginLoadingScreen(ActiveSettings.LoadingText, false);
		return;
	}

	bMapLoadCompleted = false;
	MapLoadCompletedTime = 0.0;
}

void UAetherLoadingScreenSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (LoadingState == EAetherLoadingScreenState::Hidden)
	{
		return;
	}

	NotifyMapLoadCompleted();
	TryCreateLoadingWidget(LoadedWorld);
}

bool UAetherLoadingScreenSubsystem::TickLoadingScreen(float DeltaTime)
{
	if (LoadingState == EAetherLoadingScreenState::Hidden)
	{
		return false;
	}

	const double Now = FPlatformTime::Seconds();
	if (LoadingState == EAetherLoadingScreenState::FadingIn)
	{
		const float FadeInTime = FMath::Max(ActiveSettings.FadeInTime, KINDA_SMALL_NUMBER);
		CurrentOpacity = FMath::Clamp(static_cast<float>((Now - FadeStartTime) / FadeInTime), 0.0f, 1.0f);
		if (CurrentOpacity >= 1.0f)
		{
			CurrentOpacity = 1.0f;
			LoadingState = EAetherLoadingScreenState::Holding;
		}
	}
	else if (LoadingState == EAetherLoadingScreenState::Holding)
	{
		const bool bMinimumDisplaySatisfied = Now >= DisplayStartTime + FMath::Max(0.0f, ActiveSettings.MinimumDisplayTime);
		const bool bPostLoadHoldSatisfied = bMapLoadCompleted && Now >= MapLoadCompletedTime + FMath::Max(0.0f, ActiveSettings.PostLoadHoldTime);
		const bool bReadySatisfied = !bRequireGameplayReady || bGameplayReady;
		if (bMinimumDisplaySatisfied && bPostLoadHoldSatisfied && bReadySatisfied)
		{
			StartFadeOut();
		}
	}
	else if (LoadingState == EAetherLoadingScreenState::FadingOut)
	{
		if (ActiveSettings.FadeOutTime <= 0.0f)
		{
			CompleteHide();
			return false;
		}

		const float FadeOutTime = FMath::Max(ActiveSettings.FadeOutTime, KINDA_SMALL_NUMBER);
		const float FadeAlpha = FMath::Clamp(static_cast<float>((Now - FadeStartTime) / FadeOutTime), 0.0f, 1.0f);
		CurrentOpacity = 1.0f - FadeAlpha;
		if (FadeAlpha >= 1.0f)
		{
			CompleteHide();
			return false;
		}
	}

	RefreshVisuals();
	OnLoadingScreenUpdated.Broadcast(GetCurrentViewModel());
	return true;
}

void UAetherLoadingScreenSubsystem::ShowViewportLoadingScreen()
{
	if (ViewportLoadingWidget.IsValid() || !GEngine || !GEngine->GameViewport)
	{
		return;
	}

	BackgroundBrush = MakeShared<FSlateBrush>(*FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")));
	if (UTexture2D* BackgroundTexture = ActiveSettings.BackgroundImage.LoadSynchronous())
	{
		BackgroundBrush->SetResourceObject(BackgroundTexture);
		BackgroundBrush->ImageSize = FVector2D(
			FMath::Max(1.0f, static_cast<float>(BackgroundTexture->GetSurfaceWidth())),
			FMath::Max(1.0f, static_cast<float>(BackgroundTexture->GetSurfaceHeight())));
	}

	TSharedRef<SOverlay> Overlay = SNew(SOverlay);
	Overlay->AddSlot()
	[
		SAssignNew(LoadingRootBorder, SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
		.Padding(0.0f)
	];

	if (ActiveSettings.BackgroundImage.IsValid() || !ActiveSettings.BackgroundImage.IsNull())
	{
		Overlay->AddSlot()
		[
			SAssignNew(LoadingBackgroundImage, SImage)
			.Image(BackgroundBrush.Get())
		];
	}

	Overlay->AddSlot()
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SBox)
		.WidthOverride(760.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush(TEXT("WhiteBrush")))
			.BorderBackgroundColor(FSlateColor(FLinearColor(0.012f, 0.016f, 0.022f, 0.88f)))
			.Padding(FMargin(42.0f, 34.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(LoadingTextBlock, STextBlock)
					.Text(ActiveLoadingText)
					.Font(FCoreStyle::GetDefaultFontStyle(FName(TEXT("Bold")), 30))
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 18.0f, 0.0f, 0.0f))
				[
					SAssignNew(LoadingTipTextBlock, STextBlock)
					.Text(ActiveLoadingTip)
					.Font(FCoreStyle::GetDefaultFontStyle(FName(TEXT("Regular")), 16))
					.Justification(ETextJustify::Center)
					.AutoWrapText(true)
				]
			]
		]
	];

	ViewportLoadingWidget = Overlay;
	GEngine->GameViewport->AddViewportWidgetContent(ViewportLoadingWidget.ToSharedRef(), ActiveSettings.ViewportZOrder);
}

void UAetherLoadingScreenSubsystem::RemoveViewportLoadingScreen()
{
	if (ViewportLoadingWidget.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ViewportLoadingWidget.ToSharedRef());
	}

	ViewportLoadingWidget.Reset();
	LoadingRootBorder.Reset();
	LoadingBackgroundImage.Reset();
	LoadingTextBlock.Reset();
	LoadingTipTextBlock.Reset();
}

void UAetherLoadingScreenSubsystem::TryCreateLoadingWidget(UWorld* World)
{
	if (LoadingWidget || !World)
	{
		if (LoadingWidget)
		{
			LoadingWidget->ApplyLoadingScreenViewModel(GetCurrentViewModel());
		}
		return;
	}

	TSubclassOf<UAetherLoadingScreenWidget> ResolvedWidgetClass = ActiveSettings.LoadingWidgetClass;
	if (!ResolvedWidgetClass && !ActiveSettings.LoadingWidgetBlueprintClass.IsNull())
	{
		ResolvedWidgetClass = ActiveSettings.LoadingWidgetBlueprintClass.LoadSynchronous();
	}
	if (!ResolvedWidgetClass)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	LoadingWidget = PlayerController ? CreateWidget<UAetherLoadingScreenWidget>(PlayerController, ResolvedWidgetClass) : nullptr;
	if (!LoadingWidget)
	{
		return;
	}

	LoadingWidget->AddToViewport(ActiveSettings.ViewportZOrder + 1);
	LoadingWidget->ApplyLoadingScreenViewModel(GetCurrentViewModel());
}

void UAetherLoadingScreenSubsystem::RemoveLoadingWidget()
{
	if (LoadingWidget)
	{
		LoadingWidget->RemoveFromParent();
		LoadingWidget = nullptr;
	}
}

void UAetherLoadingScreenSubsystem::StartFadeOut()
{
	if (LoadingState == EAetherLoadingScreenState::FadingOut)
	{
		return;
	}

	LoadingState = EAetherLoadingScreenState::FadingOut;
	FadeStartTime = FPlatformTime::Seconds();
	if (ActiveSettings.FadeOutTime <= 0.0f)
	{
		CompleteHide();
	}
}

void UAetherLoadingScreenSubsystem::CompleteHide()
{
	if (LoadingState == EAetherLoadingScreenState::Hidden && !ViewportLoadingWidget.IsValid() && !LoadingWidget)
	{
		RemoveTicker();
		return;
	}

	LoadingState = EAetherLoadingScreenState::Hidden;
	CurrentOpacity = 0.0f;
	bMapLoadCompleted = false;
	bRequireGameplayReady = false;
	bGameplayReady = false;
	DisplayStartTime = 0.0;
	MapLoadCompletedTime = 0.0;
	FadeStartTime = 0.0;

	const FAetherLoadingScreenViewModel HiddenViewModel = GetCurrentViewModel();
	RemoveLoadingWidget();
	RemoveViewportLoadingScreen();
	RemoveTicker();
	OnLoadingScreenHidden.Broadcast(HiddenViewModel);
}

void UAetherLoadingScreenSubsystem::EnsureTicker()
{
	if (!TickerHandle.IsValid())
	{
		TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UAetherLoadingScreenSubsystem::TickLoadingScreen));
	}
}

void UAetherLoadingScreenSubsystem::RemoveTicker()
{
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}
}

void UAetherLoadingScreenSubsystem::RefreshVisuals()
{
	if (LoadingTextBlock)
	{
		LoadingTextBlock->SetText(ActiveLoadingText);
	}
	if (LoadingTipTextBlock)
	{
		LoadingTipTextBlock->SetText(ActiveLoadingTip);
	}
	if (LoadingWidget)
	{
		LoadingWidget->ApplyLoadingScreenViewModel(GetCurrentViewModel());
	}
	ApplyOpacity(CurrentOpacity);
}

void UAetherLoadingScreenSubsystem::ApplyOpacity(float NewOpacity)
{
	const float ClampedOpacity = FMath::Clamp(NewOpacity, 0.0f, 1.0f);
	if (LoadingRootBorder)
	{
		LoadingRootBorder->SetBorderBackgroundColor(FSlateColor(ActiveSettings.BackgroundColor.CopyWithNewOpacity(ClampedOpacity)));
	}
	if (LoadingBackgroundImage)
	{
		LoadingBackgroundImage->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 0.24f * ClampedOpacity)));
	}
	if (LoadingTextBlock)
	{
		LoadingTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.94f, 0.96f, 0.98f, ClampedOpacity)));
	}
	if (LoadingTipTextBlock)
	{
		LoadingTipTextBlock->SetColorAndOpacity(FSlateColor(ActiveSettings.AccentColor.CopyWithNewOpacity(0.92f * ClampedOpacity)));
	}
}

FText UAetherLoadingScreenSubsystem::ChooseLoadingTip() const
{
	if (ActiveSettings.LoadingTips.Num() <= 0)
	{
		return FText::GetEmpty();
	}

	return ActiveSettings.LoadingTips[FMath::RandHelper(ActiveSettings.LoadingTips.Num())];
}

UWorld* UAetherLoadingScreenSubsystem::ResolveRuntimeWorld() const
{
	if (UWorld* World = GetWorld())
	{
		return World;
	}

	if (!GEngine)
	{
		return nullptr;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		UWorld* Candidate = WorldContext.World();
		if (Candidate && (WorldContext.WorldType == EWorldType::Game || WorldContext.WorldType == EWorldType::PIE))
		{
			return Candidate;
		}
	}

	return nullptr;
}
