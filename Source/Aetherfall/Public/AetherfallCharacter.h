#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AetherfallCharacter.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UAetherCombatComponent;
class UAetherHealthComponent;
class UAetherInventoryComponent;
class UAetherInteractionComponent;
class UAetherLockOnComponent;

/** 플레이어의 이동·카메라·무기 표현을 구성하고 전투·체력·인벤토리·잠금·상호작용 컴포넌트를 소유한다. */
UCLASS()
class AETHERFALL_API AAetherfallCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	/** 기본 이동과 카메라, 기능별 컴포넌트 및 에셋이 없을 때 사용할 임시 메시를 구성한다. */
	AAetherfallCharacter();

	/** 타격 카메라 오프셋을 감쇠시키고 효과가 끝나면 원래 위치를 복원한 뒤 틱을 끈다. */
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void SetDesiredMovementDirection(const FVector& WorldDirection);
	void ClearDesiredMovementDirection();
	/** 입력한 수평 이동 방향을 우선하고 입력이 없으면 캐릭터의 전방을 반환한다. */
	FVector GetDesiredMovementDirection() const;
	/** 기존 효과보다 약해지지 않게 강도를 선택하고 카메라 효과 시간 동안만 틱을 활성화한다. */
	void PlayCameraImpactFeedback(float Strength, float Duration);

	/** 정적·스켈레탈 무기를 지정 소켓에 연결하고 무기 메시 자체의 충돌은 비활성화한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Weapon")
	void RefreshWeaponAttachment();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Animation")
	float GetGroundSpeed() const;

	/** 수평 속도를 캐릭터의 전방·우측 축에 투영해 애니메이션용 이동 각도를 구한다. */
	UFUNCTION(BlueprintPure, Category = "Aetherfall|Animation")
	float GetMovementDirectionAngle() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Animation")
	bool IsMovingOnGround() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Animation")
	bool IsFallingForAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Animation")
	bool IsLockedOnForAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Animation")
	bool IsGuardingForAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Animation")
	bool IsAttackingForAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Animation")
	bool IsDodgingForAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Animation")
	bool IsCombatMovementBlockedForAnimation() const;

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Combat")
	FORCEINLINE UAetherCombatComponent* GetCombatComponent() const { return CombatComponent; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Health")
	FORCEINLINE UAetherHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Inventory")
	FORCEINLINE UAetherInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|LockOn")
	FORCEINLINE UAetherLockOnComponent* GetLockOnComponent() const { return LockOnComponent; }

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Interaction")
	FORCEINLINE UAetherInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

protected:
	virtual void BeginPlay() override;

private:
	/** 실제 스켈레탈 메시가 배정되었는지와 자동 숨김 설정에 따라 임시 몸체·무기를 표시한다. */
	void RefreshPrototypeVisualMode();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PrototypeBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prototype", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> PrototypeWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prototype", meta = (AllowPrivateAccess = "true"))
	bool bAutoHidePrototypeVisualsWhenSkeletalMeshAssigned = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> WeaponStaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> WeaponSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Weapon", meta = (AllowPrivateAccess = "true"))
	FName WeaponAttachSocketName = FName(TEXT("hand_r"));

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherLockOnComponent> LockOnComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAetherInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	FVector DesiredMovementDirection = FVector::ZeroVector;

	FVector BaseCameraSocketOffset = FVector::ZeroVector;
	FVector CameraImpactOffset = FVector::ZeroVector;
	float CameraImpactElapsed = 0.0f;
	float CameraImpactDuration = 0.0f;
	float CameraImpactStrength = 0.0f;
};
