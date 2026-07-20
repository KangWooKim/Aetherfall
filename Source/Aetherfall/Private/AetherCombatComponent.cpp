#include "AetherCombatComponent.h"

#include "AetherAudioSettingsLibrary.h"
#include "AetherCombatActionDataAsset.h"
#include "AetherCombatActionExecutionPolicy.h"
#include "AetherCombatResourcePolicy.h"
#include "AetherCombatActionStatePolicy.h"
#include "AetherCombatActionTimerPolicy.h"
#include "AetherCombatDamagePolicy.h"
#include "AetherCombatFeedbackPolicy.h"
#include "AetherCombatActionTuningPolicy.h"
#include "AetherCombatTargetSelectionPolicy.h"
#include "AetherCombatTracePolicy.h"
#include "AetherEnemyBase.h"
#include "AetherGameModeBase.h"
#include "AetherHealthComponent.h"
#include "AetherLockOnComponent.h"
#include "AetherProjectilePoolSubsystem.h"
#include "AetherSlashProjectile.h"
#include "AetherfallCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

UAetherCombatComponent::UAetherCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	LightAttackStaminaCosts = { 18.0f, 20.0f, 22.0f, 26.0f };
	LightAttackDamageValues = { 20.0f, 22.0f, 24.0f, 30.0f };
}

void UAetherCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AAetherfallCharacter>(GetOwner());
	ApplyCombatResourceMutationPlan(FAetherCombatResourcePolicy::BuildInitializePlan(MaxStamina));

	if (OwnerCharacter.IsValid() && OwnerCharacter->GetHealthComponent())
	{
		OwnerCharacter->GetHealthComponent()->OnDeath.AddDynamic(this, &UAetherCombatComponent::HandleOwnerDeath);
	}
}

void UAetherCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateDodgeMovement(DeltaTime);
	RegenerateStamina(DeltaTime);
}

float UAetherCombatComponent::GetAetherSlashCost() const
{
	return ResolveAetherSlashCost();
}

float UAetherCombatComponent::GetAetherSlashCooldownRemaining() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	const float RemainingCooldown = ResolveAetherSlashCooldown() - static_cast<float>(World->GetTimeSeconds() - LastAetherSlashTime);
	return FMath::Max(0.0f, RemainingCooldown);
}

bool UAetherCombatComponent::IsAetherSlashReady() const
{
	return CurrentAetherGauge >= ResolveAetherSlashCost() && GetAetherSlashCooldownRemaining() <= 0.0f && !bIsAetherSlashing;
}

void UAetherCombatComponent::NotifyOwnerHealed()
{
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	const UAetherHealthComponent* HealthComponent = Character ? Character->GetHealthComponent() : nullptr;
	if (!HealthComponent)
	{
		return;
	}

	FAetherCombatAudioCuePolicy::RefreshPlayerDangerStateAfterHeal(HealthComponent, BuildPlayerDangerCueConfig(), PlayerDangerCueState);
}

bool UAetherCombatComponent::ShouldBlockMovementInput() const
{
	return FAetherCombatActionGatePolicy::ShouldBlockMovementInput(BuildCombatActionStateSnapshot());
}

FAetherCombatActionStateSnapshot UAetherCombatComponent::BuildCombatActionStateSnapshot() const
{
	return FAetherCombatActionStatePolicy::BuildSnapshot(IsOwnerDead(), BuildCombatActionRuntimeFlags());
}

FAetherCombatActionRuntimeFlags UAetherCombatComponent::BuildCombatActionRuntimeFlags() const
{
	FAetherCombatActionRuntimeFlags RuntimeFlags;
	RuntimeFlags.bAttacking = bIsAttacking;
	RuntimeFlags.bHeavyAttacking = bIsHeavyAttacking;
	RuntimeFlags.bExecuting = bIsExecuting;
	RuntimeFlags.bAetherSlashing = bIsAetherSlashing;
	RuntimeFlags.bDodging = bIsDodging;
	RuntimeFlags.bGuarding = bIsGuarding;
	RuntimeFlags.bParryWindowActive = bIsParryWindowActive;
	RuntimeFlags.bParryRecovering = bIsParryRecovering;
	RuntimeFlags.bParryCounterWindowActive = bIsParryCounterWindowActive;
	RuntimeFlags.bHitReacting = bIsHitReacting;
	return RuntimeFlags;
}

void UAetherCombatComponent::ApplyCombatActionRuntimeFlags(const FAetherCombatActionRuntimeFlags& RuntimeFlags)
{
	bIsAttacking = RuntimeFlags.bAttacking;
	bIsHeavyAttacking = RuntimeFlags.bHeavyAttacking;
	bIsExecuting = RuntimeFlags.bExecuting;
	bIsAetherSlashing = RuntimeFlags.bAetherSlashing;
	bIsDodging = RuntimeFlags.bDodging;
	bIsGuarding = RuntimeFlags.bGuarding;
	bIsParryWindowActive = RuntimeFlags.bParryWindowActive;
	bIsParryRecovering = RuntimeFlags.bParryRecovering;
	bIsParryCounterWindowActive = RuntimeFlags.bParryCounterWindowActive;
	bIsHitReacting = RuntimeFlags.bHitReacting;
}

void UAetherCombatComponent::SetCombatActionMode(EAetherCombatActionMode Mode)
{
	ApplyCombatActionRuntimeFlags(FAetherCombatActionStatePolicy::BuildFlagsForMode(Mode));
}

EAetherCombatActionMode UAetherCombatComponent::GetCombatActionMode() const
{
	return FAetherCombatActionStatePolicy::ResolveDominantMode(BuildCombatActionStateSnapshot());
}

void UAetherCombatComponent::ApplyCombatActionTimerClearPlan(const FAetherCombatActionTimerClearPlan& TimerClearPlan)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	if (TimerClearPlan.bClearAttackEnd)
	{
		TimerManager.ClearTimer(AttackEndTimerHandle);
	}
	if (TimerClearPlan.bClearComboReset)
	{
		TimerManager.ClearTimer(ComboResetTimerHandle);
	}
	if (TimerClearPlan.bClearDodgeEnd)
	{
		TimerManager.ClearTimer(DodgeEndTimerHandle);
	}
	if (TimerClearPlan.bClearParryWindow)
	{
		TimerManager.ClearTimer(ParryWindowTimerHandle);
	}
	if (TimerClearPlan.bClearParryRecovery)
	{
		TimerManager.ClearTimer(ParryRecoveryTimerHandle);
	}
	if (TimerClearPlan.bClearParryCounterWindow)
	{
		TimerManager.ClearTimer(ParryCounterWindowTimerHandle);
	}
	if (TimerClearPlan.bClearDamageInvulnerability)
	{
		TimerManager.ClearTimer(DamageInvulnerabilityTimerHandle);
	}
	if (TimerClearPlan.bClearHitReaction)
	{
		TimerManager.ClearTimer(HitReactionTimerHandle);
	}
	if (TimerClearPlan.bClearHeavyImpact)
	{
		TimerManager.ClearTimer(HeavyImpactTimerHandle);
	}
	if (TimerClearPlan.bClearAetherSlashImpact)
	{
		TimerManager.ClearTimer(AetherSlashImpactTimerHandle);
	}
	if (TimerClearPlan.bClearExecutionImpact)
	{
		TimerManager.ClearTimer(ExecutionImpactTimerHandle);
	}
	if (TimerClearPlan.bClearExecution)
	{
		TimerManager.ClearTimer(ExecutionTimerHandle);
	}
}

void UAetherCombatComponent::ApplyCombatResourceMutationPlan(const FAetherCombatResourceMutationPlan& ResourceMutationPlan)
{
	if (!ResourceMutationPlan.bCanApply)
	{
		return;
	}

	if (ResourceMutationPlan.bUpdateStamina)
	{
		CurrentStamina = ResourceMutationPlan.NewStamina;
	}

	if (ResourceMutationPlan.bUpdateAetherGauge)
	{
		CurrentAetherGauge = ResourceMutationPlan.NewAetherGauge;
	}

	if (ResourceMutationPlan.bRecordStaminaSpendTime)
	{
		LastStaminaSpendTime = ResourceMutationPlan.NewLastStaminaSpendTime;
	}
}

void UAetherCombatComponent::StartLightAttack()
{
	const FAetherCombatActionGateResult GateResult = FAetherCombatActionGatePolicy::EvaluateLightAttack(BuildCombatActionStateSnapshot());
	if (!GateResult.bCanStartAction)
	{
		if (GateResult.bShouldQueueLightAttack)
		{
			bQueuedLightAttack = true;
		}
		ShowCombatDebugMessage(GateResult.FailureMessage, GateResult.MessageColor);
		return;
	}

	BeginLightAttack();
}

void UAetherCombatComponent::StartHeavyAttack()
{
	const FAetherCombatActionGateResult GateResult = FAetherCombatActionGatePolicy::EvaluateHeavyAttack(BuildCombatActionStateSnapshot());
	if (!GateResult.bCanStartAction)
	{
		ShowCombatDebugMessage(GateResult.FailureMessage, GateResult.MessageColor);
		return;
	}

	BeginHeavyAttack();
}

void UAetherCombatComponent::StartExecution()
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	UWorld* World = GetWorld();
	if (!Character || !World)
	{
		return;
	}

	if (IsOwnerDead())
	{
		ShowCombatDebugMessage(TEXT("Player is defeated. Press Y to reset."), FColor::Red);
		return;
	}

	if (bIsExecuting)
	{
		if (!bExecutionActiveInputReported)
		{
			bExecutionActiveInputReported = true;
			ShowCombatDebugMessage(TEXT("Execution already active"), FColor::Yellow);
		}
		return;
	}

	if (bIsHitReacting)
	{
		ShowCombatDebugMessage(TEXT("Cannot execute while hit reacting"), FColor::Yellow);
		return;
	}

	if (bIsDodging)
	{
		ShowCombatDebugMessage(TEXT("Cannot execute while dodging"), FColor::Yellow);
		return;
	}

	if (bIsGuarding)
	{
		ShowCombatDebugMessage(TEXT("Cannot execute while guarding"), FColor::Yellow);
		return;
	}

	if ((bIsParryWindowActive || bIsParryRecovering) && !bIsParryCounterWindowActive)
	{
		ShowCombatDebugMessage(TEXT("Cannot execute while parrying"), FColor::Yellow);
		return;
	}

	if (bIsAttacking)
	{
		if (FindExecutionTarget())
		{
			if (!bQueuedExecution)
			{
				bQueuedExecution = true;
				ShowCombatDebugMessage(TEXT("Execution buffered"), FColor::Yellow);
			}
			return;
		}

		ShowCombatDebugMessage(TEXT("Cannot execute while attacking"), FColor::Yellow);
		return;
	}

	AAetherEnemyBase* ExecutionTarget = FindExecutionTarget();
	if (!ExecutionTarget)
	{
		ShowCombatDebugMessage(TEXT("No execution target"), FColor::Yellow);
		return;
	}

	UAetherHealthComponent* TargetHealth = ExecutionTarget->GetHealthComponent();
	if (!TargetHealth || TargetHealth->IsDead())
	{
		ShowCombatDebugMessage(TEXT("No execution target"), FColor::Yellow);
		return;
	}

	ResetCombo();
	SetCombatActionMode(EAetherCombatActionMode::Execution);
	bExecutionActiveInputReported = false;
	bQueuedExecution = false;
	RestoreGuardMovementModifier();
	ApplyCombatResourceMutationPlan(FAetherCombatResourcePolicy::BuildStaminaTimestampPlan(World->GetTimeSeconds()));

	ApplyCombatActionTimerClearPlan(FAetherCombatActionTimerPolicy::BuildClearPlan(EAetherCombatActionTimerClearReason::StartExecution));

	const FAetherExecutionVariant* ExecutionVariant = SelectExecutionVariant();
	UAnimMontage* SelectedExecutionMontage = ExecutionVariant && ExecutionVariant->PlayerExecutionMontage ? ExecutionVariant->PlayerExecutionMontage.Get() : ExecutionMontage.Get();
	UAnimMontage* SelectedEnemyDeathMontage = ExecutionVariant ? ExecutionVariant->EnemyDeathMontage.Get() : nullptr;
	const float SelectedExecutionMontageLength = SelectedExecutionMontage ? SelectedExecutionMontage->GetPlayLength() : 0.0f;
	const FAetherCombatActionExecutionPlan ExecutionPlan = FAetherCombatActionExecutionPolicy::BuildExecutionPlan(
		ExecutionDuration,
		SelectedExecutionMontageLength,
		bUseExecutionImpactNotify,
		ExecutionImpactFallbackNormalizedTime);
	ExecutionTarget->ApplyExecutionSuppression(Character, ExecutionPlan.Duration);
	ShowCombatDebugMessage(TEXT("Execution target locked"), FColor::Yellow);
	const int32 SuppressedEnemyCount = SuppressNearbyEnemiesForExecution(ExecutionTarget, ExecutionPlan.Duration);
	ShowCombatDebugMessage(FString::Printf(TEXT("Execution protected / enemies paused: %d"), SuppressedEnemyCount), FColor::Yellow);
	if (ExecutionVariant)
	{
		const FString VariantLabel = ExecutionVariant->VariantName.IsNone() ? TEXT("Unnamed") : ExecutionVariant->VariantName.ToString();
		ShowCombatDebugMessage(FString::Printf(TEXT("Execution variant: %s"), *VariantLabel), FColor::Yellow);
	}

	Character->ClearDesiredMovementDirection();
	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	const FVector ToTarget = ExecutionTarget->GetActorLocation() - Character->GetActorLocation();
	const FVector FaceDirection = ToTarget.GetSafeNormal2D();
	if (!FaceDirection.IsNearlyZero())
	{
		Character->SetActorRotation(FaceDirection.Rotation());
	}

	PlayActionMontage(SelectedExecutionMontage, TEXT("Execution"));
	PlayActionSound(ExecutionStartSound.Get());

	const FVector Start = Character->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
	const FVector End = ExecutionTarget->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
	DrawDebugLine(World, Start, End, FColor::Yellow, false, ExecutionPlan.Duration, 0, 8.0f);
	DrawDebugSphere(World, End, 120.0f, 24, FColor::Yellow, false, ExecutionPlan.Duration, 0, 5.0f);

	PendingExecutionTarget = ExecutionTarget;
	bExecutionImpactResolved = false;
	ExecutionTarget->SetPendingExecutionDeathMontage(SelectedEnemyDeathMontage);

	if (ExecutionPlan.bWaitForImpactNotify)
	{
		ShowCombatDebugMessage(TEXT("Execution impact waiting for notify"), FColor::Silver);
		if (ExecutionPlan.bScheduleImpactTimer)
		{
			World->GetTimerManager().SetTimer(ExecutionImpactTimerHandle, this, &UAetherCombatComponent::HandleExecutionImpactFallback, ExecutionPlan.ImpactDelay, false);
		}
	}

	if (ExecutionPlan.bApplyImpactImmediately)
	{
		if (ExecutionPlan.bWaitForImpactNotify)
		{
			ShowCombatDebugMessage(TEXT("Execution impact fallback"), FColor::Yellow);
		}
		ResolveExecutionImpact();
	}

	World->GetTimerManager().SetTimer(ExecutionTimerHandle, this, &UAetherCombatComponent::EndExecution, ExecutionPlan.Duration, false);
}

void UAetherCombatComponent::StartAetherSlash()
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	UWorld* World = GetWorld();
	if (!Character || !World)
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	const double RemainingCooldown = GetAetherSlashCooldownRemaining();
	const FAetherCombatActionGateResult GateResult = FAetherCombatActionGatePolicy::EvaluateAetherSlash(BuildCombatActionStateSnapshot(), RemainingCooldown);
	if (!GateResult.bCanStartAction)
	{
		ShowCombatDebugMessage(GateResult.FailureMessage, GateResult.MessageColor);
		return;
	}

	const float AetherSlashCostValue = ResolveAetherSlashCost();
	if (!SpendAetherGauge(AetherSlashCostValue, TEXT("Aether Slash")))
	{
		return;
	}

	ResetCombo();
	SetCombatActionMode(EAetherCombatActionMode::AetherSlash);
	LastAetherSlashTime = CurrentTime;

	ApplyCombatActionTimerClearPlan(FAetherCombatActionTimerPolicy::BuildClearPlan(EAetherCombatActionTimerClearReason::StartAetherSlash));

	StopOwnerMovementForCombatAction();

	if (const UAetherLockOnComponent* LockOnComponent = Character->GetLockOnComponent())
	{
		if (const AActor* LockedTarget = LockOnComponent->GetLockedTarget())
		{
			const FVector ToTarget = LockedTarget->GetActorLocation() - Character->GetActorLocation();
			const FVector FaceDirection = ToTarget.GetSafeNormal2D();
			if (!FaceDirection.IsNearlyZero())
			{
				Character->SetActorRotation(FaceDirection.Rotation());
			}
		}
	}

	ShowCombatDebugMessage(FString::Printf(TEXT("Aether Slash / Aether %.0f"), CurrentAetherGauge), FColor(0, 180, 255));
	const FAetherCombatActionExecutionPlan AetherSlashPlan = FAetherCombatActionExecutionPolicy::BuildAetherSlashPlan(
		ResolveAetherSlashDuration(),
		ResolveAetherSlashImpactDelay(),
		bUseAnimationNotifiesForAttackTraces);
	PlayActionMontage(ResolveAetherSlashMontage(), TEXT("Aether Slash"));
	PlayActionSound(AetherSlashCastSound.Get());
	if (AetherSlashPlan.bWaitForImpactNotify)
	{
		ShowCombatDebugMessage(TEXT("Aether Slash waiting for notify"), FColor::Silver);
	}
	else if (AetherSlashPlan.bScheduleImpactTimer)
	{
		World->GetTimerManager().SetTimer(AetherSlashImpactTimerHandle, this, &UAetherCombatComponent::PerformAetherSlashTrace, AetherSlashPlan.ImpactDelay, false);
	}
	World->GetTimerManager().SetTimer(AttackEndTimerHandle, this, &UAetherCombatComponent::EndCurrentAttack, AetherSlashPlan.Duration, false);
}

void UAetherCombatComponent::StartDodge()
{
	UWorld* World = GetWorld();
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!World || !Character)
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	const FAetherCombatActionGateResult GateResult = FAetherCombatActionGatePolicy::EvaluateDodge(
		BuildCombatActionStateSnapshot(),
		CurrentTime,
		LastDodgeTime,
		DodgeCooldown,
		CurrentStamina,
		DodgeStaminaCost);
	if (!GateResult.bCanStartAction)
	{
		ShowCombatDebugMessage(GateResult.FailureMessage, GateResult.MessageColor);
		return;
	}

	ApplyCombatResourceMutationPlan(FAetherCombatResourcePolicy::BuildStaminaSpendPlan(CurrentStamina, DodgeStaminaCost, CurrentTime));
	LastDodgeTime = CurrentTime;
	SetCombatActionMode(EAetherCombatActionMode::Dodge);
	bQueuedLightAttack = false;
	ResetCombo();

	const FVector DodgeDirection = Character->GetDesiredMovementDirection();
	Character->ClearDesiredMovementDirection();
	if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}
	ClearDodgeMovementState();
	ActiveDodgeDirection = DodgeDirection.GetSafeNormal2D();

	if (!bUseRootMotionForDodge)
	{
		if (bUseTimedDodgeMovement)
		{
			DodgeMovementElapsed = 0.0f;
			DodgeMovementAlpha = 0.0f;
		}
		else
		{
			Character->LaunchCharacter(ActiveDodgeDirection * DodgeStrength, true, false);
		}
	}

	const FVector Start = Character->GetActorLocation();
	const FVector End = Start + ActiveDodgeDirection * 180.0f;
	DrawDebugDirectionalArrow(World, Start, End, 65.0f, FColor::Blue, false, DodgeDuration, 0, 4.0f);

	ShowCombatDebugMessage(FString::Printf(TEXT("Dodge / Stamina %.0f"), CurrentStamina), FColor::Blue);
	PlayActionMontage(GetDodgeMontageForDirection(ActiveDodgeDirection), TEXT("Dodge"));
	PlayActionSound(DodgeSound.Get());
	World->GetTimerManager().SetTimer(DodgeEndTimerHandle, this, &UAetherCombatComponent::EndDodge, DodgeDuration, false);
}

void UAetherCombatComponent::StartGuard()
{
	const FAetherCombatActionGateResult GateResult = FAetherCombatActionGatePolicy::EvaluateGuard(BuildCombatActionStateSnapshot(), CurrentStamina);
	if (!GateResult.bCanStartAction)
	{
		ShowCombatDebugMessage(GateResult.FailureMessage, GateResult.MessageColor);
		return;
	}

	SetCombatActionMode(EAetherCombatActionMode::Guard);
	ResetCombo();
	ApplyGuardMovementModifier();
	ShowCombatDebugMessage(TEXT("Guard raised"), FColor::Silver);
	PlayActionMontage(GuardStartMontage, TEXT("Guard raised"));
	PlayActionSound(GuardRaiseSound.Get());
}

void UAetherCombatComponent::StopGuard()
{
	if (!bIsGuarding)
	{
		return;
	}

	SetCombatActionMode(EAetherCombatActionMode::Idle);
	RestoreGuardMovementModifier();
	ShowCombatDebugMessage(TEXT("Guard lowered"), FColor::Silver);
	PlayActionMontage(GuardEndMontage, TEXT("Guard lowered"));
	PlayActionSound(GuardLowerSound.Get());
}

void UAetherCombatComponent::TryParry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FAetherCombatActionGateResult GateResult = FAetherCombatActionGatePolicy::EvaluateParry(
		BuildCombatActionStateSnapshot(),
		CurrentStamina,
		ParryStaminaCost);
	if (!GateResult.bCanStartAction)
	{
		ShowCombatDebugMessage(GateResult.FailureMessage, GateResult.MessageColor);
		return;
	}

	SetCombatActionMode(EAetherCombatActionMode::ParryWindow);
	RestoreGuardMovementModifier();
	ApplyCombatResourceMutationPlan(FAetherCombatResourcePolicy::BuildStaminaSpendPlan(CurrentStamina, ParryStaminaCost, World->GetTimeSeconds()));
	ResetCombo();
	StopOwnerMovementForCombatAction();

	if (AAetherfallCharacter* Character = OwnerCharacter.Get())
	{
		const FVector Location = Character->GetActorLocation() + FVector(0.0f, 0.0f, 70.0f);
		DrawDebugSphere(World, Location, 95.0f, 24, FColor::Purple, false, ParryWindowDuration, 0, 3.0f);
	}

	ShowCombatDebugMessage(FString::Printf(TEXT("Parry window / Stamina %.0f"), CurrentStamina), FColor::Purple);
	PlayActionMontage(ParryAttemptMontage, TEXT("Parry attempt"));
	PlayActionSound(ParryAttemptSound.Get());
	World->GetTimerManager().SetTimer(ParryWindowTimerHandle, this, &UAetherCombatComponent::EndParryWindow, ParryWindowDuration, false);
}

void UAetherCombatComponent::SimulateIncomingHit()
{
	ReceiveIncomingHit(PrototypeIncomingDamage, GetOwner());
}

void UAetherCombatComponent::ReceiveIncomingHit(float DamageAmount, AActor* DamageCauser)
{
	if (IsOwnerDead())
	{
		ShowCombatDebugMessage(TEXT("Player is defeated. Press Y to reset."), FColor::Red);
		return;
	}

	if (bIsDamageInvulnerable)
	{
		ShowCombatDebugMessage(TEXT("Incoming hit ignored / recovery"), FColor::Yellow);
		return;
	}

	if (bIsExecuting)
	{
		ShowCombatDebugMessage(TEXT("Incoming hit ignored / execution"), FColor::Yellow);
		return;
	}

	ShowCombatDebugMessage(TEXT("Incoming hit"), FColor::Orange);

	if (bIsParryWindowActive)
	{
		SetCombatActionMode(EAetherCombatActionMode::Idle);
		ApplyCombatActionTimerClearPlan(FAetherCombatActionTimerPolicy::BuildClearPlan(EAetherCombatActionTimerClearReason::ParrySuccess));

		ShowCombatDebugMessage(TEXT("Parry success"), FColor::Purple);
		PlayActionMontage(ParryMontage, TEXT("Parry counter"));
		AddAetherGauge(ParrySuccessAetherGain, TEXT("Parry success"));
		PlayImpactFeedback(EAetherCombatFeedbackType::ParrySuccess, ParryCameraKickStrength, ParryHitStopDuration, TEXT("Parry success"), DamageCauser);
		if (AAetherEnemyBase* EnemyCauser = Cast<AAetherEnemyBase>(DamageCauser))
		{
			EnemyCauser->ApplyParryStagger(OwnerCharacter.Get());
		}
		if (bAutoCounterOnParrySuccess)
		{
			BeginAutomaticParryCounter(DamageCauser);
		}
		else
		{
			OpenParryCounterWindow();
		}
		return;
	}

	if (bIsGuarding)
	{
		const float GuardStaminaCost = FAetherCombatActionTuningPolicy::CalculateGuardStaminaCost(DamageAmount, BuildGuardStaminaTuning());
		if (CurrentStamina < GuardStaminaCost)
		{
			SetCombatActionMode(EAetherCombatActionMode::Idle);
			RestoreGuardMovementModifier();
			ShowCombatDebugMessage(TEXT("Guard broken"), FColor::Red);
			PlayActionMontage(GuardBreakMontage, TEXT("Guard broken"));
			PlayActionSound(GuardBreakSound.Get(), 1.1f);
			ApplyIncomingDamage(DamageAmount, DamageCauser);
			return;
		}

		ApplyCombatResourceMutationPlan(
			FAetherCombatResourcePolicy::BuildStaminaSpendPlan(
				CurrentStamina,
				GuardStaminaCost,
				GetWorld() ? GetWorld()->GetTimeSeconds() : LastStaminaSpendTime));

		const float ReducedDamage = DamageAmount * (1.0f - GuardDamageReduction);
		const bool bDamageApplied = ApplyIncomingDamage(ReducedDamage, DamageCauser, false);
		if (IsOwnerDead())
		{
			return;
		}

		if (bDamageApplied)
		{
			ShowCombatDebugMessage(FString::Printf(TEXT("Guard blocked %.0f damage / Guard cost %.0f / Stamina %.0f"), DamageAmount - ReducedDamage, GuardStaminaCost, CurrentStamina), FColor::Silver);
			PlayActionMontage(GuardBlockHitMontage, TEXT("Guard block hit"));
		}
		return;
	}

	ApplyIncomingDamage(DamageAmount, DamageCauser);
}

void UAetherCombatComponent::ResetPlayerPrototypeState()
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character || !Character->GetHealthComponent())
	{
		return;
	}

	ClearCombatRuntimeState();
	ApplyCombatResourceMutationPlan(FAetherCombatResourcePolicy::BuildResetPlan(MaxStamina, GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0));
	LastDodgeTime = -100.0;
	LastAetherSlashTime = -100.0;
	ResetPlayerDangerAudioCues();
	ResetAetherResourceAudioCues();
	Character->GetHealthComponent()->ResetHealth();
	SetOwnerMovementEnabled(true);

	ShowCombatDebugMessage(FString::Printf(TEXT("Player reset / HP %.0f / Stamina %.0f / Aether %.0f"), Character->GetHealthComponent()->GetCurrentHealth(), CurrentStamina, CurrentAetherGauge), FColor::Green);
}

void UAetherCombatComponent::GrantPrototypeAetherGauge(float Amount, const FString& Reason)
{
	AddAetherGauge(Amount, Reason);
}

void UAetherCombatComponent::HandleLightAttackHitNotify()
{
	if (!bUseAnimationNotifiesForAttackTraces)
	{
		return;
	}

	if (!bIsAttacking || bIsHeavyAttacking || bIsAetherSlashing || CurrentComboStep <= 0)
	{
		ShowCombatDebugMessage(TEXT("Light notify ignored"), FColor::Yellow);
		return;
	}

	PerformPrototypeTrace(CurrentComboStep);
}

void UAetherCombatComponent::HandleHeavyAttackHitNotify()
{
	if (!bUseAnimationNotifiesForAttackTraces)
	{
		return;
	}

	if (!bIsAttacking || !bIsHeavyAttacking)
	{
		ShowCombatDebugMessage(TEXT("Heavy notify ignored"), FColor::Yellow);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HeavyImpactTimerHandle);
	}
	PerformHeavyAttackTrace();
}

void UAetherCombatComponent::HandleAetherSlashFireNotify()
{
	if (!bUseAnimationNotifiesForAttackTraces)
	{
		return;
	}

	if (!bIsAttacking || !bIsAetherSlashing)
	{
		ShowCombatDebugMessage(TEXT("Aether Slash notify ignored"), FColor::Yellow);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AetherSlashImpactTimerHandle);
	}
	PerformAetherSlashTrace();
}

void UAetherCombatComponent::HandleExecutionImpactNotify()
{
	if (!bIsExecuting || bExecutionImpactResolved)
	{
		ShowCombatDebugMessage(TEXT("Execution impact notify ignored"), FColor::Yellow);
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExecutionImpactTimerHandle);
	}

	ShowCombatDebugMessage(TEXT("Execution impact notify"), FColor::Yellow);
	ResolveExecutionImpact();
}

void UAetherCombatComponent::BeginLightAttack()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CurrentComboStep >= MaxComboSteps)
	{
		CurrentComboStep = 0;
	}

	const int32 NextComboStep = CurrentComboStep + 1;
	const float StaminaCost = FAetherCombatActionTuningPolicy::SelectLightAttackCost(NextComboStep, ResolveLightAttackStaminaCosts());
	if (CurrentStamina < StaminaCost)
	{
		bQueuedLightAttack = false;
		ShowCombatDebugMessage(FString::Printf(TEXT("Not enough stamina: %.1f / %.1f"), CurrentStamina, StaminaCost), FColor::Red);
		return;
	}

	ApplyCombatActionTimerClearPlan(FAetherCombatActionTimerPolicy::BuildClearPlan(EAetherCombatActionTimerClearReason::StartLightAttack));

	ApplyCombatResourceMutationPlan(FAetherCombatResourcePolicy::BuildStaminaSpendPlan(CurrentStamina, StaminaCost, World->GetTimeSeconds()));
	CurrentComboStep = NextComboStep;
	SetCombatActionMode(EAetherCombatActionMode::LightAttack);
	bQueuedLightAttack = false;
	StopOwnerMovementForCombatAction();

	const FAetherCombatActionExecutionPlan LightAttackPlan = FAetherCombatActionExecutionPolicy::BuildLightAttackPlan(
		ResolveLightAttackDuration(),
		bUseAnimationNotifiesForAttackTraces);
	PlayActionMontage(GetLightAttackMontage(CurrentComboStep), FString::Printf(TEXT("Light %d"), CurrentComboStep));
	PlayActionSound(LightAttackSwingSound.Get());
	if (LightAttackPlan.bWaitForImpactNotify)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Light %d waiting for notify"), CurrentComboStep), FColor::Silver);
	}
	else if (LightAttackPlan.bApplyImpactImmediately)
	{
		PerformPrototypeTrace(CurrentComboStep);
	}
	ShowCombatDebugMessage(FString::Printf(TEXT("Light %d / Stamina %.0f"), CurrentComboStep, CurrentStamina), FColor::Cyan);

	World->GetTimerManager().SetTimer(AttackEndTimerHandle, this, &UAetherCombatComponent::EndCurrentAttack, LightAttackPlan.Duration, false);
}

void UAetherCombatComponent::BeginHeavyAttack()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float HeavyStaminaCost = ResolveHeavyAttackStaminaCost();
	if (CurrentStamina < HeavyStaminaCost)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Not enough stamina for heavy: %.1f / %.1f"), CurrentStamina, HeavyStaminaCost), FColor::Red);
		return;
	}

	const bool bCounterHeavy = bIsParryCounterWindowActive;
	ApplyCombatActionTimerClearPlan(FAetherCombatActionTimerPolicy::BuildClearPlan(EAetherCombatActionTimerClearReason::StartHeavyAttack));
	ResetCombo();
	ApplyCombatResourceMutationPlan(FAetherCombatResourcePolicy::BuildStaminaSpendPlan(CurrentStamina, HeavyStaminaCost, World->GetTimeSeconds()));
	SetCombatActionMode(EAetherCombatActionMode::HeavyAttack);
	StopOwnerMovementForCombatAction();

	if (bCounterHeavy)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Heavy counter attack / Stamina %.0f"), CurrentStamina), FColor::Cyan);
	}
	else
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Heavy attack / Stamina %.0f"), CurrentStamina), FColor::Cyan);
	}

	const FAetherCombatActionExecutionPlan HeavyAttackPlan = FAetherCombatActionExecutionPolicy::BuildHeavyAttackPlan(
		ResolveHeavyAttackDuration(),
		ResolveHeavyAttackImpactDelay(),
		bUseAnimationNotifiesForAttackTraces);
	PlayActionMontage(ResolveHeavyAttackMontage(bCounterHeavy), bCounterHeavy ? TEXT("Heavy counter attack") : TEXT("Heavy attack"));
	PlayActionSound((bCounterHeavy && HeavyCounterSwingSound) ? HeavyCounterSwingSound.Get() : HeavyAttackSwingSound.Get());
	if (HeavyAttackPlan.bWaitForImpactNotify)
	{
		ShowCombatDebugMessage(TEXT("Heavy waiting for notify"), FColor::Silver);
	}
	else if (HeavyAttackPlan.bScheduleImpactTimer)
	{
		World->GetTimerManager().SetTimer(HeavyImpactTimerHandle, this, &UAetherCombatComponent::PerformHeavyAttackTrace, HeavyAttackPlan.ImpactDelay, false);
	}
	World->GetTimerManager().SetTimer(AttackEndTimerHandle, this, &UAetherCombatComponent::EndCurrentAttack, HeavyAttackPlan.Duration, false);
}

void UAetherCombatComponent::EndExecution()
{
	if (!bExecutionImpactResolved && PendingExecutionTarget.IsValid())
	{
		ShowCombatDebugMessage(TEXT("Execution impact fallback at end"), FColor::Yellow);
		ResolveExecutionImpact();
	}

	SetCombatActionMode(EAetherCombatActionMode::Idle);
	bExecutionActiveInputReported = false;
	ClearPendingExecutionImpact();
	ShowCombatDebugMessage(TEXT("Execution ended"), FColor::Silver);
}

void UAetherCombatComponent::EndCurrentAttack()
{
	const bool bWasHeavyAttacking = bIsHeavyAttacking;
	const bool bWasAetherSlashing = bIsAetherSlashing;
	SetCombatActionMode(EAetherCombatActionMode::Idle);
	PreferredHitTarget.Reset();
	if (bWasHeavyAttacking)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(HeavyImpactTimerHandle);
		}
	}
	if (bWasAetherSlashing)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(AetherSlashImpactTimerHandle);
		}
	}

	if (bQueuedExecution)
	{
		bQueuedExecution = false;
		StartExecution();
		return;
	}

	if (!bWasHeavyAttacking && !bWasAetherSlashing && bQueuedLightAttack && CurrentComboStep < MaxComboSteps)
	{
		BeginLightAttack();
		return;
	}

	bQueuedLightAttack = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ComboResetTimerHandle, this, &UAetherCombatComponent::ResetCombo, ComboResetDelay, false);
	}
}

void UAetherCombatComponent::EndDodge()
{
	SetCombatActionMode(EAetherCombatActionMode::Idle);
	if (bStopMovementWhenDodgeEnds)
	{
		StopOwnerMovementForCombatAction(false);
	}
	ClearDodgeMovementState();
}

void UAetherCombatComponent::EndParryWindow()
{
	SetCombatActionMode(EAetherCombatActionMode::ParryRecovery);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ParryRecoveryTimerHandle, this, &UAetherCombatComponent::EndParryRecovery, ParryRecoveryDuration, false);
	}

	ShowCombatDebugMessage(TEXT("Parry missed"), FColor::Yellow);
}

void UAetherCombatComponent::EndParryRecovery()
{
	SetCombatActionMode(EAetherCombatActionMode::Idle);
}

void UAetherCombatComponent::EndParryCounterWindow()
{
	SetCombatActionMode(EAetherCombatActionMode::Idle);
	ShowCombatDebugMessage(TEXT("Parry counter window ended"), FColor::Purple);
}

void UAetherCombatComponent::OpenParryCounterWindow()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SetCombatActionMode(EAetherCombatActionMode::ParryCounterWindow);
	ApplyCombatActionTimerClearPlan(FAetherCombatActionTimerPolicy::BuildClearPlan(EAetherCombatActionTimerClearReason::OpenParryCounterWindow));
	World->GetTimerManager().SetTimer(ParryCounterWindowTimerHandle, this, &UAetherCombatComponent::EndParryCounterWindow, ParryCounterWindowDuration, false);
	ShowCombatDebugMessage(TEXT("Parry counter window"), FColor::Purple);
}

void UAetherCombatComponent::BeginAutomaticParryCounter(AActor* CounterTarget)
{
	UWorld* World = GetWorld();
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!World || !Character || bIsAttacking || bIsExecuting || bIsHitReacting)
	{
		return;
	}

	PreferredHitTarget = CounterTarget;
	SetCombatActionMode(EAetherCombatActionMode::HeavyAttack);
	bQueuedLightAttack = false;
	bQueuedExecution = false;

	ApplyCombatActionTimerClearPlan(FAetherCombatActionTimerPolicy::BuildClearPlan(EAetherCombatActionTimerClearReason::StartAutoParryCounter));

	StopOwnerMovementForCombatAction();

	if (CounterTarget)
	{
		const FVector FaceDirection = (CounterTarget->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal2D();
		if (!FaceDirection.IsNearlyZero())
		{
			Character->SetActorRotation(FaceDirection.Rotation());
		}
	}

	ShowCombatDebugMessage(TEXT("Parry auto counter"), FColor::Purple);
	const FAetherCombatActionExecutionPlan AutoCounterPlan = FAetherCombatActionExecutionPolicy::BuildAutoParryCounterPlan(
		AutoParryCounterDuration,
		AutoParryCounterImpactDelay,
		bUseAnimationNotifiesForAttackTraces);
	PlayActionSound(HeavyCounterSwingSound ? HeavyCounterSwingSound.Get() : HeavyAttackSwingSound.Get());
	if (AutoCounterPlan.bWaitForImpactNotify)
	{
		ShowCombatDebugMessage(TEXT("Parry auto counter waiting for notify"), FColor::Silver);
	}
	else if (AutoCounterPlan.bScheduleImpactTimer)
	{
		World->GetTimerManager().SetTimer(HeavyImpactTimerHandle, this, &UAetherCombatComponent::PerformHeavyAttackTrace, AutoCounterPlan.ImpactDelay, false);
	}
	World->GetTimerManager().SetTimer(AttackEndTimerHandle, this, &UAetherCombatComponent::EndCurrentAttack, AutoCounterPlan.Duration, false);
}

void UAetherCombatComponent::BeginDamageInvulnerability()
{
	UWorld* World = GetWorld();
	if (!World || DamageInvulnerabilityDuration <= 0.0f)
	{
		return;
	}

	bIsDamageInvulnerable = true;
	ApplyCombatActionTimerClearPlan(FAetherCombatActionTimerPolicy::BuildClearPlan(EAetherCombatActionTimerClearReason::RefreshDamageInvulnerability));
	World->GetTimerManager().SetTimer(DamageInvulnerabilityTimerHandle, this, &UAetherCombatComponent::EndDamageInvulnerability, DamageInvulnerabilityDuration, false);
	ShowCombatDebugMessage(FString::Printf(TEXT("Hit recovery / I-frames %.2f"), DamageInvulnerabilityDuration), FColor::Orange);
}

void UAetherCombatComponent::EndDamageInvulnerability()
{
	bIsDamageInvulnerable = false;
	ShowCombatDebugMessage(TEXT("Hit recovery ended"), FColor::Silver);

	if (bDelayEnemyAttackAfterDamageInvulnerability)
	{
		bDelayEnemyAttackAfterDamageInvulnerability = false;
		if (EnemyAttackDelayAfterHitRecovery > 0.0f)
		{
			if (AAetherGameModeBase* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AAetherGameModeBase>() : nullptr)
			{
				GameMode->DelayPrototypeEnemyAttacks(EnemyAttackDelayAfterHitRecovery);
			}
		}
	}
}

void UAetherCombatComponent::BeginHitReaction(AActor* DamageCauser)
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	UWorld* World = GetWorld();
	if (!Character || !World || HitReactionDuration <= 0.0f)
	{
		return;
	}

	SetCombatActionMode(EAetherCombatActionMode::HitReaction);
	bExecutionActiveInputReported = false;
	bQueuedLightAttack = false;
	bQueuedExecution = false;
	ClearDodgeMovementState();
	RestoreGuardMovementModifier();
	ResetCombo();

	ApplyCombatActionTimerClearPlan(FAetherCombatActionTimerPolicy::BuildClearPlan(EAetherCombatActionTimerClearReason::HitReaction));
	ClearPendingExecutionImpact();

	const FVector KnockbackDirection = GetHitReactionDirection(DamageCauser);
	Character->ClearDesiredMovementDirection();
	Character->LaunchCharacter((KnockbackDirection * HitKnockbackStrength) + FVector(0.0f, 0.0f, HitKnockbackUpwardStrength), true, true);

	const FVector Start = Character->GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
	const FVector End = Start + KnockbackDirection * 150.0f;
	DrawDebugDirectionalArrow(World, Start, End, 45.0f, FColor::Red, false, HitReactionDuration, 0, 3.0f);

	World->GetTimerManager().SetTimer(HitReactionTimerHandle, this, &UAetherCombatComponent::EndHitReaction, HitReactionDuration, false);
	ShowCombatDebugMessage(TEXT("Hit reaction / knockback"), FColor::Red);
}

void UAetherCombatComponent::EndHitReaction()
{
	SetCombatActionMode(EAetherCombatActionMode::Idle);
	ShowCombatDebugMessage(TEXT("Hit reaction ended"), FColor::Silver);
}

FVector UAetherCombatComponent::GetHitReactionDirection(AActor* DamageCauser) const
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return FVector::ForwardVector;
	}

	if (DamageCauser && DamageCauser != Character)
	{
		const FVector FromCauser = Character->GetActorLocation() - DamageCauser->GetActorLocation();
		const FVector Direction = FromCauser.GetSafeNormal2D();
		if (!Direction.IsNearlyZero())
		{
			return Direction;
		}
	}

	return -Character->GetActorForwardVector().GetSafeNormal2D();
}

void UAetherCombatComponent::UpdateDodgeMovement(float DeltaTime)
{
	if (!bIsDodging || bUseRootMotionForDodge || !bUseTimedDodgeMovement || DodgeDuration <= KINDA_SMALL_NUMBER || DodgeTravelDistance <= 0.0f || ActiveDodgeDirection.IsNearlyZero())
	{
		return;
	}

	AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return;
	}

	DodgeMovementElapsed = FMath::Min(DodgeMovementElapsed + DeltaTime, DodgeDuration);
	const float LinearAlpha = FMath::Clamp(DodgeMovementElapsed / DodgeDuration, 0.0f, 1.0f);
	const float EasedAlpha = 1.0f - FMath::Pow(1.0f - LinearAlpha, DodgeEaseOutExponent);
	const float DeltaAlpha = FMath::Max(0.0f, EasedAlpha - DodgeMovementAlpha);
	DodgeMovementAlpha = EasedAlpha;

	const FVector MoveDelta = ActiveDodgeDirection * DodgeTravelDistance * DeltaAlpha;
	if (MoveDelta.IsNearlyZero())
	{
		return;
	}

	FHitResult SweepHit;
	Character->AddActorWorldOffset(MoveDelta, true, &SweepHit);
	if (SweepHit.IsValidBlockingHit())
	{
		DodgeMovementElapsed = DodgeDuration;
		DodgeMovementAlpha = 1.0f;
	}
}

void UAetherCombatComponent::ClearDodgeMovementState()
{
	ActiveDodgeDirection = FVector::ZeroVector;
	DodgeMovementElapsed = 0.0f;
	DodgeMovementAlpha = 0.0f;
}

void UAetherCombatComponent::ApplyGuardMovementModifier()
{
	if (!bSlowMovementWhileGuarding)
	{
		return;
	}

	AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent || MovementComponent->MovementMode == MOVE_None)
	{
		return;
	}

	if (!bHasGuardMovementSpeedCache)
	{
		CachedPreGuardMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
		bHasGuardMovementSpeedCache = true;
	}

	MovementComponent->MaxWalkSpeed = CachedPreGuardMaxWalkSpeed * GuardMovementSpeedMultiplier;
}

void UAetherCombatComponent::RestoreGuardMovementModifier()
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	UCharacterMovementComponent* MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
	if (MovementComponent && bHasGuardMovementSpeedCache)
	{
		MovementComponent->MaxWalkSpeed = CachedPreGuardMaxWalkSpeed;
	}

	bHasGuardMovementSpeedCache = false;
	CachedPreGuardMaxWalkSpeed = 0.0f;
}

void UAetherCombatComponent::ResetCombo()
{
	CurrentComboStep = 0;
	bQueuedLightAttack = false;
}

void UAetherCombatComponent::RegenerateStamina(float DeltaTime)
{
	const UWorld* World = GetWorld();
	if (!World ||
		!FAetherCombatActionGatePolicy::CanRegenerateStamina(
			BuildCombatActionStateSnapshot(),
			CurrentStamina,
			MaxStamina,
			World->GetTimeSeconds(),
			LastStaminaSpendTime,
			StaminaRegenDelay))
	{
		return;
	}

	ApplyCombatResourceMutationPlan(FAetherCombatResourcePolicy::BuildStaminaRegenPlan(CurrentStamina, MaxStamina, StaminaRegenPerSecond, DeltaTime));
}

void UAetherCombatComponent::AddAetherGauge(float Amount, const FString& Reason)
{
	const FAetherCombatResourceMutationPlan ResourceMutationPlan =
		FAetherCombatResourcePolicy::BuildAetherGainPlan(CurrentAetherGauge, MaxAetherGauge, Amount);
	if (Amount <= 0.0f || MaxAetherGauge <= 0.0f)
	{
		return;
	}

	ApplyCombatResourceMutationPlan(ResourceMutationPlan);
	const float OldAetherGauge = ResourceMutationPlan.PreviousAetherGauge;
	const float ActualGain = ResourceMutationPlan.AetherGaugeDelta;
	if (ActualGain <= KINDA_SMALL_NUMBER)
	{
		ShowCombatDebugMessage(TEXT("Aether full"), FColor(0, 180, 255));
		return;
	}

	ShowCombatDebugMessage(FString::Printf(TEXT("Aether +%.0f (%s) / %.0f / %.0f"), ActualGain, *Reason, CurrentAetherGauge, MaxAetherGauge), FColor(0, 180, 255));
	UpdateAetherResourceAudioCues(OldAetherGauge);
}

bool UAetherCombatComponent::SpendAetherGauge(float Amount, const FString& Reason)
{
	const FAetherCombatResourceMutationPlan ResourceMutationPlan =
		FAetherCombatResourcePolicy::BuildAetherSpendPlan(CurrentAetherGauge, Amount);
	if (Amount <= 0.0f)
	{
		return true;
	}

	if (!ResourceMutationPlan.bCanApply)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Not enough Aether: %.1f / %.1f"), CurrentAetherGauge, Amount), FColor::Red);
		return false;
	}

	ApplyCombatResourceMutationPlan(ResourceMutationPlan);
	FAetherCombatAudioCuePolicy::RefreshResourceStateAfterSpend(CurrentAetherGauge, BuildResourceCueConfig(), ResourceCueState);

	ShowCombatDebugMessage(FString::Printf(TEXT("Aether -%.0f (%s) / %.0f / %.0f"), Amount, *Reason, CurrentAetherGauge, MaxAetherGauge), FColor(0, 180, 255));
	return true;
}

void UAetherCombatComponent::PerformPrototypeTrace(int32 ComboStep)
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	UWorld* World = GetWorld();
	if (!Character || !World || bIsHeavyAttacking)
	{
		return;
	}

	const FAetherCombatTraceRequest TraceRequest = FAetherCombatTracePolicy::BuildMeleeSphereTrace(
		Character,
		PrototypeTraceHeightOffset,
		ResolveLightTraceDistance(),
		ResolveLightTraceRadius(),
		ResolveLightAttackDuration(),
		FColor::Green,
		FColor::Orange,
		3.0f,
		2.0f,
		16);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AetherLightAttackTrace), false, Character);
	PrepareReusableTraceHitResults(8);
	const FCollisionShape Shape = FCollisionShape::MakeSphere(TraceRequest.Radius);
	const bool bHit = World->SweepMultiByChannel(ReusableTraceHitResults, TraceRequest.Start, TraceRequest.End, FQuat::Identity, ECC_Pawn, Shape, QueryParams);

	const FColor TraceColor = TraceRequest.GetDebugColor(bHit);
	DrawDebugLine(World, TraceRequest.Start, TraceRequest.End, TraceColor, false, TraceRequest.DebugDuration, 0, TraceRequest.DebugLineThickness);
	DrawDebugSphere(World, TraceRequest.End, TraceRequest.Radius, TraceRequest.DebugSphereSegments, TraceColor, false, TraceRequest.DebugDuration, 0, TraceRequest.DebugSphereThickness);

	if (bHit)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Light %d hit candidates: %d"), ComboStep, ReusableTraceHitResults.Num()), FColor::Green);
		ApplyPrototypeDamage(ReusableTraceHitResults, ComboStep);
	}
}

void UAetherCombatComponent::PrepareReusableTraceHitResults(int32 ExpectedHitCount)
{
	ReusableTraceHitResults.Reset();
	ReusableTraceHitResults.Reserve(FMath::Max(1, ExpectedHitCount));
}

void UAetherCombatComponent::PerformHeavyAttackTrace()
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	UWorld* World = GetWorld();
	if (!Character || !World || !bIsHeavyAttacking)
	{
		return;
	}

	const FAetherCombatTraceRequest TraceRequest = FAetherCombatTracePolicy::BuildMeleeSphereTrace(
		Character,
		PrototypeTraceHeightOffset,
		ResolveHeavyTraceDistance(),
		ResolveHeavyTraceRadius(),
		ResolveHeavyAttackDuration(),
		FColor(0, 200, 120),
		FColor::Orange,
		5.0f,
		3.0f,
		20);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AetherHeavyAttackTrace), false, Character);
	PrepareReusableTraceHitResults(8);
	const FCollisionShape Shape = FCollisionShape::MakeSphere(TraceRequest.Radius);
	const bool bHit = World->SweepMultiByChannel(ReusableTraceHitResults, TraceRequest.Start, TraceRequest.End, FQuat::Identity, ECC_Pawn, Shape, QueryParams);

	const FColor TraceColor = TraceRequest.GetDebugColor(bHit);
	DrawDebugLine(World, TraceRequest.Start, TraceRequest.End, TraceColor, false, TraceRequest.DebugDuration, 0, TraceRequest.DebugLineThickness);
	DrawDebugSphere(World, TraceRequest.End, TraceRequest.Radius, TraceRequest.DebugSphereSegments, TraceColor, false, TraceRequest.DebugDuration, 0, TraceRequest.DebugSphereThickness);

	if (bHit)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Heavy hit candidates: %d"), ReusableTraceHitResults.Num()), FColor::Green);
		ApplyHeavyAttackDamage(ReusableTraceHitResults);
	}
	else
	{
		ShowCombatDebugMessage(TEXT("Heavy attack missed"), FColor::Yellow);
	}
}

void UAetherCombatComponent::PerformAetherSlashTrace()
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	UWorld* World = GetWorld();
	if (!Character || !World || !bIsAetherSlashing)
	{
		return;
	}

	const FVector SlashDirection = Character->GetActorForwardVector().GetSafeNormal();
	const FVector SpawnLocation = Character->GetActorLocation() + SlashDirection * 90.0f + FVector(0.0f, 0.0f, PrototypeTraceHeightOffset);

	UAetherProjectilePoolSubsystem* ProjectilePool = World->GetSubsystem<UAetherProjectilePoolSubsystem>();
	AAetherSlashProjectile* SlashProjectile = ProjectilePool
		? ProjectilePool->AcquireAetherSlashProjectile(Character, Character, SpawnLocation, SlashDirection.Rotation())
		: nullptr;
	if (!SlashProjectile)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Character;
		SpawnParameters.Instigator = Character;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SlashProjectile = World->SpawnActor<AAetherSlashProjectile>(AAetherSlashProjectile::StaticClass(), SpawnLocation, SlashDirection.Rotation(), SpawnParameters);
	}
	if (!SlashProjectile)
	{
		ShowCombatDebugMessage(TEXT("Aether Slash projectile failed"), FColor::Red);
		return;
	}

	AActor* LockedTarget = nullptr;
	if (const UAetherLockOnComponent* LockOnComponent = Character->GetLockOnComponent())
	{
		LockedTarget = LockOnComponent->GetLockedTarget();
	}

	SlashProjectile->InitializeSlash(Character, LockedTarget, SlashDirection, ResolveAetherSlashDamage(), ResolveAetherSlashProjectileSpeed(), ResolveAetherSlashTraceDistance(), ResolveAetherSlashTraceRadius());
	SlashProjectile->SetImpactAssets(AetherSlashImpactEffect.Get(), AetherSlashHitSound.Get(), ImpactEffectScale, ImpactSoundVolume);
}

void UAetherCombatComponent::ApplyHeavyAttackDamage(const TArray<FHitResult>& HitResults)
{
	AActor* DamageCauser = GetOwner();
	const FAetherCombatTargetSelection TargetSelection =
		FAetherCombatTargetSelectionPolicy::BuildTargetSelection(HitResults, DamageCauser, PreferredHitTarget.Get(), GetLockedCombatTarget());

	if (TargetSelection.HasPriorityTarget())
	{
		const AAetherEnemyBase* PriorityEnemy = Cast<AAetherEnemyBase>(TargetSelection.PriorityTarget);
		const bool bStaggerBonus = PriorityEnemy && PriorityEnemy->IsParryStaggered();
		const float DamageAmount = FAetherCombatActionTuningPolicy::CalculateHeavyAttackDamage(ResolveHeavyAttackDamage(), ResolveHeavyStaggerDamageMultiplier(), bStaggerBonus);
		if (TargetSelection.IsPreferredPriority())
		{
			ShowCombatDebugMessage(TEXT("Heavy preferred counter target"), FColor::Cyan);
		}
		else if (TargetSelection.IsLockedPriority())
		{
			ShowCombatDebugMessage(TEXT("Heavy locked target priority"), FColor::Cyan);
		}
		ApplyHeavyDamageToActor(TargetSelection.PriorityTarget, DamageAmount, DamageCauser, true, bStaggerBonus);
		return;
	}

	for (AActor* HitActor : TargetSelection.AdditionalTargets)
	{
		const AAetherEnemyBase* Enemy = Cast<AAetherEnemyBase>(HitActor);
		const bool bStaggerBonus = Enemy && Enemy->IsParryStaggered();
		const float DamageAmount = FAetherCombatActionTuningPolicy::CalculateHeavyAttackDamage(ResolveHeavyAttackDamage(), ResolveHeavyStaggerDamageMultiplier(), bStaggerBonus);
		ApplyHeavyDamageToActor(HitActor, DamageAmount, DamageCauser, false, bStaggerBonus);
	}
}

bool UAetherCombatComponent::ApplyHeavyDamageToActor(AActor* TargetActor, float DamageAmount, AActor* DamageCauser, bool bLockedTargetDamage, bool bStaggerBonus)
{
	FAetherCombatDamageRequest DamageRequest;
	DamageRequest.TargetActor = TargetActor;
	DamageRequest.DamageCauser = DamageCauser;
	DamageRequest.DamageAmount = DamageAmount;
	const FAetherCombatDamageResult DamageResult = FAetherCombatDamagePolicy::TryApplyDamage(DamageRequest);
	if (!DamageResult.WasApplied())
	{
		return false;
	}

	if (bStaggerBonus)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Heavy dealt %.0f damage with stagger bonus"), DamageAmount), FColor::Green);
	}
	else if (bLockedTargetDamage)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Heavy dealt %.0f damage to locked target"), DamageAmount), FColor::Green);
	}
	else
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Heavy dealt %.0f damage"), DamageAmount), FColor::Green);
	}

	AddAetherGauge(HeavyHitAetherGain, TEXT("Heavy hit"));
	PlayImpactFeedback(
		bStaggerBonus ? EAetherCombatFeedbackType::HeavyCounterHit : EAetherCombatFeedbackType::HeavyHit,
		HeavyHitCameraKickStrength,
		bStaggerBonus ? HeavyCounterHitStopDuration : HeavyHitStopDuration,
		bStaggerBonus ? TEXT("Heavy counter hit") : TEXT("Heavy hit"),
		TargetActor);

	RefreshExecutionOpportunityAfterHeavyCounter(TargetActor, bStaggerBonus);

	return true;
}

void UAetherCombatComponent::RefreshExecutionOpportunityAfterHeavyCounter(AActor* TargetActor, bool bStaggerBonus) const
{
	if (!bStaggerBonus || HeavyCounterExecutionStaggerRefreshDuration <= 0.0f)
	{
		return;
	}

	AAetherEnemyBase* Enemy = Cast<AAetherEnemyBase>(TargetActor);
	if (!Enemy)
	{
		return;
	}

	UAetherHealthComponent* HealthComponent = Enemy->GetHealthComponent();
	if (!HealthComponent || HealthComponent->IsDead())
	{
		return;
	}

	const float MaxHealthValue = FMath::Max(HealthComponent->GetMaxHealth(), 1.0f);
	const float HealthPercent = HealthComponent->GetCurrentHealth() / MaxHealthValue;
	if (HealthPercent > ExecutionHealthThresholdPercent)
	{
		return;
	}

	Enemy->RefreshParryStagger(HeavyCounterExecutionStaggerRefreshDuration);
	ShowCombatDebugMessage(TEXT("Execution opportunity refreshed"), FColor::Purple);
}

AAetherEnemyBase* UAetherCombatComponent::FindExecutionTarget() const
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	UWorld* World = GetWorld();
	if (!Character || !World)
	{
		return nullptr;
	}

	if (const UAetherLockOnComponent* LockOnComponent = Character->GetLockOnComponent())
	{
		AAetherEnemyBase* LockedEnemy = Cast<AAetherEnemyBase>(LockOnComponent->GetLockedTarget());
		if (IsEnemyExecutionReady(LockedEnemy))
		{
			return LockedEnemy;
		}
	}

	AAetherEnemyBase* BestEnemy = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();
	const FVector PlayerLocation = Character->GetActorLocation();

	for (TActorIterator<AAetherEnemyBase> EnemyIt(World); EnemyIt; ++EnemyIt)
	{
		AAetherEnemyBase* Enemy = *EnemyIt;
		if (!IsEnemyExecutionReady(Enemy))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(PlayerLocation, Enemy->GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestEnemy = Enemy;
		}
	}

	return BestEnemy;
}

void UAetherCombatComponent::ResolveExecutionImpact()
{
	if (bExecutionImpactResolved)
	{
		return;
	}

	bExecutionImpactResolved = true;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExecutionImpactTimerHandle);
	}

	AAetherfallCharacter* Character = OwnerCharacter.Get();
	AAetherEnemyBase* ExecutionTarget = PendingExecutionTarget.Get();
	PendingExecutionTarget.Reset();

	if (!Character || !ExecutionTarget)
	{
		ShowCombatDebugMessage(TEXT("Execution failed"), FColor::Red);
		return;
	}

	FAetherCombatDamageRequest DamageRequest;
	DamageRequest.TargetActor = ExecutionTarget;
	DamageRequest.DamageCauser = Character;
	DamageRequest.DamageAmount = ExecutionDamage;
	const FAetherCombatDamageResult DamageResult = FAetherCombatDamagePolicy::TryApplyDamage(DamageRequest);
	if (DamageResult.WasApplied())
	{
		ShowCombatDebugMessage(TEXT("Execution performed"), FColor::Yellow);
		AddAetherGauge(ExecutionAetherGain, TEXT("Execution"));
		PlayImpactFeedback(EAetherCombatFeedbackType::Execution, ExecutionCameraKickStrength, ExecutionHitStopDuration, TEXT("Execution"), ExecutionTarget);
	}
	else
	{
		ExecutionTarget->SetPendingExecutionDeathMontage(nullptr);
		ShowCombatDebugMessage(TEXT("Execution failed"), FColor::Red);
	}
}

void UAetherCombatComponent::HandleExecutionImpactFallback()
{
	if (!bIsExecuting || bExecutionImpactResolved)
	{
		return;
	}

	ShowCombatDebugMessage(TEXT("Execution impact fallback"), FColor::Yellow);
	ResolveExecutionImpact();
}

void UAetherCombatComponent::ClearPendingExecutionImpact()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExecutionImpactTimerHandle);
	}

	if (!bExecutionImpactResolved)
	{
		if (AAetherEnemyBase* ExecutionTarget = PendingExecutionTarget.Get())
		{
			ExecutionTarget->SetPendingExecutionDeathMontage(nullptr);
		}
	}

	PendingExecutionTarget.Reset();
	bExecutionImpactResolved = false;
}

const FAetherExecutionVariant* UAetherCombatComponent::SelectExecutionVariant()
{
	if (ExecutionVariants.Num() <= 0)
	{
		return nullptr;
	}

	const int32 StartIndex = FMath::Clamp(NextExecutionVariantIndex, 0, ExecutionVariants.Num() - 1);
	for (int32 Offset = 0; Offset < ExecutionVariants.Num(); ++Offset)
	{
		const int32 VariantIndex = (StartIndex + Offset) % ExecutionVariants.Num();
		const FAetherExecutionVariant& Variant = ExecutionVariants[VariantIndex];
		if (Variant.PlayerExecutionMontage || Variant.EnemyDeathMontage)
		{
			NextExecutionVariantIndex = (VariantIndex + 1) % ExecutionVariants.Num();
			return &Variant;
		}
	}

	return nullptr;
}

int32 UAetherCombatComponent::SuppressNearbyEnemiesForExecution(AAetherEnemyBase* ExecutionTarget, float SuppressionDuration) const
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	UWorld* World = GetWorld();
	if (!Character || !World || ExecutionCrowdPauseRadius <= 0.0f || SuppressionDuration <= 0.0f)
	{
		return 0;
	}

	int32 SuppressedEnemyCount = 0;
	const FVector PlayerLocation = Character->GetActorLocation();
	const float PauseRadiusSquared = FMath::Square(ExecutionCrowdPauseRadius);

	for (TActorIterator<AAetherEnemyBase> EnemyIt(World); EnemyIt; ++EnemyIt)
	{
		AAetherEnemyBase* Enemy = *EnemyIt;
		if (!Enemy || Enemy == ExecutionTarget || !Enemy->GetHealthComponent() || Enemy->GetHealthComponent()->IsDead())
		{
			continue;
		}

		if (FVector::DistSquared2D(PlayerLocation, Enemy->GetActorLocation()) > PauseRadiusSquared)
		{
			continue;
		}

		Enemy->ApplyExecutionSuppression(Character, SuppressionDuration);
		++SuppressedEnemyCount;
	}

	return SuppressedEnemyCount;
}

FAetherGuardStaminaTuning UAetherCombatComponent::BuildGuardStaminaTuning() const
{
	FAetherGuardStaminaTuning GuardTuning;
	GuardTuning.BaseCost = GuardStaminaCostPerHit;
	GuardTuning.bScaleByIncomingDamage = bScaleGuardStaminaCostByIncomingDamage;
	GuardTuning.DamageReference = GuardStaminaDamageReference;
	GuardTuning.MinCost = MinGuardStaminaCostPerHit;
	GuardTuning.MaxCost = MaxGuardStaminaCostPerHit;
	return GuardTuning;
}

const TArray<float>& UAetherCombatComponent::ResolveLightAttackStaminaCosts() const
{
	if (CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideLightAttack())
	{
		const TArray<float>& AssetCosts = CombatActionDataAsset->GetLightAttackData().GetStaminaCosts();
		if (AssetCosts.Num() > 0)
		{
			return AssetCosts;
		}
	}

	return LightAttackStaminaCosts;
}

const TArray<float>& UAetherCombatComponent::ResolveLightAttackDamageValues() const
{
	if (CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideLightAttack())
	{
		const TArray<float>& AssetDamageValues = CombatActionDataAsset->GetLightAttackData().GetDamageValues();
		if (AssetDamageValues.Num() > 0)
		{
			return AssetDamageValues;
		}
	}

	return LightAttackDamageValues;
}

float UAetherCombatComponent::ResolveLightAttackDuration() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideLightAttack()
		? CombatActionDataAsset->GetLightAttackData().GetDuration()
		: LightAttackDuration;
}

float UAetherCombatComponent::ResolveLightTraceDistance() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideLightAttack()
		? CombatActionDataAsset->GetLightAttackData().GetTraceDistance()
		: PrototypeTraceDistance;
}

float UAetherCombatComponent::ResolveLightTraceRadius() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideLightAttack()
		? CombatActionDataAsset->GetLightAttackData().GetTraceRadius()
		: PrototypeTraceRadius;
}

float UAetherCombatComponent::ResolveHeavyAttackDuration() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideHeavyAttack()
		? CombatActionDataAsset->GetHeavyAttackData().GetDuration()
		: HeavyAttackDuration;
}

float UAetherCombatComponent::ResolveHeavyAttackImpactDelay() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideHeavyAttack()
		? CombatActionDataAsset->GetHeavyAttackData().GetImpactDelay()
		: HeavyAttackImpactDelay;
}

float UAetherCombatComponent::ResolveHeavyAttackStaminaCost() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideHeavyAttack()
		? CombatActionDataAsset->GetHeavyAttackData().GetStaminaCost()
		: HeavyAttackStaminaCost;
}

float UAetherCombatComponent::ResolveHeavyAttackDamage() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideHeavyAttack()
		? CombatActionDataAsset->GetHeavyAttackData().GetDamage()
		: HeavyAttackDamage;
}

float UAetherCombatComponent::ResolveHeavyStaggerDamageMultiplier() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideHeavyAttack()
		? CombatActionDataAsset->GetHeavyAttackData().GetStaggerDamageMultiplier()
		: HeavyStaggerDamageMultiplier;
}

float UAetherCombatComponent::ResolveHeavyTraceDistance() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideHeavyAttack()
		? CombatActionDataAsset->GetHeavyAttackData().GetTraceDistance()
		: HeavyTraceDistance;
}

float UAetherCombatComponent::ResolveHeavyTraceRadius() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideHeavyAttack()
		? CombatActionDataAsset->GetHeavyAttackData().GetTraceRadius()
		: HeavyTraceRadius;
}

float UAetherCombatComponent::ResolveAetherSlashCost() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideAetherSlash()
		? CombatActionDataAsset->GetAetherSlashData().GetCost()
		: AetherSlashCost;
}

float UAetherCombatComponent::ResolveAetherSlashDamage() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideAetherSlash()
		? CombatActionDataAsset->GetAetherSlashData().GetDamage()
		: AetherSlashDamage;
}

float UAetherCombatComponent::ResolveAetherSlashDuration() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideAetherSlash()
		? CombatActionDataAsset->GetAetherSlashData().GetDuration()
		: AetherSlashDuration;
}

float UAetherCombatComponent::ResolveAetherSlashImpactDelay() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideAetherSlash()
		? CombatActionDataAsset->GetAetherSlashData().GetImpactDelay()
		: AetherSlashImpactDelay;
}

float UAetherCombatComponent::ResolveAetherSlashCooldown() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideAetherSlash()
		? CombatActionDataAsset->GetAetherSlashData().GetCooldown()
		: AetherSlashCooldown;
}

float UAetherCombatComponent::ResolveAetherSlashProjectileSpeed() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideAetherSlash()
		? CombatActionDataAsset->GetAetherSlashData().GetProjectileSpeed()
		: AetherSlashProjectileSpeed;
}

float UAetherCombatComponent::ResolveAetherSlashTraceDistance() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideAetherSlash()
		? CombatActionDataAsset->GetAetherSlashData().GetTraceDistance()
		: AetherSlashTraceDistance;
}

float UAetherCombatComponent::ResolveAetherSlashTraceRadius() const
{
	return CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideAetherSlash()
		? CombatActionDataAsset->GetAetherSlashData().GetTraceRadius()
		: AetherSlashTraceRadius;
}

UAnimMontage* UAetherCombatComponent::ResolveHeavyAttackMontage(bool bCounterHeavy) const
{
	if (CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideHeavyAttack())
	{
		const FAetherCombatHeavyAttackData& HeavyData = CombatActionDataAsset->GetHeavyAttackData();
		if (bCounterHeavy && HeavyData.GetCounterAttackMontage())
		{
			return HeavyData.GetCounterAttackMontage();
		}

		if (HeavyData.GetAttackMontage())
		{
			return HeavyData.GetAttackMontage();
		}
	}

	return bCounterHeavy && HeavyCounterAttackMontage ? HeavyCounterAttackMontage.Get() : HeavyAttackMontage.Get();
}

UAnimMontage* UAetherCombatComponent::ResolveAetherSlashMontage() const
{
	if (CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideAetherSlash())
	{
		if (UAnimMontage* AssetMontage = CombatActionDataAsset->GetAetherSlashData().GetMontage())
		{
			return AssetMontage;
		}
	}

	return AetherSlashMontage.Get();
}

bool UAetherCombatComponent::IsEnemyExecutionReady(const AAetherEnemyBase* Enemy) const
{
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character || !Enemy || !Enemy->IsParryStaggered())
	{
		return false;
	}

	const UAetherHealthComponent* HealthComponent = Enemy->GetHealthComponent();
	if (!HealthComponent || HealthComponent->IsDead())
	{
		return false;
	}

	const float MaxHealthValue = FMath::Max(HealthComponent->GetMaxHealth(), 1.0f);
	const float HealthPercent = HealthComponent->GetCurrentHealth() / MaxHealthValue;
	if (HealthPercent > ExecutionHealthThresholdPercent)
	{
		return false;
	}

	return FVector::DistSquared2D(Character->GetActorLocation(), Enemy->GetActorLocation()) <= FMath::Square(ExecutionRange);
}

bool UAetherCombatComponent::ApplyIncomingDamage(float DamageAmount, AActor* DamageCauser, bool bTriggerHitReaction)
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return false;
	}

	FAetherCombatDamageRequest DamageRequest;
	DamageRequest.TargetActor = Character;
	DamageRequest.DamageCauser = DamageCauser;
	DamageRequest.DamageAmount = DamageAmount;
	const FAetherCombatDamageResult DamageResult = FAetherCombatDamagePolicy::TryApplyDamage(DamageRequest);
	UAetherHealthComponent* HealthComponent = DamageResult.HealthComponent;
	if (DamageResult.WasApplied() && HealthComponent)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Player HP: %.0f / %.0f"), HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth()), FColor::Red);
		PlayImpactFeedback(
			bTriggerHitReaction ? EAetherCombatFeedbackType::PlayerHit : EAetherCombatFeedbackType::GuardBlock,
			bTriggerHitReaction ? PlayerHitCameraKickStrength : GuardBlockCameraKickStrength,
			bTriggerHitReaction ? PlayerHitStopDuration : GuardBlockHitStopDuration,
			bTriggerHitReaction ? TEXT("Player hit") : TEXT("Guard block"),
			DamageCauser);
		UpdatePlayerDangerAudioCue(HealthComponent);
		if (!HealthComponent->IsDead())
		{
			BeginDamageInvulnerability();
			if (bTriggerHitReaction)
			{
				bDelayEnemyAttackAfterDamageInvulnerability = true;
				BeginHitReaction(DamageCauser);
			}
		}
		return true;
	}

	return false;
}

void UAetherCombatComponent::ClearCombatRuntimeState()
{
	ApplyCombatActionTimerClearPlan(FAetherCombatActionTimerPolicy::BuildClearPlan(EAetherCombatActionTimerClearReason::RuntimeReset));

	ClearPendingExecutionImpact();
	SetCombatActionMode(EAetherCombatActionMode::Idle);
	bQueuedLightAttack = false;
	bQueuedExecution = false;
	RestoreGuardMovementModifier();
	bIsDamageInvulnerable = false;
	bDelayEnemyAttackAfterDamageInvulnerability = false;
	PreferredHitTarget.Reset();
	ClearDodgeMovementState();
	ResetCombo();
}

bool UAetherCombatComponent::IsOwnerDead() const
{
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	return Character && Character->GetHealthComponent() && Character->GetHealthComponent()->IsDead();
}

void UAetherCombatComponent::HandleOwnerDeath(UAetherHealthComponent* DeadHealthComponent, AActor* DamageCauser)
{
	ClearCombatRuntimeState();
	SetOwnerMovementEnabled(false);
	ShowCombatDebugMessage(TEXT("Player defeated"), FColor::Red);

	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->SchedulePrototypePlayerRetryAfterDefeat();
	}
}

void UAetherCombatComponent::ApplyPrototypeDamage(const TArray<FHitResult>& HitResults, int32 ComboStep)
{
	const float DamageAmount = FAetherCombatActionTuningPolicy::SelectLightAttackDamage(ComboStep, ResolveLightAttackDamageValues());
	AActor* DamageCauser = GetOwner();
	const FAetherCombatTargetSelection TargetSelection =
		FAetherCombatTargetSelectionPolicy::BuildTargetSelection(HitResults, DamageCauser, nullptr, GetLockedCombatTarget());

	if (TargetSelection.IsLockedPriority())
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Light %d locked target priority"), ComboStep), FColor::Cyan);
		ApplyPrototypeDamageToActor(TargetSelection.PriorityTarget, ComboStep, DamageAmount, DamageCauser, true);
		return;
	}

	for (AActor* HitActor : TargetSelection.AdditionalTargets)
	{
		ApplyPrototypeDamageToActor(HitActor, ComboStep, DamageAmount, DamageCauser, false);
	}
}

AActor* UAetherCombatComponent::GetLockedCombatTarget() const
{
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character || !Character->GetLockOnComponent())
	{
		return nullptr;
	}

	return Character->GetLockOnComponent()->GetLockedTarget();
}

bool UAetherCombatComponent::ApplyPrototypeDamageToActor(AActor* TargetActor, int32 ComboStep, float DamageAmount, AActor* DamageCauser, bool bLockedTargetDamage)
{
	FAetherCombatDamageRequest DamageRequest;
	DamageRequest.TargetActor = TargetActor;
	DamageRequest.DamageCauser = DamageCauser;
	DamageRequest.DamageAmount = DamageAmount;
	const FAetherCombatDamageResult DamageResult = FAetherCombatDamagePolicy::TryApplyDamage(DamageRequest);
	if (!DamageResult.WasApplied())
	{
		return false;
	}

	if (bLockedTargetDamage)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Light %d dealt %.0f damage to locked target"), ComboStep, DamageAmount), FColor::Green);
	}
	else
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Light %d dealt %.0f damage"), ComboStep, DamageAmount), FColor::Green);
	}

	AddAetherGauge(LightHitAetherGain, TEXT("Light hit"));
	PlayImpactFeedback(EAetherCombatFeedbackType::LightHit, LightHitCameraKickStrength, LightHitStopDuration, TEXT("Light hit"), TargetActor);

	return true;
}

void UAetherCombatComponent::StopOwnerMovementForCombatAction(bool bRespectActionSetting)
{
	if (bRespectActionSetting && !bStopMovementOnCombatAction)
	{
		return;
	}

	AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return;
	}

	Character->ClearDesiredMovementDirection();

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent || MovementComponent->MovementMode == MOVE_None)
	{
		return;
	}

	MovementComponent->StopMovementImmediately();
}

void UAetherCombatComponent::SetOwnerMovementEnabled(bool bEnabled)
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return;
	}

	Character->ClearDesiredMovementDirection();
	if (!bEnabled && Character->GetLockOnComponent())
	{
		Character->GetLockOnComponent()->ClearLockOn();
	}

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	MovementComponent->StopMovementImmediately();
	if (bEnabled)
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
		ShowCombatDebugMessage(TEXT("Player movement restored"), FColor::Green);
	}
	else
	{
		MovementComponent->DisableMovement();
		ShowCombatDebugMessage(TEXT("Player movement locked"), FColor::Red);
	}
}

void UAetherCombatComponent::ShowCombatDebugMessage(const FString& Message, const FColor& Color) const
{
	UE_LOG(LogTemp, Log, TEXT("[AetherCombat] %s"), *Message);

	if (bShowCombatScreenDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.25f, Color, Message);
	}
}

void UAetherCombatComponent::PlayImpactCameraFeedback(float Strength, const FString& Reason) const
{
	AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character || Strength <= 0.0f)
	{
		return;
	}

	Character->PlayCameraImpactFeedback(Strength, ImpactCameraKickDuration);
	ShowCombatDebugMessage(FString::Printf(TEXT("Camera impact kick (%s)"), *Reason), FColor::Silver);
}

void UAetherCombatComponent::PlayImpactFeedback(EAetherCombatFeedbackType FeedbackType, float CameraStrength, float HitStopDuration, const FString& Reason, AActor* ImpactTarget) const
{
	PlayImpactCameraFeedback(CameraStrength, Reason);
	PlayFeedbackAssets(FeedbackType, ImpactTarget);

	if (HitStopDuration <= 0.0f)
	{
		return;
	}

	ApplyHitStopToActor(OwnerCharacter.Get(), HitStopDuration);
	if (ImpactTarget && ImpactTarget != OwnerCharacter.Get())
	{
		ApplyHitStopToActor(ImpactTarget, HitStopDuration);
	}

	ShowCombatDebugMessage(FString::Printf(TEXT("Hit stop (%s) / %.2f"), *Reason, HitStopDuration), FColor::Silver);
}

void UAetherCombatComponent::PlayActionMontage(UAnimMontage* Montage, const FString& Reason) const
{
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character || !Montage)
	{
		return;
	}

	USkeletalMeshComponent* MeshComponent = Character->GetMesh();
	USkeletalMesh* SkeletalMesh = MeshComponent ? MeshComponent->GetSkeletalMeshAsset() : nullptr;
	if (!MeshComponent || !SkeletalMesh)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("No Skeletal Mesh for montage (%s)"), *Reason), FColor::Yellow);
		return;
	}

	if (Montage->GetSkeleton() && SkeletalMesh->GetSkeleton() && Montage->GetSkeleton() != SkeletalMesh->GetSkeleton())
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Montage skeleton mismatch (%s). Retarget montage to current Mesh skeleton."), *Reason), FColor::Yellow);
		return;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("No AnimInstance for montage (%s). Assign an AnimBP to Mesh."), *Reason), FColor::Yellow);
		return;
	}

	const float PlayedLength = AnimInstance->Montage_Play(Montage);
	if (PlayedLength <= 0.0f)
	{
		ShowCombatDebugMessage(FString::Printf(TEXT("Montage failed (%s)"), *Reason), FColor::Yellow);
		return;
	}

	ShowCombatDebugMessage(FString::Printf(TEXT("Montage played (%s)"), *Reason), FColor::Silver);
}

void UAetherCombatComponent::PlayActionSound(USoundBase* Sound, float VolumeMultiplier) const
{
	UWorld* World = GetWorld();
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!World || !Character || !Sound)
	{
		return;
	}

	const float Volume = GetRandomizedAudioVolume(ActionSoundVolume * VolumeMultiplier, ActionSoundVolumeVariance);
	if (Volume <= 0.0f)
	{
		return;
	}

	UAetherAudioSettingsLibrary::SpawnSoundAtLocationForCategory(
		this,
		Sound,
		Character->GetActorLocation(),
		EAetherAudioCategory::Sfx,
		Volume,
		GetRandomizedAudioPitch(ActionSoundPitchMin, ActionSoundPitchMax));
}

FAetherPlayerDangerCueConfig UAetherCombatComponent::BuildPlayerDangerCueConfig() const
{
	FAetherPlayerDangerCueConfig Config;
	Config.bEnabled = bEnablePlayerDangerWarningSounds;
	Config.LowHealthThresholdPercent = LowHealthWarningThresholdPercent;
	Config.CriticalHealthThresholdPercent = CriticalHealthWarningThresholdPercent;
	return Config;
}

void UAetherCombatComponent::UpdatePlayerDangerAudioCue(const UAetherHealthComponent* HealthComponent)
{
	switch (FAetherCombatAudioCuePolicy::EvaluatePlayerDangerCue(HealthComponent, BuildPlayerDangerCueConfig(), PlayerDangerCueState))
	{
	case EAetherPlayerDangerCue::Defeated:
		PlayWarningSound(DefeatedWarningSound.Get(), TEXT("Defeated warning"));
		return;

	case EAetherPlayerDangerCue::CriticalHealth:
		PlayWarningSound(CriticalHealthWarningSound.Get(), TEXT("Critical HP warning"));
		return;

	case EAetherPlayerDangerCue::LowHealth:
		PlayWarningSound(LowHealthWarningSound.Get(), TEXT("Low HP warning"));
		return;

	case EAetherPlayerDangerCue::None:
	default:
		return;
	}
}

void UAetherCombatComponent::ResetPlayerDangerAudioCues()
{
	FAetherCombatAudioCuePolicy::ResetPlayerDangerState(PlayerDangerCueState);
}

void UAetherCombatComponent::PlayWarningSound(USoundBase* Sound, const FString& Reason) const
{
	UWorld* World = GetWorld();
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!World || !Character || !Sound)
	{
		return;
	}

	const float Volume = GetRandomizedAudioVolume(WarningSoundVolume, WarningSoundVolumeVariance);
	if (Volume <= 0.0f)
	{
		return;
	}

	UAetherAudioSettingsLibrary::SpawnSoundAtLocationForCategory(
		this,
		Sound,
		Character->GetActorLocation(),
		EAetherAudioCategory::Sfx,
		Volume,
		GetRandomizedAudioPitch(WarningSoundPitchMin, WarningSoundPitchMax));
	ShowCombatDebugMessage(FString::Printf(TEXT("Warning sound played (%s)"), *Reason), FColor::Silver);
}

FAetherResourceCueConfig UAetherCombatComponent::BuildResourceCueConfig() const
{
	FAetherResourceCueConfig Config;
	Config.bEnabled = bEnableAetherResourceCueSounds;
	Config.MaxAetherGauge = MaxAetherGauge;
	Config.AetherSlashCost = ResolveAetherSlashCost();
	return Config;
}

void UAetherCombatComponent::UpdateAetherResourceAudioCues(float PreviousAetherGauge)
{
	switch (FAetherCombatAudioCuePolicy::EvaluateResourceCue(PreviousAetherGauge, CurrentAetherGauge, BuildResourceCueConfig(), ResourceCueState))
	{
	case EAetherResourceCue::AetherGaugeFull:
		PlayAetherResourceSound(AetherGaugeFullSound.Get(), TEXT("Aether full"));
		return;

	case EAetherResourceCue::AetherSlashReady:
		PlayAetherResourceSound(AetherSlashReadySound.Get(), TEXT("Aether Slash ready"));
		return;

	case EAetherResourceCue::None:
	default:
		return;
	}
}

void UAetherCombatComponent::ResetAetherResourceAudioCues()
{
	FAetherCombatAudioCuePolicy::ResetResourceState(ResourceCueState);
}

void UAetherCombatComponent::PlayAetherResourceSound(USoundBase* Sound, const FString& Reason) const
{
	UWorld* World = GetWorld();
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!World || !Character || !Sound)
	{
		return;
	}

	const float Volume = GetRandomizedAudioVolume(AetherResourceSoundVolume, AetherResourceSoundVolumeVariance);
	if (Volume <= 0.0f)
	{
		return;
	}

	UAetherAudioSettingsLibrary::SpawnSoundAtLocationForCategory(
		this,
		Sound,
		Character->GetActorLocation(),
		EAetherAudioCategory::Sfx,
		Volume,
		GetRandomizedAudioPitch(AetherResourceSoundPitchMin, AetherResourceSoundPitchMax));
	ShowCombatDebugMessage(FString::Printf(TEXT("Aether cue sound played (%s)"), *Reason), FColor::Silver);
}

float UAetherCombatComponent::GetRandomizedAudioVolume(float BaseVolume, float Variance) const
{
	return FAetherCombatAudioCuePolicy::GetRandomizedVolume(BaseVolume, Variance);
}

float UAetherCombatComponent::GetRandomizedAudioPitch(float MinPitch, float MaxPitch) const
{
	return FAetherCombatAudioCuePolicy::GetRandomizedPitch(MinPitch, MaxPitch);
}

UAnimMontage* UAetherCombatComponent::GetLightAttackMontage(int32 ComboStep) const
{
	const TArray<TObjectPtr<UAnimMontage>>* MontageSource = &LightAttackMontages;
	if (CombatActionDataAsset && CombatActionDataAsset->ShouldOverrideLightAttack())
	{
		const TArray<TObjectPtr<UAnimMontage>>& AssetMontages = CombatActionDataAsset->GetLightAttackData().GetMontages();
		if (AssetMontages.Num() > 0)
		{
			MontageSource = &AssetMontages;
		}
	}

	const int32 MontageIndex = ComboStep - 1;
	if (MontageSource->IsValidIndex(MontageIndex))
	{
		return (*MontageSource)[MontageIndex];
	}

	return MontageSource->Num() > 0 ? MontageSource->Last() : nullptr;
}

UAnimMontage* UAetherCombatComponent::GetDodgeMontageForDirection(const FVector& WorldDirection) const
{
	const AAetherfallCharacter* Character = OwnerCharacter.Get();
	if (!Character)
	{
		return DodgeMontage.Get();
	}

	const FVector Direction = WorldDirection.GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		return DodgeMontage.Get();
	}

	const float ForwardDot = FVector::DotProduct(Character->GetActorForwardVector().GetSafeNormal2D(), Direction);
	const float RightDot = FVector::DotProduct(Character->GetActorRightVector().GetSafeNormal2D(), Direction);

	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot))
	{
		if (ForwardDot >= 0.0f)
		{
			return DodgeForwardMontage ? DodgeForwardMontage.Get() : DodgeMontage.Get();
		}

		return DodgeBackwardMontage ? DodgeBackwardMontage.Get() : DodgeMontage.Get();
	}

	if (RightDot >= 0.0f)
	{
		return DodgeRightMontage ? DodgeRightMontage.Get() : DodgeMontage.Get();
	}

	return DodgeLeftMontage ? DodgeLeftMontage.Get() : DodgeMontage.Get();
}

FAetherCombatFeedbackAssets UAetherCombatComponent::BuildCombatFeedbackAssets() const
{
	FAetherCombatFeedbackAssets FeedbackAssets;
	FeedbackAssets.LightHitImpactEffect = LightHitImpactEffect.Get();
	FeedbackAssets.HeavyHitImpactEffect = HeavyHitImpactEffect.Get();
	FeedbackAssets.HeavyCounterHitImpactEffect = HeavyCounterHitImpactEffect.Get();
	FeedbackAssets.ExecutionImpactEffect = ExecutionImpactEffect.Get();
	FeedbackAssets.ParryImpactEffect = ParryImpactEffect.Get();
	FeedbackAssets.PlayerHitImpactEffect = PlayerHitImpactEffect.Get();
	FeedbackAssets.GuardBlockImpactEffect = GuardBlockImpactEffect.Get();
	FeedbackAssets.AetherSlashImpactEffect = AetherSlashImpactEffect.Get();
	FeedbackAssets.LightHitSound = LightHitSound.Get();
	FeedbackAssets.HeavyHitSound = HeavyHitSound.Get();
	FeedbackAssets.HeavyCounterHitSound = HeavyCounterHitSound.Get();
	FeedbackAssets.ExecutionSound = ExecutionSound.Get();
	FeedbackAssets.ParrySuccessSound = ParrySuccessSound.Get();
	FeedbackAssets.PlayerHitSound = PlayerHitSound.Get();
	FeedbackAssets.GuardBlockSound = GuardBlockSound.Get();
	FeedbackAssets.AetherSlashHitSound = AetherSlashHitSound.Get();
	return FeedbackAssets;
}

void UAetherCombatComponent::PlayFeedbackAssets(EAetherCombatFeedbackType FeedbackType, AActor* ImpactTarget) const
{
	UWorld* World = GetWorld();
	if (!World || !ImpactTarget)
	{
		return;
	}

	const FVector ImpactLocation = GetFeedbackImpactLocation(ImpactTarget);
	const FAetherCombatFeedbackAssets FeedbackAssets = BuildCombatFeedbackAssets();
	if (UParticleSystem* ImpactEffect = FAetherCombatFeedbackPolicy::SelectImpactEffect(FeedbackType, FeedbackAssets))
	{
		UGameplayStatics::SpawnEmitterAtLocation(World, ImpactEffect, ImpactLocation, FRotator::ZeroRotator, FVector(ImpactEffectScale));
	}

	if (USoundBase* ImpactSound = FAetherCombatFeedbackPolicy::SelectImpactSound(FeedbackType, FeedbackAssets))
	{
		UAetherAudioSettingsLibrary::SpawnSoundAtLocationForCategory(
			this,
			ImpactSound,
			ImpactLocation,
			EAetherAudioCategory::Sfx,
			GetRandomizedAudioVolume(ImpactSoundVolume, ImpactSoundVolumeVariance),
			GetRandomizedAudioPitch(ImpactSoundPitchMin, ImpactSoundPitchMax));
	}
}

FVector UAetherCombatComponent::GetFeedbackImpactLocation(AActor* ImpactTarget) const
{
	if (!ImpactTarget)
	{
		return FVector::ZeroVector;
	}

	FVector TargetOrigin = ImpactTarget->GetActorLocation();
	FVector TargetExtent = FVector::ZeroVector;
	ImpactTarget->GetActorBounds(false, TargetOrigin, TargetExtent);

	FVector ImpactLocation = TargetOrigin;
	const float TargetHeightAlpha = FMath::Clamp(ImpactEffectTargetHeightAlpha, 0.0f, 1.0f);
	ImpactLocation.Z = TargetOrigin.Z - TargetExtent.Z + (TargetExtent.Z * 2.0f * TargetHeightAlpha);

	if (const AAetherfallCharacter* Character = OwnerCharacter.Get())
	{
		FVector ToTarget = ImpactTarget->GetActorLocation() - Character->GetActorLocation();
		ToTarget.Z = 0.0f;
		const FVector DirectionToTarget = ToTarget.GetSafeNormal();
		if (!DirectionToTarget.IsNearlyZero())
		{
			ImpactLocation -= DirectionToTarget * ImpactEffectForwardOffset;
		}
	}

	return ImpactLocation;
}

void UAetherCombatComponent::ApplyHitStopToActor(AActor* Actor, float Duration) const
{
	UWorld* World = GetWorld();
	if (!Actor || !World || Duration <= 0.0f)
	{
		return;
	}

	const float AppliedTimeDilation = FMath::Clamp(HitStopTimeDilation, 0.01f, 1.0f);
	const float PreviousTimeDilation = Actor->CustomTimeDilation;
	const float RestoreTimeDilation = PreviousTimeDilation <= AppliedTimeDilation + KINDA_SMALL_NUMBER
		? 1.0f
		: PreviousTimeDilation;
	Actor->CustomTimeDilation = AppliedTimeDilation;

	TWeakObjectPtr<AActor> WeakActor = Actor;
	FTimerDelegate RestoreDelegate = FTimerDelegate::CreateLambda([WeakActor, RestoreTimeDilation]()
	{
		if (AActor* RestoredActor = WeakActor.Get())
		{
			RestoredActor->CustomTimeDilation = RestoreTimeDilation;
		}
	});

	FTimerHandle RestoreTimerHandle;
	World->GetTimerManager().SetTimer(RestoreTimerHandle, RestoreDelegate, Duration, false);
}
