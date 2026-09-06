#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherPauseMenuComponent.generated.h"

class AAetherPlayerController;
class UAetherMainMenuWidget;
#if !UE_BUILD_SHIPPING
struct FAetherPauseMenuRuntimeValidationAccess;
#endif

/** 플레이어 컨트롤러의 일시 정지 메뉴 수명과 입력 전환을 관리하고 공유 메뉴 위젯을 재사용한다. */
UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherPauseMenuComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherPauseMenuComponent();

	/** 공유 위젯을 일시 정지 문맥으로 표시하고 월드 정지에 성공하면 UI 전용 입력과 포커스를 설정한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Pause Menu")
	bool OpenPauseMenu();

	/** 이벤트 연결과 화면 표시를 제거하되 위젯 참조를 남겨 재사용하고 게임 입력을 복원한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Pause Menu")
	void ClosePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Pause Menu")
	void TogglePauseMenu();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Pause Menu")
	bool IsPauseMenuOpen() const { return bPauseMenuOpen && PauseMenuWidget != nullptr; }

protected:
	/** 위젯과 델리게이트를 최종 정리하고 맵 전환 중이 아닌 경우 게임 입력도 복원한다. */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
#if !UE_BUILD_SHIPPING
	friend struct FAetherPauseMenuRuntimeValidationAccess;
#endif
	/** 로컬 컨트롤러, 생존 플레이어, 게임 모드와 전환·재시작 상태를 확인해 메뉴 열기 가능 여부를 판단한다. */
	bool CanOpenPauseMenu() const;
	/** 보관된 위젯이 있으면 재사용하고, 없으면 Blueprint 클래스를 우선해 새 인스턴스를 만든다. */
	UAetherMainMenuWidget* GetOrCreatePauseMenuWidget(AAetherPlayerController* Controller);
	void RestoreGameplayInput();

	UFUNCTION()
	void HandleResumeRequested();

	/** 중복 복귀 요청을 막고 맵 이동 전에 정지를 해제한다. 요청이 즉시 거절되면 일시 정지를 복원한다. */
	UFUNCTION()
	void HandleReturnToMainMenuRequested();

	UPROPERTY(Transient)
	TObjectPtr<UAetherMainMenuWidget> PauseMenuWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Pause Menu")
	TSoftClassPtr<UAetherMainMenuWidget> PauseMenuWidgetBlueprintClass;

	bool bPauseMenuOpen = false;
	bool bTransitionRequested = false;
};
