#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AetherPlayerController.generated.h"

class UAetherPauseMenuComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/** Enhanced Input을 구성하고 플레이어 입력을 이동·전투·상호작용·대화·메뉴 컴포넌트에 전달한다. */
UCLASS()
class AETHERFALL_API AAetherPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAetherPlayerController();

protected:
	virtual void BeginPlay() override;
	/** Enhanced Input 컴포넌트가 있을 때 입력 시작·완료 사건을 행동별 처리 함수에 연결한다. */
	virtual void SetupInputComponent() override;

private:
	/** 기본 키보드·마우스·게임패드 매핑과 축 보정자를 생성한다. 디버그 초기화 입력도 이 경로에 포함된다. */
	void BuildDefaultInputMapping();
	/** 입력 잠금을 확인하고 카메라 수평 시선 기준의 이동 방향을 계산해 폰과 회피용 방향 상태에 전달한다. */
	void Move(const FInputActionValue& Value);
	void StopMove();
	/** 연출 입력 차단을 확인한 뒤 사용자 카메라 감도와 수직 반전 설정을 적용한다. */
	void Look(const FInputActionValue& Value);
	void LightAttack();
	void HeavyAttack();
	void Execution();
	void AetherSlash();
	void UseQuickItem();
	void Interact();
	void Dodge();
	void StartGuard();
	void StopGuard();
	void Parry();
	void DebugIncomingHit();
	void DebugResetPlayer();
	void DebugResetCombatRound();
	void DebugClearPrototypeCheckpointProgress();
	/** 건너뛸 수 있는 활성 연출을 먼저 처리하고, 그렇지 않으면 대화 진행을 요청한다. */
	void AdvanceDialogue();
	/** 활성 연출의 건너뛰기와 입력 차단을 우선 처리한 뒤 일시 정지 메뉴를 전환한다. */
	void TogglePauseMenu();
	void ToggleLockOn();
	void SwitchLockOnTargetLeft();
	void SwitchLockOnTargetRight();
	bool IsControlledCharacterDead() const;
	/** 사망·대화·연출 잠금과 전투 컴포넌트의 이동 차단을 합쳐 이동 입력 허용 여부를 판단한다. */
	bool IsControlledCharacterMovementLocked() const;
	bool IsPrototypeDialogueBlockingGameplayInput() const;
	bool IsCinematicBlockingGameplayInput() const;
	/** 대화 및 연출이 요청한 게임 행동 차단만 확인한다. 생존과 행동별 자원 조건은 해당 처리 계층에서 검사한다. */
	bool IsGameplayActionBlocked() const;
	bool TrySkipActiveCinematic();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LightAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> HeavyAttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ExecutionAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AetherSlashAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> UseQuickItemAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> GuardAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ParryAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DebugIncomingHitAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DebugResetPlayerAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DebugResetRoundAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DebugClearCheckpointProgressAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> DialogueAdvanceAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PauseMenuAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherPauseMenuComponent> PauseMenuComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LockOnAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LockOnPreviousTargetAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LockOnNextTargetAction;
};
