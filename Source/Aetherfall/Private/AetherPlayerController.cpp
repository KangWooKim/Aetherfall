#include "AetherPlayerController.h"

#include "AetherCinematicDirectorSubsystem.h"
#include "AetherPauseMenuComponent.h"
#include "AetherSettingsSubsystem.h"
#include "AetherCombatComponent.h"
#include "AetherGameModeBase.h"
#include "AetherHealthComponent.h"
#include "AetherInventoryComponent.h"
#include "AetherInteractionComponent.h"
#include "AetherLockOnComponent.h"
#include "AetherfallCharacter.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	UInputModifierNegate* CreateNegateModifier(UObject* Outer)
	{
		return NewObject<UInputModifierNegate>(Outer);
	}

	UInputModifierSwizzleAxis* CreateYXZSwizzleModifier(UObject* Outer)
	{
		UInputModifierSwizzleAxis* Modifier = NewObject<UInputModifierSwizzleAxis>(Outer);
		Modifier->Order = EInputAxisSwizzle::YXZ;
		return Modifier;
	}
}

AAetherPlayerController::AAetherPlayerController()
{
	bShowMouseCursor = false;
	PauseMenuComponent = CreateDefaultSubobject<UAetherPauseMenuComponent>(TEXT("PauseMenuComponent"));

	DefaultMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("IMC_DefaultPlayer"));

	MoveAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Move"));
	MoveAction->ValueType = EInputActionValueType::Axis2D;

	LookAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;

	LightAttackAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_LightAttack"));
	LightAttackAction->ValueType = EInputActionValueType::Boolean;

	HeavyAttackAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_HeavyAttack"));
	HeavyAttackAction->ValueType = EInputActionValueType::Boolean;

	ExecutionAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Execution"));
	ExecutionAction->ValueType = EInputActionValueType::Boolean;

	AetherSlashAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_AetherSlash"));
	AetherSlashAction->ValueType = EInputActionValueType::Boolean;

	UseQuickItemAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_UseQuickItem"));
	UseQuickItemAction->ValueType = EInputActionValueType::Boolean;

	InteractAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Interact"));
	InteractAction->ValueType = EInputActionValueType::Boolean;

	DodgeAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Dodge"));
	DodgeAction->ValueType = EInputActionValueType::Boolean;

	GuardAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Guard"));
	GuardAction->ValueType = EInputActionValueType::Boolean;

	ParryAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Parry"));
	ParryAction->ValueType = EInputActionValueType::Boolean;

	DebugIncomingHitAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DebugIncomingHit"));
	DebugIncomingHitAction->ValueType = EInputActionValueType::Boolean;

	DebugResetPlayerAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DebugResetPlayer"));
	DebugResetPlayerAction->ValueType = EInputActionValueType::Boolean;

	DebugResetRoundAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DebugResetRound"));
	DebugResetRoundAction->ValueType = EInputActionValueType::Boolean;

	DebugClearCheckpointProgressAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DebugClearCheckpointProgress"));
	DebugClearCheckpointProgressAction->ValueType = EInputActionValueType::Boolean;

	DialogueAdvanceAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_DialogueAdvance"));
	DialogueAdvanceAction->ValueType = EInputActionValueType::Boolean;

	PauseMenuAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_PauseMenu"));
	PauseMenuAction->ValueType = EInputActionValueType::Boolean;
	PauseMenuAction->bTriggerWhenPaused = true;

	LockOnAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_LockOn"));
	LockOnAction->ValueType = EInputActionValueType::Boolean;

	LockOnPreviousTargetAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_LockOnPreviousTarget"));
	LockOnPreviousTargetAction->ValueType = EInputActionValueType::Boolean;

	LockOnNextTargetAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_LockOnNextTarget"));
	LockOnNextTargetAction->ValueType = EInputActionValueType::Boolean;

	BuildDefaultInputMapping();
}

void AAetherPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (const UAetherSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr)
	{
		SetDisableHaptics(!Settings->GetCurrentSettings().Custom.bVibrationEnabled);
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AAetherPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AetherPlayerController requires Enhanced Input. Check DefaultInput.ini."));
		return;
	}

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAetherPlayerController::Move);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AAetherPlayerController::StopMove);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAetherPlayerController::Look);
	EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AAetherPlayerController::LightAttack);
	EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AAetherPlayerController::HeavyAttack);
	EnhancedInputComponent->BindAction(ExecutionAction, ETriggerEvent::Started, this, &AAetherPlayerController::Execution);
	EnhancedInputComponent->BindAction(AetherSlashAction, ETriggerEvent::Started, this, &AAetherPlayerController::AetherSlash);
	EnhancedInputComponent->BindAction(UseQuickItemAction, ETriggerEvent::Started, this, &AAetherPlayerController::UseQuickItem);
	EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AAetherPlayerController::Interact);
	EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &AAetherPlayerController::Dodge);
	EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Started, this, &AAetherPlayerController::StartGuard);
	EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Completed, this, &AAetherPlayerController::StopGuard);
	EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Started, this, &AAetherPlayerController::Parry);
	EnhancedInputComponent->BindAction(DebugIncomingHitAction, ETriggerEvent::Started, this, &AAetherPlayerController::DebugIncomingHit);
	EnhancedInputComponent->BindAction(DebugResetPlayerAction, ETriggerEvent::Started, this, &AAetherPlayerController::DebugResetPlayer);
	EnhancedInputComponent->BindAction(DebugResetRoundAction, ETriggerEvent::Started, this, &AAetherPlayerController::DebugResetCombatRound);
	EnhancedInputComponent->BindAction(DebugClearCheckpointProgressAction, ETriggerEvent::Started, this, &AAetherPlayerController::DebugClearPrototypeCheckpointProgress);
	EnhancedInputComponent->BindAction(DialogueAdvanceAction, ETriggerEvent::Started, this, &AAetherPlayerController::AdvanceDialogue);
	EnhancedInputComponent->BindAction(PauseMenuAction, ETriggerEvent::Started, this, &AAetherPlayerController::TogglePauseMenu);
	EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &AAetherPlayerController::ToggleLockOn);
	EnhancedInputComponent->BindAction(LockOnPreviousTargetAction, ETriggerEvent::Started, this, &AAetherPlayerController::SwitchLockOnTargetLeft);
	EnhancedInputComponent->BindAction(LockOnNextTargetAction, ETriggerEvent::Started, this, &AAetherPlayerController::SwitchLockOnTargetRight);
}

void AAetherPlayerController::BuildDefaultInputMapping()
{
	if (!DefaultMappingContext || !MoveAction || !LookAction || !LightAttackAction || !HeavyAttackAction || !ExecutionAction || !AetherSlashAction || !UseQuickItemAction || !InteractAction || !DodgeAction || !GuardAction || !ParryAction || !DebugIncomingHitAction || !DebugResetPlayerAction || !DebugResetRoundAction || !DebugClearCheckpointProgressAction || !DialogueAdvanceAction || !PauseMenuAction || !LockOnAction || !LockOnPreviousTargetAction || !LockOnNextTargetAction)
	{
		return;
	}

	DefaultMappingContext->MapKey(MoveAction, EKeys::D);

	FEnhancedActionKeyMapping& MoveLeft = DefaultMappingContext->MapKey(MoveAction, EKeys::A);
	MoveLeft.Modifiers.Add(CreateNegateModifier(DefaultMappingContext));

	FEnhancedActionKeyMapping& MoveForward = DefaultMappingContext->MapKey(MoveAction, EKeys::W);
	MoveForward.Modifiers.Add(CreateYXZSwizzleModifier(DefaultMappingContext));

	FEnhancedActionKeyMapping& MoveBackward = DefaultMappingContext->MapKey(MoveAction, EKeys::S);
	MoveBackward.Modifiers.Add(CreateNegateModifier(DefaultMappingContext));
	MoveBackward.Modifiers.Add(CreateYXZSwizzleModifier(DefaultMappingContext));

	DefaultMappingContext->MapKey(MoveAction, EKeys::Gamepad_Left2D);
	DefaultMappingContext->MapKey(LookAction, EKeys::Mouse2D);
	DefaultMappingContext->MapKey(LookAction, EKeys::Gamepad_Right2D);
	DefaultMappingContext->MapKey(LightAttackAction, EKeys::LeftMouseButton);
	DefaultMappingContext->MapKey(LightAttackAction, EKeys::Gamepad_FaceButton_Right);
	DefaultMappingContext->MapKey(HeavyAttackAction, EKeys::E);
	DefaultMappingContext->MapKey(HeavyAttackAction, EKeys::Gamepad_FaceButton_Top);
	DefaultMappingContext->MapKey(ExecutionAction, EKeys::F);
	DefaultMappingContext->MapKey(ExecutionAction, EKeys::Gamepad_FaceButton_Left);
	DefaultMappingContext->MapKey(AetherSlashAction, EKeys::X);
	DefaultMappingContext->MapKey(AetherSlashAction, EKeys::Gamepad_RightTrigger);
	DefaultMappingContext->MapKey(UseQuickItemAction, EKeys::H);
	DefaultMappingContext->MapKey(UseQuickItemAction, EKeys::Gamepad_DPad_Down);
	DefaultMappingContext->MapKey(InteractAction, EKeys::V);
	DefaultMappingContext->MapKey(InteractAction, EKeys::Gamepad_DPad_Up);
	DefaultMappingContext->MapKey(DodgeAction, EKeys::SpaceBar);
	DefaultMappingContext->MapKey(DodgeAction, EKeys::Gamepad_FaceButton_Bottom);
	DefaultMappingContext->MapKey(GuardAction, EKeys::RightMouseButton);
	DefaultMappingContext->MapKey(GuardAction, EKeys::Gamepad_LeftShoulder);
	DefaultMappingContext->MapKey(ParryAction, EKeys::Q);
	DefaultMappingContext->MapKey(ParryAction, EKeys::Gamepad_LeftTrigger);
	DefaultMappingContext->MapKey(DebugIncomingHitAction, EKeys::T);
	DefaultMappingContext->MapKey(DebugResetPlayerAction, EKeys::Y);
	DefaultMappingContext->MapKey(DebugResetRoundAction, EKeys::R);
	DefaultMappingContext->MapKey(DebugClearCheckpointProgressAction, EKeys::BackSpace);
	DefaultMappingContext->MapKey(DialogueAdvanceAction, EKeys::Enter);
	DefaultMappingContext->MapKey(PauseMenuAction, EKeys::Escape);
	DefaultMappingContext->MapKey(PauseMenuAction, EKeys::Gamepad_Special_Right);
	DefaultMappingContext->MapKey(LockOnAction, EKeys::Tab);
	DefaultMappingContext->MapKey(LockOnAction, EKeys::MiddleMouseButton);
	DefaultMappingContext->MapKey(LockOnAction, EKeys::Gamepad_RightShoulder);
	DefaultMappingContext->MapKey(LockOnPreviousTargetAction, EKeys::Z);
	DefaultMappingContext->MapKey(LockOnPreviousTargetAction, EKeys::MouseScrollDown);
	DefaultMappingContext->MapKey(LockOnPreviousTargetAction, EKeys::Gamepad_DPad_Left);
	DefaultMappingContext->MapKey(LockOnNextTargetAction, EKeys::C);
	DefaultMappingContext->MapKey(LockOnNextTargetAction, EKeys::MouseScrollUp);
	DefaultMappingContext->MapKey(LockOnNextTargetAction, EKeys::Gamepad_DPad_Right);
}

void AAetherPlayerController::Move(const FInputActionValue& Value)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	if (IsControlledCharacterMovementLocked())
	{
		if (AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(ControlledPawn))
		{
			AetherCharacter->ClearDesiredMovementDirection();
		}
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (MovementVector.IsNearlyZero())
	{
		return;
	}

	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector WorldMovementDirection = (ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X).GetSafeNormal();

	if (AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(ControlledPawn))
	{
		AetherCharacter->SetDesiredMovementDirection(WorldMovementDirection);
	}

	ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
	ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
}

void AAetherPlayerController::StopMove()
{
	if (AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn()))
	{
		AetherCharacter->ClearDesiredMovementDirection();
	}
}

void AAetherPlayerController::Look(const FInputActionValue& Value)
{
	if (IsCinematicBlockingGameplayInput())
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (LookAxisVector.IsNearlyZero())
	{
		return;
	}

	const UAetherSettingsSubsystem* Settings = GetGameInstance() ? GetGameInstance()->GetSubsystem<UAetherSettingsSubsystem>() : nullptr;
	const FAetherCustomSettings CustomSettings = Settings ? Settings->GetCurrentSettings().Custom : FAetherCustomSettings();
	const float PitchDirection = CustomSettings.bInvertCameraY ? -1.0f : 1.0f;
	AddYawInput(LookAxisVector.X * CustomSettings.CameraSensitivityX);
	AddPitchInput(LookAxisVector.Y * CustomSettings.CameraSensitivityY * PitchDirection);
}

void AAetherPlayerController::LightAttack()
{
	if (IsGameplayActionBlocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetCombatComponent())
	{
		return;
	}

	AetherCharacter->GetCombatComponent()->StartLightAttack();
}

void AAetherPlayerController::HeavyAttack()
{
	if (IsGameplayActionBlocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetCombatComponent())
	{
		return;
	}

	AetherCharacter->GetCombatComponent()->StartHeavyAttack();
}

void AAetherPlayerController::Execution()
{
	if (IsGameplayActionBlocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetCombatComponent())
	{
		return;
	}

	AetherCharacter->GetCombatComponent()->StartExecution();
}

void AAetherPlayerController::AetherSlash()
{
	if (IsGameplayActionBlocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetCombatComponent())
	{
		return;
	}

	AetherCharacter->GetCombatComponent()->StartAetherSlash();
}

void AAetherPlayerController::UseQuickItem()
{
	if (IsGameplayActionBlocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetInventoryComponent())
	{
		return;
	}

	AetherCharacter->GetInventoryComponent()->UsePrototypeHealingItem();
}

void AAetherPlayerController::Interact()
{
	if (IsControlledCharacterMovementLocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetInteractionComponent())
	{
		return;
	}

	AetherCharacter->GetInteractionComponent()->TryInteract();
}

void AAetherPlayerController::Dodge()
{
	if (IsGameplayActionBlocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetCombatComponent())
	{
		return;
	}

	AetherCharacter->GetCombatComponent()->StartDodge();
}

void AAetherPlayerController::StartGuard()
{
	if (IsGameplayActionBlocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetCombatComponent())
	{
		return;
	}

	AetherCharacter->GetCombatComponent()->StartGuard();
}

void AAetherPlayerController::StopGuard()
{
	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetCombatComponent())
	{
		return;
	}

	AetherCharacter->GetCombatComponent()->StopGuard();
}

void AAetherPlayerController::Parry()
{
	if (IsGameplayActionBlocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetCombatComponent())
	{
		return;
	}

	AetherCharacter->GetCombatComponent()->TryParry();
}

void AAetherPlayerController::DebugIncomingHit()
{
	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetCombatComponent())
	{
		return;
	}

	AetherCharacter->GetCombatComponent()->SimulateIncomingHit();
}

void AAetherPlayerController::DebugResetPlayer()
{
	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->ResetPrototypePlayerAtCheckpoint();
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (AetherCharacter && AetherCharacter->GetCombatComponent())
	{
		AetherCharacter->GetCombatComponent()->ResetPlayerPrototypeState();
	}
}

void AAetherPlayerController::DebugResetCombatRound()
{
	AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!GameMode)
	{
		return;
	}

	GameMode->ResetPrototypeCombatRound();
}

void AAetherPlayerController::DebugClearPrototypeCheckpointProgress()
{
	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->ClearPrototypeCheckpointProgress();
	}
}

void AAetherPlayerController::AdvanceDialogue()
{
	if (TrySkipActiveCinematic())
	{
		return;
	}

	if (AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->AdvancePrototypeDialogue();
	}
}

void AAetherPlayerController::TogglePauseMenu()
{
	if (TrySkipActiveCinematic() || IsCinematicBlockingGameplayInput())
	{
		return;
	}

	if (PauseMenuComponent)
	{
		PauseMenuComponent->TogglePauseMenu();
	}
}

void AAetherPlayerController::ToggleLockOn()
{
	if (IsGameplayActionBlocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetLockOnComponent())
	{
		return;
	}

	AetherCharacter->GetLockOnComponent()->ToggleLockOn();
}

void AAetherPlayerController::SwitchLockOnTargetLeft()
{
	if (IsGameplayActionBlocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetLockOnComponent())
	{
		return;
	}

	AetherCharacter->GetLockOnComponent()->SwitchTarget(-1.0f);
}

void AAetherPlayerController::SwitchLockOnTargetRight()
{
	if (IsGameplayActionBlocked())
	{
		return;
	}

	AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter || !AetherCharacter->GetLockOnComponent())
	{
		return;
	}

	AetherCharacter->GetLockOnComponent()->SwitchTarget(1.0f);
}

bool AAetherPlayerController::IsControlledCharacterDead() const
{
	const AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	const UAetherHealthComponent* HealthComponent = AetherCharacter ? AetherCharacter->GetHealthComponent() : nullptr;
	return HealthComponent && HealthComponent->IsDead();
}

bool AAetherPlayerController::IsControlledCharacterMovementLocked() const
{
	const AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(GetPawn());
	if (!AetherCharacter)
	{
		return false;
	}

	const UAetherHealthComponent* HealthComponent = AetherCharacter->GetHealthComponent();
	if (HealthComponent && HealthComponent->IsDead())
	{
		return true;
	}

	if (IsGameplayActionBlocked())
	{
		return true;
	}

	const UAetherCombatComponent* CombatComponent = AetherCharacter->GetCombatComponent();
	return CombatComponent && CombatComponent->ShouldBlockMovementInput();
}

bool AAetherPlayerController::IsPrototypeDialogueBlockingGameplayInput() const
{
	const AAetherGameModeBase* GameMode = Cast<AAetherGameModeBase>(UGameplayStatics::GetGameMode(this));
	return GameMode && GameMode->ShouldPrototypeDialogueBlockGameplayInput();
}

bool AAetherPlayerController::IsCinematicBlockingGameplayInput() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const UAetherCinematicDirectorSubsystem* CinematicDirector = GameInstance ? GameInstance->GetSubsystem<UAetherCinematicDirectorSubsystem>() : nullptr;
	return CinematicDirector && CinematicDirector->ShouldBlockGameplayInput();
}

bool AAetherPlayerController::IsGameplayActionBlocked() const
{
	return IsPrototypeDialogueBlockingGameplayInput() || IsCinematicBlockingGameplayInput();
}

bool AAetherPlayerController::TrySkipActiveCinematic()
{
	UGameInstance* GameInstance = GetGameInstance();
	UAetherCinematicDirectorSubsystem* CinematicDirector = GameInstance ? GameInstance->GetSubsystem<UAetherCinematicDirectorSubsystem>() : nullptr;
	return CinematicDirector && CinematicDirector->SkipActiveCinematic();
}
