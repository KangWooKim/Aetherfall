/** 게임 인스턴스 수명 동안 컷신 요청과 입력·카메라 잠금, 종료 이벤트를 조정한다. 실제 시퀀스 연출은 요청 이벤트를 받은 쪽에서 연결한다. */
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

/** 등록된 트리거 정의를 복사하고 요청별 이벤트 라벨을 덮어쓴 뒤 컷신을 요청한다. */
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

/** 비활성 정의나 중복 재생 요청을 거절한다. 잠금을 적용한 후 시작·연출 요청 이벤트를 전달하고 대체 종료 타이머를 등록한다. */
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

/** 건너뛰기가 허용된 활성 컷신만 종료하며, 건너뛰기 이벤트와 공통 종료 경로를 함께 사용한다. */
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

/** 현재 월드의 컨트롤러를 잠그고 이동을 중단한다. 해제할 컨트롤러는 약한 참조로 보관한다. */
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

/** 이 서브시스템이 기록한 유효한 컨트롤러의 컷신 모드를 해제한다. */
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

/** 타이머와 잠금을 정리하고 활성 상태를 초기화한 뒤 종료 스냅샷을 알린다. 종료 이벤트에서 다음 요청을 시작할 수 있도록 상태를 먼저 비운다. */
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

/** 시퀀스 유무와 자동 종료 설정을 평가해 종료 타이머를 예약한다. 시퀀스를 직접 재생하는 함수는 아니다. */
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

/** 소속 월드를 우선 사용하고, 없으면 엔진 컨텍스트에서 실행 또는 PIE 월드를 찾는다. */
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
