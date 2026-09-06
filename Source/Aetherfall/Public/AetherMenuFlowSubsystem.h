#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AetherMenuFlowSubsystem.generated.h"

/** 메뉴 요청 대기·맵 전환 진행·실패 상태를 구분한다. */
UENUM(BlueprintType)
enum class EAetherMenuFlowState : uint8
{
	Idle,
	Transitioning,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAetherMenuFlowChangedSignature, EAetherMenuFlowState, State, FText, Message);

/** 저장 가능 여부와 중복 요청을 확인하며 메뉴·게임 맵 사이의 전환 상태와 로딩 요청을 관리한다. */
UCLASS()
class AETHERFALL_API UAetherMenuFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Menu")
	EAetherMenuFlowState GetFlowState() const { return FlowState; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Menu")
	bool IsTransitionInProgress() const { return FlowState == EAetherMenuFlowState::Transitioning; }

	/** 슬롯 존재만이 아니라 스키마와 체크포인트를 포함한 로드 가능 여부를 확인한 후 게임 맵을 연다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Menu")
	bool ContinueGame();

	/** 기존 진행이 있으면 덮어쓰기 인자를 요구하고 삭제 성공 후 게임 맵 전환을 요청한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Menu")
	bool StartNewGame(bool bOverwriteExistingProgress);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Menu")
	bool LoadGame();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Menu")
	bool ReturnToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Menu")
	void QuitGame(APlayerController* PlayerController);

	UPROPERTY(BlueprintAssignable, Category = "Aetherfall|Menu")
	FAetherMenuFlowChangedSignature OnMenuFlowChanged;

private:
	/** 중복 전환을 거부하고 로딩 화면과 전환 상태를 설정한 뒤 엔진에 레벨 열기를 요청한다. */
	bool OpenMap(FName MapName, const FText& LoadingMessage);
	void SetFlowState(EAetherMenuFlowState NewState, const FText& Message);
	/** 맵 로드 완료 콜백에서 진행 중 전환 상태를 대기 상태로 되돌린다. */
	void HandlePostLoadMap(UWorld* LoadedWorld);

	EAetherMenuFlowState FlowState = EAetherMenuFlowState::Idle;
	FName GameplayMapName = FName(TEXT("/Game/Aetherfall/Maps/M_VerticalSlice"));
	FName MainMenuMapName = FName(TEXT("/Game/Aetherfall/Maps/M_MainMenu"));
};
