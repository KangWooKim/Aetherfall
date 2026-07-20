#include "AetherHUD.h"

#include "AetherCinematicDirectorSubsystem.h"
#include "AetherSettingsSubsystem.h"
#include "AetherCombatComponent.h"
#include "AetherCombatHudPresenter.h"
#include "AetherEnemyBase.h"
#include "AetherGameModeBase.h"
#include "AetherHealthComponent.h"
#include "AetherInteractionComponent.h"
#include "AetherInventoryComponent.h"
#include "AetherLockOnComponent.h"
#include "AetherPrototypeRouteGuide.h"
#include "AetherfallCharacter.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

void AAetherHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UAetherCinematicDirectorSubsystem* CinematicDirector = GameInstance ? GameInstance->GetSubsystem<UAetherCinematicDirectorSubsystem>() : nullptr;
	if (CinematicDirector && CinematicDirector->ShouldHideHud())
	{
		return;
	}

	const AAetherfallCharacter* PlayerCharacter = Cast<AAetherfallCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!PlayerCharacter)
	{
		return;
	}

	DrawPlayerStatus(PlayerCharacter);
	DrawPrototypeQuickItemStatus(PlayerCharacter);
	DrawEnemyStatus(PlayerCharacter);
	DrawIncomingThreatStatus(PlayerCharacter);
	DrawPlayerDangerStatus(PlayerCharacter);
	DrawPrototypeRoundStatus();
	DrawPrototypeLevelStatus();
	DrawPrototypeCheckpointStatus();
	DrawPrototypeCheckpointFeedback();
	DrawPrototypeProgressFeedback();
	DrawPrototypeDialogue();
	DrawPrototypeRouteGuidance(PlayerCharacter);
	DrawInteractionPrompt(PlayerCharacter);
}

void AAetherHUD::DrawPlayerStatus(const AAetherfallCharacter* PlayerCharacter)
{
	const UAetherHealthComponent* HealthComponent = PlayerCharacter ? PlayerCharacter->GetHealthComponent() : nullptr;
	const UAetherCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	if (!HealthComponent || !CombatComponent)
	{
		return;
	}

	constexpr float X = 32.0f;
	const float BaseY = Canvas->SizeY - 152.0f;
	const float SafeMaxHealth = FMath::Max(HealthComponent->GetMaxHealth(), 1.0f);
	const float HealthPercent = FMath::Clamp(HealthComponent->GetCurrentHealth() / SafeMaxHealth, 0.0f, 1.0f);
	const FLinearColor HealthFillColor = HealthComponent->IsDead()
		? FLinearColor(0.24f, 0.01f, 0.01f, 1.0f)
		: (HealthPercent <= 0.25f
			? FLinearColor(0.95f, 0.02f, 0.01f, 1.0f)
			: (HealthPercent <= 0.45f
				? FLinearColor(0.98f, 0.34f, 0.06f, 1.0f)
				: FLinearColor(0.82f, 0.08f, 0.07f, 1.0f)));

	DrawText(TEXT("PLAYER"), FLinearColor::White, X, BaseY - 26.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 1.1f);
	DrawStatusBar(TEXT("HP"), HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth(), X, BaseY, 300.0f, 18.0f, HealthFillColor);
	DrawStatusBar(TEXT("ST"), CombatComponent->GetCurrentStamina(), CombatComponent->GetMaxStamina(), X, BaseY + 30.0f, 300.0f, 18.0f, FLinearColor(0.10f, 0.62f, 0.24f, 1.0f));
	DrawStatusBar(TEXT("AE"), CombatComponent->GetCurrentAetherGauge(), CombatComponent->GetMaxAetherGauge(), X, BaseY + 60.0f, 300.0f, 18.0f, FLinearColor(0.00f, 0.58f, 0.95f, 1.0f));
	const AAetherGameModeBase* AetherGameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	const FString CombatStateLabel = FAetherCombatHudPresenter::BuildCombatStateLabel(CombatComponent, HealthComponent, AetherGameMode);
	DrawText(CombatStateLabel, FLinearColor(0.86f, 0.90f, 0.95f, 1.0f), X, BaseY + 88.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.95f);
	DrawText(FAetherCombatHudPresenter::BuildAetherSlashStatusLabel(CombatComponent), FLinearColor(0.36f, 0.86f, 1.0f, 1.0f), X, BaseY + 108.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f);
}

void AAetherHUD::DrawPrototypeQuickItemStatus(const AAetherfallCharacter* PlayerCharacter)
{
	const UAetherInventoryComponent* InventoryComponent = PlayerCharacter ? PlayerCharacter->GetInventoryComponent() : nullptr;
	if (!Canvas || !InventoryComponent)
	{
		return;
	}

	const FString QuickItemLabel = FString::Printf(
		TEXT("QUICK ITEM [H] HEAL x%d"),
		InventoryComponent->GetPrototypeHealingItemCount());

	constexpr float X = 32.0f;
	const float Y = Canvas->SizeY - 20.0f;
	DrawText(QuickItemLabel, FLinearColor(0.98f, 0.86f, 0.42f, 1.0f), X, Y, GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f);
}

void AAetherHUD::DrawPlayerDangerStatus(const AAetherfallCharacter* PlayerCharacter)
{
	const UAetherHealthComponent* HealthComponent = PlayerCharacter ? PlayerCharacter->GetHealthComponent() : nullptr;
	if (!Canvas || !HealthComponent)
	{
		return;
	}

	const AAetherGameModeBase* AetherGameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	const FAetherPlayerDangerViewData DangerViewData = FAetherCombatHudPresenter::BuildPlayerDangerViewData(HealthComponent, AetherGameMode);
	if (!DangerViewData.bShouldDisplay)
	{
		return;
	}

	constexpr float Width = 420.0f;
	constexpr float Height = 30.0f;
	const float X = (Canvas->SizeX - Width) * 0.5f;
	const float Y = Canvas->SizeY - 218.0f;

	DrawRect(FLinearColor(0.02f, 0.02f, 0.025f, 0.78f), X - 4.0f, Y - 3.0f, Width + 8.0f, Height + 6.0f);
	DrawRect(FLinearColor(0.16f, 0.02f, 0.02f, 0.70f), X, Y, Width, Height);
	DrawRect(DangerViewData.Color.CopyWithNewOpacity(0.95f), X, Y, 5.0f, Height);
	DrawText(DangerViewData.Label, DangerViewData.Color, X + 14.0f, Y + 4.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f);
}

void AAetherHUD::DrawEnemyStatus(const AAetherfallCharacter* PlayerCharacter)
{
	const UAetherLockOnComponent* LockOnComponent = PlayerCharacter ? PlayerCharacter->GetLockOnComponent() : nullptr;
	const UAetherCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	const bool bHasLockedTarget = LockOnComponent && LockOnComponent->IsLockedOn();
	const AAetherEnemyBase* Enemy = bHasLockedTarget ? LockOnComponent->GetLockedTarget() : FindNearestLivingEnemy(PlayerCharacter);
	const UAetherHealthComponent* EnemyHealth = Enemy ? Enemy->GetHealthComponent() : nullptr;
	if (!EnemyHealth)
	{
		return;
	}

	if (Enemy->GetEnemyArchetype() == EAetherEnemyArchetype::Aurel)
	{
		DrawBossEnemyStatus(Enemy, bHasLockedTarget, CombatComponent);
		return;
	}

	constexpr float Width = 360.0f;
	constexpr float Height = 18.0f;
	const float X = (Canvas->SizeX - Width) * 0.5f;
	constexpr float Y = 42.0f;

	const bool bExecutionReady = CombatComponent && CombatComponent->IsEnemyExecutionReady(Enemy);
	const FString TargetPrefix = bExecutionReady ? TEXT("EXECUTION READY") : (Enemy->IsParryStaggered() ? TEXT("STAGGERED") : (bHasLockedTarget ? TEXT("LOCKED") : TEXT("TARGET")));
	const FString EnemyLabel = FString::Printf(TEXT("%s %s / %s"), *TargetPrefix, *Enemy->GetEnemyArchetypeName().ToString().ToUpper(), *Enemy->GetEnemyCombatRoleLabel().ToString().ToUpper());
	const FLinearColor ArchetypeColor = Enemy->GetEnemyArchetypeColor();
	DrawText(EnemyLabel, ArchetypeColor, X, Y - 24.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 1.0f);
	DrawStatusBar(TEXT("HP"), EnemyHealth->GetCurrentHealth(), EnemyHealth->GetMaxHealth(), X, Y, Width, Height, ArchetypeColor);

	if (bHasLockedTarget)
	{
		DrawLockOnReticle(Enemy);
	}
}

void AAetherHUD::DrawBossEnemyStatus(const AAetherEnemyBase* Enemy, bool bHasLockedTarget, const UAetherCombatComponent* CombatComponent)
{
	const UAetherHealthComponent* EnemyHealth = Enemy ? Enemy->GetHealthComponent() : nullptr;
	if (!Canvas || !EnemyHealth)
	{
		return;
	}

	const float LeftReserve = Canvas->SizeX >= 920.0f ? 424.0f : 32.0f;
	const float AvailableWidth = FMath::Max(320.0f, Canvas->SizeX - LeftReserve - 32.0f);
	const float Width = FMath::Min(760.0f, AvailableWidth);
	constexpr float Height = 24.0f;
	const float X = LeftReserve + (AvailableWidth - Width) * 0.5f;
	constexpr float Y = 34.0f;

	const bool bExecutionReady = CombatComponent && CombatComponent->IsEnemyExecutionReady(Enemy);
	const FString TargetPrefix = bExecutionReady ? TEXT("EXECUTION READY") : (Enemy->IsParryStaggered() ? TEXT("STAGGERED") : (bHasLockedTarget ? TEXT("LOCKED") : TEXT("BOSS")));
	const FString PhaseSuffix = Enemy->IsBossPhaseTwo() ? TEXT(" / PHASE II") : (Enemy->IsBossPhaseOne() ? TEXT(" / PHASE I") : TEXT(""));
	const FString BossLabel = FString::Printf(TEXT("%s AUREL, THE HOLLOW WARDEN%s"), *TargetPrefix, *PhaseSuffix);
	const FLinearColor BossColor = Enemy->GetEnemyArchetypeColor();
	const float HealthPercent = FMath::Clamp(EnemyHealth->GetCurrentHealth() / FMath::Max(EnemyHealth->GetMaxHealth(), 1.0f), 0.0f, 1.0f);

	DrawRect(FLinearColor(0.01f, 0.018f, 0.025f, 0.86f), X - 6.0f, Y - 28.0f, Width + 12.0f, Height + 44.0f);
	DrawRect(FLinearColor(0.025f, 0.045f, 0.060f, 0.92f), X, Y, Width, Height);
	DrawRect(BossColor.CopyWithNewOpacity(0.30f), X, Y, Width * HealthPercent, Height);
	DrawRect(BossColor.CopyWithNewOpacity(0.96f), X, Y, 6.0f, Height);
	DrawRect(FLinearColor(0.90f, 0.96f, 1.0f, 0.36f), X, Y, Width, 1.0f);
	DrawText(BossLabel, BossColor, X + 8.0f, Y - 23.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 1.05f);
	DrawText(
		FString::Printf(TEXT("HP %.0f / %.0f"), EnemyHealth->GetCurrentHealth(), EnemyHealth->GetMaxHealth()),
		FLinearColor::White,
		X + 12.0f,
		Y + 2.0f,
		GEngine ? GEngine->GetSmallFont() : nullptr,
		0.88f);

	if (bHasLockedTarget)
	{
		DrawLockOnReticle(Enemy);
	}
}

void AAetherHUD::DrawIncomingThreatStatus(const AAetherfallCharacter* PlayerCharacter)
{
	const AAetherEnemyBase* ThreatEnemy = FindIncomingThreatEnemy(PlayerCharacter);
	if (!ThreatEnemy)
	{
		return;
	}

	const UAetherLockOnComponent* LockOnComponent = PlayerCharacter ? PlayerCharacter->GetLockOnComponent() : nullptr;
	const bool bThreatIsLocked = LockOnComponent && LockOnComponent->GetLockedTarget() == ThreatEnemy;
	const FName DefenseHint = ThreatEnemy->GetActiveAttackDefenseHintLabel();
	const FString DefenseHintSuffix = DefenseHint.IsNone()
		? TEXT("")
		: FString::Printf(TEXT(" / %s"), *DefenseHint.ToString().ToUpper());
	const FString ThreatLabel = FString::Printf(
		TEXT("INCOMING %s / %s%s%s"),
		*ThreatEnemy->GetEnemyArchetypeName().ToString().ToUpper(),
		*ThreatEnemy->GetActiveAttackPatternName().ToString().ToUpper(),
		*DefenseHintSuffix,
		bThreatIsLocked ? TEXT("") : TEXT(" / OFF-LOCK"));

	constexpr float Width = 360.0f;
	constexpr float Height = 18.0f;
	const float X = (Canvas->SizeX - Width) * 0.5f;
	constexpr float Y = 76.0f;
	const FLinearColor ThreatColor = ThreatEnemy->GetEnemyArchetypeColor();

	DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.76f), X - 4.0f, Y - 3.0f, Width + 8.0f, Height + 6.0f);
	DrawRect(FLinearColor(0.18f, 0.03f, 0.03f, 0.55f), X, Y, Width, Height);
	DrawRect(ThreatColor.CopyWithNewOpacity(0.95f), X, Y, 4.0f, Height);
	DrawText(ThreatLabel, ThreatColor, X + 10.0f, Y - 1.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f);

	DrawThreatReticle(ThreatEnemy);
}

void AAetherHUD::DrawPrototypeRoundStatus()
{
	const AAetherGameModeBase* AetherGameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!Canvas || !AetherGameMode || !AetherGameMode->IsPrototypeCombatRoundGoalEnabled())
	{
		return;
	}

	const int32 DefeatCount = AetherGameMode->GetPrototypeRoundDefeatCount();
	const int32 KillGoal = FMath::Max(AetherGameMode->GetPrototypeRoundKillGoal(), 1);
	const int32 LivingEnemies = AetherGameMode->GetLivingPrototypeEnemyCountForHUD();
	const bool bRoundComplete = AetherGameMode->IsPrototypeCombatRoundComplete();
	const bool bGoalReached = DefeatCount >= KillGoal;

	FString RoundLabel;
	FLinearColor RoundColor;
	if (AetherGameMode->IsPrototypeBossEncounterActive())
	{
		RoundLabel = TEXT("BOSS ENCOUNTER - DEFEAT AUREL");
		RoundColor = FLinearColor(0.18f, 0.76f, 1.0f, 1.0f);
	}
	else if (bRoundComplete && AetherGameMode->GetLastCompletedPrototypeEncounterLabel() == AetherGameMode->GetPrototypeBossEncounterLabel())
	{
		RoundLabel = TEXT("BOSS CLEAR - RIFT SEAL AWAITS");
		RoundColor = FLinearColor(0.18f, 0.95f, 0.95f, 1.0f);
	}
	else if (bRoundComplete)
	{
		RoundLabel = AetherGameMode->IsPrototypeLevelComplete()
			? TEXT("ROUND CLEAR - LEVEL COMPLETE")
			: TEXT("ROUND CLEAR - PROCEED TO NEXT AREA");
		RoundColor = FLinearColor(0.18f, 0.95f, 0.45f, 1.0f);
	}
	else if (bGoalReached)
	{
		RoundLabel = FString::Printf(TEXT("ROUND GOAL MET - CLEAR REMAINING %d"), LivingEnemies);
		RoundColor = FLinearColor(1.0f, 0.76f, 0.18f, 1.0f);
	}
	else
	{
		RoundLabel = FString::Printf(TEXT("ROUND KILLS %d / %d"), DefeatCount, KillGoal);
		RoundColor = FLinearColor(0.76f, 0.88f, 1.0f, 1.0f);
	}

	constexpr float X = 32.0f;
	constexpr float Y = 32.0f;
	constexpr float Width = 360.0f;
	constexpr float Height = 24.0f;
	const float Percent = bRoundComplete ? 1.0f : FMath::Clamp(static_cast<float>(DefeatCount) / static_cast<float>(KillGoal), 0.0f, 1.0f);

	DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.78f), X - 4.0f, Y - 3.0f, Width + 8.0f, Height + 6.0f);
	DrawRect(FLinearColor(0.08f, 0.09f, 0.11f, 0.86f), X, Y, Width, Height);
	DrawRect(RoundColor.CopyWithNewOpacity(0.40f), X, Y, Width * Percent, Height);
	DrawRect(RoundColor.CopyWithNewOpacity(0.95f), X, Y, 4.0f, Height);
	DrawText(RoundLabel, RoundColor, X + 12.0f, Y + 2.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f);

	if (bRoundComplete && !AetherGameMode->IsPrototypeLevelComplete() && AetherGameMode->ShouldShowPrototypeRoundClearBanner())
	{
		constexpr float BannerWidth = 560.0f;
		constexpr float BannerHeight = 82.0f;
		const float BannerX = (Canvas->SizeX - BannerWidth) * 0.5f;
		const float BannerY = Canvas->SizeY * 0.22f;
		const FString DetailLabel = FString::Printf(
			TEXT("DEFEATED %d / %d - NEXT ROUTE OPEN / R RESTART"),
			DefeatCount,
			KillGoal);
		const float RemainingTime = AetherGameMode->GetPrototypeRoundClearBannerRemainingTime();
		const float BannerAlpha = FMath::Clamp(0.35f + RemainingTime * 0.20f, 0.35f, 1.0f);
		const FLinearColor BannerTextColor = RoundColor.CopyWithNewOpacity(BannerAlpha);

		DrawRect(FLinearColor(0.015f, 0.025f, 0.018f, 0.84f * BannerAlpha), BannerX - 6.0f, BannerY - 5.0f, BannerWidth + 12.0f, BannerHeight + 10.0f);
		DrawRect(FLinearColor(0.03f, 0.12f, 0.07f, 0.80f * BannerAlpha), BannerX, BannerY, BannerWidth, BannerHeight);
		DrawRect(RoundColor.CopyWithNewOpacity(0.95f * BannerAlpha), BannerX, BannerY, 6.0f, BannerHeight);
		DrawRect(RoundColor.CopyWithNewOpacity(0.18f * BannerAlpha), BannerX, BannerY, BannerWidth, BannerHeight);
		DrawText(TEXT("ROUND CLEAR"), BannerTextColor, BannerX + 28.0f, BannerY + 12.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 1.55f);
		DrawText(DetailLabel, FLinearColor(0.88f, 0.96f, 0.90f, BannerAlpha), BannerX + 30.0f, BannerY + 52.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.98f);
	}
}

void AAetherHUD::DrawPrototypeLevelStatus()
{
	const AAetherGameModeBase* AetherGameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!Canvas || !AetherGameMode || !AetherGameMode->IsPrototypeLevelComplete() || !AetherGameMode->ShouldShowPrototypeLevelCompleteBanner())
	{
		return;
	}

	constexpr float BannerWidth = 620.0f;
	constexpr float BannerHeight = 92.0f;
	const float BannerX = (Canvas->SizeX - BannerWidth) * 0.5f;
	const float BannerY = Canvas->SizeY * 0.22f;
	const FLinearColor BannerColor(0.18f, 0.92f, 0.95f, 1.0f);
	const FName GoalLabel = AetherGameMode->GetCompletedPrototypeLevelGoalLabel();
	const bool bCathedralEnding = GoalLabel == AetherGameMode->GetPrototypeCathedralEndingGoalLabel();
	const FString TitleLabel = bCathedralEnding
		? TEXT("AETHER RIFT SEALED")
		: TEXT("PROTOTYPE LEVEL COMPLETE");
	const FString DetailLabel = bCathedralEnding
		? TEXT("THE RUINED KINGDOM SLEEPS")
		: (GoalLabel.IsNone()
		? TEXT("PROTOTYPE ROUTE COMPLETE")
		: FString::Printf(TEXT("GOAL %s REACHED"), *GoalLabel.ToString().ToUpper()));
	const float RemainingTime = AetherGameMode->GetPrototypeLevelCompleteBannerRemainingTime();
	const float BannerAlpha = FMath::Clamp(0.35f + RemainingTime * 0.16f, 0.35f, 1.0f);
	const FLinearColor BannerTextColor = BannerColor.CopyWithNewOpacity(BannerAlpha);

	DrawRect(FLinearColor(0.01f, 0.03f, 0.04f, 0.86f * BannerAlpha), BannerX - 6.0f, BannerY - 5.0f, BannerWidth + 12.0f, BannerHeight + 10.0f);
	DrawRect(FLinearColor(0.02f, 0.11f, 0.13f, 0.82f * BannerAlpha), BannerX, BannerY, BannerWidth, BannerHeight);
	DrawRect(BannerColor.CopyWithNewOpacity(0.95f * BannerAlpha), BannerX, BannerY, 6.0f, BannerHeight);
	DrawRect(BannerColor.CopyWithNewOpacity(0.16f * BannerAlpha), BannerX, BannerY, BannerWidth, BannerHeight);
	DrawText(TitleLabel, BannerTextColor, BannerX + 28.0f, BannerY + 12.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 1.48f);
	DrawText(DetailLabel, FLinearColor(0.88f, 0.97f, 0.99f, BannerAlpha), BannerX + 30.0f, BannerY + 56.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.98f);
}

void AAetherHUD::DrawPrototypeCheckpointStatus()
{
	const AAetherGameModeBase* AetherGameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!Canvas || !AetherGameMode)
	{
		return;
	}

	const bool bHasCheckpoint = AetherGameMode->HasActivePrototypeCheckpoint();
	const FName CheckpointLabel = AetherGameMode->GetActivePrototypeCheckpointLabel();
	const FString CheckpointName = !CheckpointLabel.IsNone()
		? CheckpointLabel.ToString().ToUpper()
		: TEXT("ACTIVE");
	const FString StatusLabel = bHasCheckpoint
		? FString::Printf(TEXT("CHECKPOINT %s / RANK %d"), *CheckpointName, AetherGameMode->GetActivePrototypeCheckpointProgressRank())
		: TEXT("CHECKPOINT NONE");
	const FString HintLabel = bHasCheckpoint
		? TEXT("[BACKSPACE] CLEAR CHECKPOINT")
		: TEXT("[BACKSPACE] CLEAR PROGRESS");
	const FString ProgressLabel = GetPrototypeProgressStatusLabel(AetherGameMode);
	const FString PacingLabel = AetherGameMode->GetPrototypeRoutePacingStatusLabel();
	const FLinearColor StatusColor = bHasCheckpoint
		? FLinearColor(0.42f, 0.86f, 1.0f, 1.0f)
		: FLinearColor(0.70f, 0.76f, 0.84f, 1.0f);
	const FLinearColor ProgressColor = AetherGameMode->IsPrototypeLevelComplete()
		? FLinearColor(0.18f, 0.92f, 0.95f, 1.0f)
		: (AetherGameMode->IsPrototypeCombatRoundComplete()
			? FLinearColor(0.28f, 0.96f, 0.48f, 1.0f)
			: FLinearColor(0.78f, 0.84f, 0.92f, 1.0f));
	const FLinearColor AccentColor = bHasCheckpoint
		? FLinearColor(0.06f, 0.62f, 0.96f, 0.95f)
		: FLinearColor(0.36f, 0.40f, 0.46f, 0.95f);

	constexpr float X = 32.0f;
	constexpr float Y = 66.0f;
	constexpr float Width = 420.0f;
	constexpr float Height = 86.0f;

	DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.74f), X - 4.0f, Y - 3.0f, Width + 8.0f, Height + 6.0f);
	DrawRect(FLinearColor(0.07f, 0.08f, 0.10f, 0.80f), X, Y, Width, Height);
	DrawRect(AccentColor, X, Y, 4.0f, Height);
	DrawText(StatusLabel, StatusColor, X + 12.0f, Y + 5.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f);
	DrawText(HintLabel, FLinearColor(0.92f, 0.80f, 0.46f, 1.0f), X + 12.0f, Y + 26.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f);
	DrawText(ProgressLabel, ProgressColor, X + 12.0f, Y + 46.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f);
	DrawText(PacingLabel, FLinearColor(0.80f, 0.88f, 0.98f, 1.0f), X + 12.0f, Y + 66.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.70f);
}

void AAetherHUD::DrawPrototypeCheckpointFeedback()
{
	const AAetherGameModeBase* AetherGameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!Canvas || !AetherGameMode || !AetherGameMode->HasPrototypeCheckpointFeedback())
	{
		return;
	}

	const float RemainingTime = AetherGameMode->GetPrototypeCheckpointFeedbackRemainingTime();
	if (RemainingTime <= 0.0f)
	{
		return;
	}

	const FString FeedbackLabel = AetherGameMode->GetPrototypeCheckpointFeedbackLabel();
	FLinearColor FeedbackColor = AetherGameMode->GetPrototypeCheckpointFeedbackColor();
	const float FadeAlpha = FMath::Clamp(0.35f + RemainingTime * 0.32f, 0.35f, 0.96f);
	FeedbackColor.A = FadeAlpha;

	constexpr float X = 32.0f;
	constexpr float Y = 140.0f;
	constexpr float Width = 360.0f;
	constexpr float Height = 28.0f;

	DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.68f * FadeAlpha), X - 4.0f, Y - 3.0f, Width + 8.0f, Height + 6.0f);
	DrawRect(FLinearColor(0.06f, 0.08f, 0.10f, 0.74f * FadeAlpha), X, Y, Width, Height);
	DrawRect(FeedbackColor, X, Y, 4.0f, Height);
	DrawText(FeedbackLabel, FeedbackColor, X + 12.0f, Y + 4.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.84f);
}

void AAetherHUD::DrawPrototypeProgressFeedback()
{
	const AAetherGameModeBase* AetherGameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!Canvas || !AetherGameMode || !AetherGameMode->HasPrototypeProgressFeedback())
	{
		return;
	}

	const float RemainingTime = AetherGameMode->GetPrototypeProgressFeedbackRemainingTime();
	if (RemainingTime <= 0.0f)
	{
		return;
	}

	const FString FeedbackLabel = AetherGameMode->GetPrototypeProgressFeedbackLabel();
	FLinearColor FeedbackColor = AetherGameMode->GetPrototypeProgressFeedbackColor();
	const float FadeAlpha = FMath::Clamp(0.35f + RemainingTime * 0.32f, 0.35f, 0.96f);
	FeedbackColor.A = FadeAlpha;

	constexpr float X = 32.0f;
	constexpr float Y = 172.0f;
	constexpr float Width = 360.0f;
	constexpr float Height = 28.0f;

	DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.68f * FadeAlpha), X - 4.0f, Y - 3.0f, Width + 8.0f, Height + 6.0f);
	DrawRect(FLinearColor(0.06f, 0.08f, 0.10f, 0.74f * FadeAlpha), X, Y, Width, Height);
	DrawRect(FeedbackColor, X, Y, 4.0f, Height);
	DrawText(FeedbackLabel, FeedbackColor, X + 12.0f, Y + 4.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f);
}

void AAetherHUD::DrawPrototypeDialogue()
{
	const AAetherGameModeBase* AetherGameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	const UAetherSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr;
	const FAetherCustomSettings CustomSettings = Settings ? Settings->GetCurrentSettings().Custom : FAetherCustomSettings();
	if (!Canvas || !AetherGameMode || !AetherGameMode->IsPrototypeDialogueActive() || !CustomSettings.bSubtitlesEnabled)
	{
		return;
	}

	const FString SpeakerLabel = AetherGameMode->GetPrototypeDialogueSpeakerName().ToString().ToUpper();
	const FString DialogueLabel = AetherGameMode->GetPrototypeDialogueText().ToString();
	const FString ObjectiveHintLabel = AetherGameMode->GetPrototypeDialogueObjectiveHint().ToString();
	if (DialogueLabel.IsEmpty())
	{
		return;
	}

	const float SubtitleScale = CustomSettings.SubtitleSize == EAetherSubtitleSize::Small
		? 0.82f
		: (CustomSettings.SubtitleSize == EAetherSubtitleSize::Large ? 1.12f : 0.94f);
	const float Width = FMath::Clamp(Canvas->SizeX - 96.0f, 420.0f, 820.0f);
	const float Height = CustomSettings.SubtitleSize == EAetherSubtitleSize::Large ? 112.0f : 98.0f;
	const float X = (Canvas->SizeX - Width) * 0.5f;
	const float Y = Canvas->SizeY - 252.0f;
	const FLinearColor AccentColor(0.38f, 0.78f, 1.0f, 0.96f);

	DrawRect(FLinearColor(0.01f, 0.015f, 0.020f, 0.82f), X - 6.0f, Y - 5.0f, Width + 12.0f, Height + 10.0f);
	DrawRect(FLinearColor(0.035f, 0.045f, 0.055f, 0.88f), X, Y, Width, Height);
	DrawRect(AccentColor, X, Y, 5.0f, Height);
	DrawRect(FLinearColor(0.90f, 0.95f, 1.0f, 0.18f), X, Y, Width, 1.0f);
	DrawText(SpeakerLabel, AccentColor, X + 16.0f, Y + 10.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.88f);
	DrawText(DialogueLabel, FLinearColor(0.94f, 0.96f, 0.98f, 1.0f), X + 16.0f, Y + 38.0f, GEngine ? GEngine->GetSmallFont() : nullptr, SubtitleScale);

	if (!ObjectiveHintLabel.IsEmpty())
	{
		DrawText(ObjectiveHintLabel, FLinearColor(0.86f, 0.92f, 0.72f, 1.0f), X + 16.0f, Y + 68.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.76f);
	}
}

void AAetherHUD::DrawPrototypeRouteGuidance(const AAetherfallCharacter* PlayerCharacter)
{
	const AAetherGameModeBase* AetherGameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!Canvas || !AetherGameMode)
	{
		return;
	}

	const FAetherPrototypeRouteGuidanceViewData Guidance = FAetherPrototypeRouteGuide::BuildGuidance(AetherGameMode, PlayerCharacter);
	if (!Guidance.HasGuidance())
	{
		return;
	}

	constexpr float X = 32.0f;
	constexpr float Y = 206.0f;
	const float Width = FMath::Clamp(Canvas->SizeX - 64.0f, 360.0f, 560.0f);
	constexpr float Height = 58.0f;
	const FLinearColor AccentColor = Guidance.bCriticalObjective
		? FLinearColor(0.44f, 0.72f, 1.0f, 0.95f)
		: FLinearColor(0.72f, 0.86f, 0.48f, 0.95f);
	const FLinearColor ObjectiveColor = Guidance.bCriticalObjective
		? FLinearColor(0.62f, 0.86f, 1.0f, 1.0f)
		: FLinearColor(0.86f, 0.95f, 0.70f, 1.0f);

	DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.70f), X - 4.0f, Y - 3.0f, Width + 8.0f, Height + 6.0f);
	DrawRect(FLinearColor(0.055f, 0.065f, 0.075f, 0.78f), X, Y, Width, Height);
	DrawRect(AccentColor, X, Y, 4.0f, Height);
	DrawText(Guidance.ObjectiveLabel, ObjectiveColor, X + 12.0f, Y + 6.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f);
	DrawText(Guidance.TutorialHintLabel, FLinearColor(0.88f, 0.92f, 0.96f, 1.0f), X + 12.0f, Y + 32.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f);
}

void AAetherHUD::DrawInteractionPrompt(const AAetherfallCharacter* PlayerCharacter)
{
	const UAetherInteractionComponent* InteractionComponent = PlayerCharacter ? PlayerCharacter->GetInteractionComponent() : nullptr;
	if (!Canvas || !InteractionComponent)
	{
		return;
	}

	const FText PromptText = InteractionComponent->GetFocusedInteractionPrompt();
	if (PromptText.IsEmpty())
	{
		return;
	}

	const FString PromptLabel = FString::Printf(TEXT("[V] %s"), *PromptText.ToString());
	constexpr float Width = 360.0f;
	constexpr float Height = 28.0f;
	const float X = (Canvas->SizeX - Width) * 0.5f;
	const float Y = Canvas->SizeY - 108.0f;

	DrawRect(FLinearColor(0.015f, 0.02f, 0.025f, 0.78f), X - 4.0f, Y - 3.0f, Width + 8.0f, Height + 6.0f);
	DrawRect(FLinearColor(0.07f, 0.08f, 0.10f, 0.84f), X, Y, Width, Height);
	DrawRect(FLinearColor(0.96f, 0.76f, 0.20f, 0.92f), X, Y, 4.0f, Height);
	DrawText(PromptLabel, FLinearColor(0.96f, 0.92f, 0.76f, 1.0f), X + 12.0f, Y + 3.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.92f);
}

void AAetherHUD::DrawLockOnReticle(const AAetherEnemyBase* Enemy)
{
	if (!Enemy || !PlayerOwner)
	{
		return;
	}

	FVector2D ScreenLocation;
	const FVector TargetLocation = Enemy->GetActorLocation() + FVector(0.0f, 0.0f, 110.0f);
	if (!PlayerOwner->ProjectWorldLocationToScreen(TargetLocation, ScreenLocation))
	{
		return;
	}

	constexpr float ReticleHalfSize = 22.0f;
	constexpr float SegmentLength = 10.0f;
	const FLinearColor ReticleColor(0.0f, 0.78f, 1.0f, 0.95f);
	const float Left = ScreenLocation.X - ReticleHalfSize;
	const float Right = ScreenLocation.X + ReticleHalfSize;
	const float Top = ScreenLocation.Y - ReticleHalfSize;
	const float Bottom = ScreenLocation.Y + ReticleHalfSize;

	DrawLine(Left, Top, Left + SegmentLength, Top, ReticleColor, 2.0f);
	DrawLine(Left, Top, Left, Top + SegmentLength, ReticleColor, 2.0f);
	DrawLine(Right, Top, Right - SegmentLength, Top, ReticleColor, 2.0f);
	DrawLine(Right, Top, Right, Top + SegmentLength, ReticleColor, 2.0f);
	DrawLine(Left, Bottom, Left + SegmentLength, Bottom, ReticleColor, 2.0f);
	DrawLine(Left, Bottom, Left, Bottom - SegmentLength, ReticleColor, 2.0f);
	DrawLine(Right, Bottom, Right - SegmentLength, Bottom, ReticleColor, 2.0f);
	DrawLine(Right, Bottom, Right, Bottom - SegmentLength, ReticleColor, 2.0f);
}

void AAetherHUD::DrawThreatReticle(const AAetherEnemyBase* Enemy)
{
	if (!Enemy || !PlayerOwner)
	{
		return;
	}

	FVector2D ScreenLocation;
	const FVector TargetLocation = Enemy->GetActorLocation() + FVector(0.0f, 0.0f, 145.0f);
	if (!PlayerOwner->ProjectWorldLocationToScreen(TargetLocation, ScreenLocation))
	{
		return;
	}

	constexpr float ReticleHalfSize = 30.0f;
	constexpr float SegmentLength = 14.0f;
	const FLinearColor ThreatColor = Enemy->GetEnemyArchetypeColor().CopyWithNewOpacity(0.95f);
	const float Left = ScreenLocation.X - ReticleHalfSize;
	const float Right = ScreenLocation.X + ReticleHalfSize;
	const float Top = ScreenLocation.Y - ReticleHalfSize;
	const float Bottom = ScreenLocation.Y + ReticleHalfSize;

	DrawText(TEXT("!"), ThreatColor, ScreenLocation.X - 4.0f, Top - 22.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 1.25f);
	DrawLine(Left, Top, Left + SegmentLength, Top, ThreatColor, 3.0f);
	DrawLine(Left, Top, Left, Top + SegmentLength, ThreatColor, 3.0f);
	DrawLine(Right, Top, Right - SegmentLength, Top, ThreatColor, 3.0f);
	DrawLine(Right, Top, Right, Top + SegmentLength, ThreatColor, 3.0f);
	DrawLine(Left, Bottom, Left + SegmentLength, Bottom, ThreatColor, 3.0f);
	DrawLine(Left, Bottom, Left, Bottom - SegmentLength, ThreatColor, 3.0f);
	DrawLine(Right, Bottom, Right - SegmentLength, Bottom, ThreatColor, 3.0f);
	DrawLine(Right, Bottom, Right, Bottom - SegmentLength, ThreatColor, 3.0f);
}

void AAetherHUD::DrawStatusBar(const FString& Label, float CurrentValue, float MaxValue, float X, float Y, float Width, float Height, const FLinearColor& FillColor)
{
	const float SafeMaxValue = FMath::Max(MaxValue, 1.0f);
	const float Percent = FMath::Clamp(CurrentValue / SafeMaxValue, 0.0f, 1.0f);

	DrawRect(FLinearColor(0.02f, 0.025f, 0.03f, 0.82f), X - 2.0f, Y - 2.0f, Width + 4.0f, Height + 4.0f);
	DrawRect(FLinearColor(0.11f, 0.12f, 0.13f, 0.92f), X, Y, Width, Height);
	DrawRect(FillColor, X, Y, Width * Percent, Height);
	DrawRect(FLinearColor(0.86f, 0.88f, 0.90f, 0.35f), X, Y, Width, 1.0f);

	const FString ValueText = FString::Printf(TEXT("%s %.0f / %.0f"), *Label, CurrentValue, MaxValue);
	DrawText(ValueText, FLinearColor::White, X + 8.0f, Y - 1.0f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.82f);
}

FString AAetherHUD::GetPrototypeProgressStatusLabel(const AAetherGameModeBase* AetherGameMode) const
{
	if (!AetherGameMode)
	{
		return TEXT("PROGRESS UNKNOWN");
	}

	if (AetherGameMode->IsPrototypeLevelComplete())
	{
		if (AetherGameMode->IsCathedralEndingComplete())
		{
			return TEXT("PROGRESS CATHEDRAL ENDING SEALED");
		}

		const FName GoalLabel = AetherGameMode->GetCompletedPrototypeLevelGoalLabel();
		return GoalLabel.IsNone()
			? TEXT("PROGRESS LEVEL COMPLETE")
			: FString::Printf(TEXT("PROGRESS LEVEL COMPLETE %s"), *GoalLabel.ToString().ToUpper());
	}

	if (AetherGameMode->IsPrototypeCombatRoundComplete())
	{
		const FName EncounterLabel = AetherGameMode->GetLastCompletedPrototypeEncounterLabel();
		return EncounterLabel.IsNone()
			? TEXT("PROGRESS ROUND CLEAR")
			: FString::Printf(TEXT("PROGRESS ENCOUNTER CLEAR %s"), *EncounterLabel.ToString().ToUpper());
	}

	const FName ActiveEncounterLabel = AetherGameMode->GetActivePrototypeEncounterLabel();
	if (!ActiveEncounterLabel.IsNone())
	{
		return FString::Printf(TEXT("PROGRESS ENCOUNTER %s"), *ActiveEncounterLabel.ToString().ToUpper());
	}

	return TEXT("PROGRESS EXPLORING");
}

AAetherEnemyBase* AAetherHUD::FindNearestLivingEnemy(const AAetherfallCharacter* PlayerCharacter) const
{
	if (!PlayerCharacter)
	{
		return nullptr;
	}

	AAetherEnemyBase* NearestEnemy = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	const FVector PlayerLocation = PlayerCharacter->GetActorLocation();

	for (TActorIterator<AAetherEnemyBase> EnemyIt(GetWorld()); EnemyIt; ++EnemyIt)
	{
		AAetherEnemyBase* Enemy = *EnemyIt;
		const UAetherHealthComponent* HealthComponent = Enemy ? Enemy->GetHealthComponent() : nullptr;
		if (!HealthComponent || HealthComponent->IsDead())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(PlayerLocation, Enemy->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			NearestEnemy = Enemy;
		}
	}

	return NearestEnemy;
}

AAetherEnemyBase* AAetherHUD::FindIncomingThreatEnemy(const AAetherfallCharacter* PlayerCharacter) const
{
	if (!PlayerCharacter)
	{
		return nullptr;
	}

	AAetherEnemyBase* ThreatEnemy = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	const FVector PlayerLocation = PlayerCharacter->GetActorLocation();

	for (TActorIterator<AAetherEnemyBase> EnemyIt(GetWorld()); EnemyIt; ++EnemyIt)
	{
		AAetherEnemyBase* Enemy = *EnemyIt;
		const UAetherHealthComponent* HealthComponent = Enemy ? Enemy->GetHealthComponent() : nullptr;
		if (!HealthComponent || HealthComponent->IsDead() || !Enemy->IsAttackWindingUpForHUD())
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(PlayerLocation, Enemy->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			ThreatEnemy = Enemy;
		}
	}

	return ThreatEnemy;
}
