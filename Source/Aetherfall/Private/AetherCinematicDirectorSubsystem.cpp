#include "AetherCinematicDirectorSubsystem.h"

#include "AetherfallCharacter.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

namespace
{
	FAetherCinematicDefinition MakeDefaultCinematic(
		EAetherCinematicTrigger Trigger,
		FName EventLabel,
		const FText& DisplayName,
		float FallbackDuration)
	{
		FAetherCinematicDefinition Definition;
		Definition.Trigger = Trigger;
		Definition.EventLabel = EventLabel;
		Definition.DisplayName = DisplayName;
		Definition.FallbackDuration = FallbackDuration;
		return Definition;
	}
}

UAetherCinematicDirectorSubsystem::UAetherCinematicDirectorSubsystem()
{
	RegisterCinematicDefinition(MakeDefaultCinematic(
		EAetherCinematicTrigger::GameIntro,
		FName(TEXT("Story_OpeningWake")),
		NSLOCTEXT("AetherCinematic", "GameIntro", "Opening Cinematic"),
		2.0f));
	RegisterCinematicDefinition(MakeDefaultCinematic(
		EAetherCinematicTrigger::BossIntro,
		FName(TEXT("CathedralBoss")),
		NSLOCTEXT("AetherCinematic", "BossIntro", "Aurel Entrance"),
		2.5f));
	RegisterCinematicDefinition(MakeDefaultCinematic(
		EAetherCinematicTrigger::BossDefeated,
		FName(TEXT("CathedralBoss")),
		NSLOCTEXT("AetherCinematic", "BossDefeated", "Aurel Defeated"),
		2.25f));
}

void UAetherCinematicDirectorSubsystem::Deinitialize()
{
	FinishActiveCinematicInternal(false);
	CinematicDefinitions.Reset();
	Super::Deinitialize();
}

void UAetherCinematicDirectorSubsystem::RegisterCinematicDefinition(const FAetherCinematicDefinition& Definition)
{
	CinematicDefinitions.Add(Definition.Trigger, Definition);
}

bool UAetherCinematicDirectorSubsystem::RequestCinematicByTrigger(EAetherCinematicTrigger Trigger, FName EventLabel)
{
	const FAetherCinematicDefinition* Definition = CinematicDefinitions.Find(Trigger);
	if (!Definition)
	{
		return false;
	}

	FAetherCinematicDefinition RuntimeDefinition = *Definition;
	if (!EventLabel.IsNone())
	{
		RuntimeDefinition.EventLabel = EventLabel;
	}
	return RequestCinematic(RuntimeDefinition);
}

bool UAetherCinematicDirectorSubsystem::RequestCinematic(const FAetherCinematicDefinition& Definition)
{
	if (ActiveState.bActive || !Definition.bEnabled)
	{
		return false;
	}

	ActiveState = FAetherCinematicRuntimeState();
	ActiveState.bActive = true;
	ActiveState.Definition = Definition;
	ActiveState.bSkipped = false;
	ActiveCinematicStartTime = FPlatformTime::Seconds();

	ApplyCinematicLocks(Definition);
	OnCinematicStarted.Broadcast(GetActiveCinematicState());
	OnCinematicPresentationRequested.Broadcast(GetActiveCinematicState());
	ScheduleFallbackFinish(Definition);
	return true;
}

bool UAetherCinematicDirectorSubsystem::SkipActiveCinematic()
{
	if (!CanSkipActiveCinematic())
	{
		return false;
	}

	OnCinematicSkipped.Broadcast(GetActiveCinematicState());
	FinishActiveCinematicInternal(true);
	return true;
}

void UAetherCinematicDirectorSubsystem::FinishActiveCinematic()
{
	FinishActiveCinematicInternal(false);
}

FAetherCinematicRuntimeState UAetherCinematicDirectorSubsystem::GetActiveCinematicState() const
{
	FAetherCinematicRuntimeState RuntimeState = ActiveState;
	if (RuntimeState.bActive)
	{
		RuntimeState.ElapsedSeconds = static_cast<float>(FPlatformTime::Seconds() - ActiveCinematicStartTime);
	}
	return RuntimeState;
}

void UAetherCinematicDirectorSubsystem::ApplyCinematicLocks(const FAetherCinematicDefinition& Definition)
{
	UWorld* World = ResolveRuntimeWorld();
	if (!World)
	{
		return;
	}

	LockedControllers.Reset();
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (!PlayerController)
		{
			continue;
		}

		PlayerController->SetCinematicMode(
			true,
			false,
			Definition.bHideHud,
			Definition.bLockPlayerInput,
			Definition.bLockCameraControl);
		if (APawn* Pawn = PlayerController->GetPawn())
		{
			if (AAetherfallCharacter* AetherCharacter = Cast<AAetherfallCharacter>(Pawn))
			{
				AetherCharacter->ClearDesiredMovementDirection();
			}
			if (UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent()))
			{
				MovementComponent->StopMovementImmediately();
			}
		}

		LockedControllers.Add(PlayerController);
	}
}

void UAetherCinematicDirectorSubsystem::RestoreCinematicLocks(const FAetherCinematicDefinition& Definition)
{
	for (TWeakObjectPtr<APlayerController>& ControllerPtr : LockedControllers)
	{
		APlayerController* PlayerController = ControllerPtr.Get();
		if (!PlayerController)
		{
			continue;
		}

		PlayerController->SetCinematicMode(
			false,
			false,
			Definition.bHideHud,
			Definition.bLockPlayerInput,
			Definition.bLockCameraControl);
	}
	LockedControllers.Reset();
}

void UAetherCinematicDirectorSubsystem::FinishActiveCinematicInternal(bool bSkipped)
{
	if (!ActiveState.bActive)
	{
		return;
	}

	if (UWorld* World = ResolveRuntimeWorld())
	{
		World->GetTimerManager().ClearTimer(FallbackFinishTimerHandle);
	}

	ActiveState.ElapsedSeconds = static_cast<float>(FPlatformTime::Seconds() - ActiveCinematicStartTime);
	ActiveState.bSkipped = bSkipped;
	const FAetherCinematicRuntimeState FinishedState = ActiveState;
	RestoreCinematicLocks(ActiveState.Definition);

	ActiveState = FAetherCinematicRuntimeState();
	ActiveCinematicStartTime = 0.0;

	OnCinematicFinished.Broadcast(FinishedState);
	OnCinematicFinishedNative.Broadcast(FinishedState);
}

void UAetherCinematicDirectorSubsystem::ScheduleFallbackFinish(const FAetherCinematicDefinition& Definition)
{
	UWorld* World = ResolveRuntimeWorld();
	if (!World)
	{
		return;
	}

	const bool bNeedsAutoFinish = Definition.SequenceAsset.IsNull() ? Definition.bAutoFinishWhenNoSequenceAsset : Definition.FallbackDuration > 0.0f;
	if (!bNeedsAutoFinish || Definition.FallbackDuration <= 0.0f)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		FallbackFinishTimerHandle,
		this,
		&UAetherCinematicDirectorSubsystem::FinishActiveCinematic,
		Definition.FallbackDuration,
		false);
}

UWorld* UAetherCinematicDirectorSubsystem::ResolveRuntimeWorld() const
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
