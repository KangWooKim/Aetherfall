/** 로딩 표시 요청과 연출 활성화·입력 차단·HUD 숨김·종료의 실행 상태를 확인하는 비배포용 검증 명령이다. */
#include "AetherCinematicDirectorSubsystem.h"
#include "AetherLoadingScreenSubsystem.h"

#if !UE_BUILD_SHIPPING

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogAetherPresentationValidation, Log, All);

namespace
{
	UWorld* FindRuntimeWorld()
	{
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

	void RecordCheck(bool bCondition, const TCHAR* CheckName, bool& bFailed)
	{
		bFailed |= !bCondition;
		UE_LOG(
			LogAetherPresentationValidation,
			Display,
			TEXT("[AetherPresentationValidation] %s: %s"),
			bCondition ? TEXT("PASS") : TEXT("FAIL"),
			CheckName);
	}

	/** 실제 월드의 서브시스템을 사용해 로딩 및 자산 없는 사용자 정의 연출의 상태 계약을 검사한다. */
	void ValidatePresentationRuntime()
	{
		bool bFailed = false;
		UWorld* World = FindRuntimeWorld();
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		UAetherLoadingScreenSubsystem* LoadingScreen = GameInstance ? GameInstance->GetSubsystem<UAetherLoadingScreenSubsystem>() : nullptr;
		UAetherCinematicDirectorSubsystem* CinematicDirector = GameInstance ? GameInstance->GetSubsystem<UAetherCinematicDirectorSubsystem>() : nullptr;

		RecordCheck(World != nullptr, TEXT("runtime world found"), bFailed);
		RecordCheck(LoadingScreen != nullptr, TEXT("loading screen subsystem available"), bFailed);
		RecordCheck(CinematicDirector != nullptr, TEXT("cinematic director subsystem available"), bFailed);
		if (!LoadingScreen || !CinematicDirector)
		{
			UE_LOG(
				LogAetherPresentationValidation,
				Display,
				TEXT("[AetherPresentationValidation] RESULT: %s"),
				bFailed ? TEXT("FAIL") : TEXT("PASS"));
			return;
		}

		LoadingScreen->BeginLoadingScreen(NSLOCTEXT("AetherPresentationValidation", "Loading", "Validation loading screen"), true);
		RecordCheck(LoadingScreen->IsLoadingScreenVisible(), TEXT("loading screen can be shown"), bFailed);
		LoadingScreen->NotifyMapLoadCompleted();
		LoadingScreen->NotifyGameplayWorldReady();
		LoadingScreen->RequestHideLoadingScreen();

		FAetherCinematicDefinition ValidationCinematic;
		ValidationCinematic.Trigger = EAetherCinematicTrigger::Custom;
		ValidationCinematic.EventLabel = FName(TEXT("Validation"));
		ValidationCinematic.DisplayName = NSLOCTEXT("AetherPresentationValidation", "Cinematic", "Validation cinematic");
		ValidationCinematic.FallbackDuration = 0.0f;
		ValidationCinematic.bAutoFinishWhenNoSequenceAsset = false;
		RecordCheck(CinematicDirector->RequestCinematic(ValidationCinematic), TEXT("custom cinematic request accepted"), bFailed);
		RecordCheck(CinematicDirector->IsCinematicActive(), TEXT("cinematic becomes active"), bFailed);
		RecordCheck(CinematicDirector->ShouldBlockGameplayInput(), TEXT("cinematic blocks gameplay input"), bFailed);
		RecordCheck(CinematicDirector->ShouldHideHud(), TEXT("cinematic hides HUD"), bFailed);
		CinematicDirector->FinishActiveCinematic();
		RecordCheck(!CinematicDirector->IsCinematicActive(), TEXT("cinematic finishes cleanly"), bFailed);

		UE_LOG(
			LogAetherPresentationValidation,
			Display,
			TEXT("[AetherPresentationValidation] RESULT: %s"),
			bFailed ? TEXT("FAIL") : TEXT("PASS"));
	}

	FAutoConsoleCommand ValidatePresentationRuntimeCommand(
		TEXT("Aether.Presentation.ValidateRuntime"),
		TEXT("Validates Aetherfall loading screen and cinematic director runtime contracts."),
		FConsoleCommandDelegate::CreateStatic(&ValidatePresentationRuntime));
}

#endif
