#pragma once

#include "CoreMinimal.h"
#include "AetherCinematicTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TimerManager.h"
#include "AetherCinematicDirectorSubsystem.generated.h"

class APlayerController;
class UWorld;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAetherCinematicChangedSignature, const FAetherCinematicRuntimeState&, RuntimeState);
DECLARE_MULTICAST_DELEGATE_OneParam(FAetherCinematicNativeChangedSignature, const FAetherCinematicRuntimeState&);

/** 게임 인스턴스 수명 동안 컷신 요청과 입력·카메라 잠금, 종료 이벤트를 조정한다. 실제 시퀀스 연출은 요청 이벤트를 받은 쪽에서 연결한다. */
UCLASS()
class AETHERFALL_API UAetherCinematicDirectorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAetherCinematicDirectorSubsystem();

	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Cinematic")
	void RegisterCinematicDefinition(const FAetherCinematicDefinition& Definition);

	/** 등록된 트리거 정의를 복사하고 요청별 이벤트 라벨을 덮어쓴 뒤 컷신을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Cinematic")
	bool RequestCinematicByTrigger(EAetherCinematicTrigger Trigger, FName EventLabel);

	/** 비활성 정의나 중복 재생 요청을 거절한다. 잠금을 적용한 후 시작·연출 요청 이벤트를 전달하고 대체 종료 타이머를 등록한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Cinematic")
	bool RequestCinematic(const FAetherCinematicDefinition& Definition);

	/** 건너뛰기가 허용된 활성 컷신만 종료하며, 건너뛰기 이벤트와 공통 종료 경로를 함께 사용한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Cinematic")
	bool SkipActiveCinematic();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Cinematic")
	void FinishActiveCinematic();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	bool IsCinematicActive() const { return ActiveState.bActive; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	bool CanSkipActiveCinematic() const { return ActiveState.bActive && ActiveState.Definition.bSkippable; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	bool ShouldBlockGameplayInput() const { return ActiveState.bActive && ActiveState.Definition.bLockPlayerInput; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	bool ShouldBlockCameraControl() const { return ActiveState.bActive && ActiveState.Definition.bLockCameraControl; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	bool ShouldHideHud() const { return ActiveState.bActive && ActiveState.Definition.bHideHud; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Cinematic")
	FAetherCinematicRuntimeState GetActiveCinematicState() const;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Cinematic")
	FAetherCinematicChangedSignature OnCinematicStarted;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Cinematic")
	FAetherCinematicChangedSignature OnCinematicPresentationRequested;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Cinematic")
	FAetherCinematicChangedSignature OnCinematicSkipped;

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Cinematic")
	FAetherCinematicChangedSignature OnCinematicFinished;

	FAetherCinematicNativeChangedSignature OnCinematicFinishedNative;

private:
	/** 현재 월드의 컨트롤러를 잠그고 이동을 중단한다. 해제할 컨트롤러는 약한 참조로 보관한다. */
	void ApplyCinematicLocks(const FAetherCinematicDefinition& Definition);
	/** 이 서브시스템이 기록한 유효한 컨트롤러의 컷신 모드를 해제한다. */
	void RestoreCinematicLocks(const FAetherCinematicDefinition& Definition);
	/** 타이머와 잠금을 정리하고 활성 상태를 초기화한 뒤 종료 스냅샷을 알린다. 종료 이벤트에서 다음 요청을 시작할 수 있도록 상태를 먼저 비운다. */
	void FinishActiveCinematicInternal(bool bSkipped);
	/** 시퀀스 유무와 자동 종료 설정을 평가해 종료 타이머를 예약한다. 시퀀스를 직접 재생하는 함수는 아니다. */
	void ScheduleFallbackFinish(const FAetherCinematicDefinition& Definition);
	/** 소속 월드를 우선 사용하고, 없으면 엔진 컨텍스트에서 실행 또는 PIE 월드를 찾는다. */
	UWorld* ResolveRuntimeWorld() const;

	TMap<EAetherCinematicTrigger, FAetherCinematicDefinition> CinematicDefinitions;

	FAetherCinematicRuntimeState ActiveState;

	TArray<TWeakObjectPtr<APlayerController>> LockedControllers;

	FTimerHandle FallbackFinishTimerHandle;
	double ActiveCinematicStartTime = 0.0;
};
