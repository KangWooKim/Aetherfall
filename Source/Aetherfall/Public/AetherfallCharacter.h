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

UCLASS()
class AETHERFALL_API AAetherfallCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAetherfallCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void SetDesiredMovementDirection(const FVector& WorldDirection);
	void ClearDesiredMovementDirection();
	FVector GetDesiredMovementDirection() const;
	void PlayCameraImpactFeedback(float Strength, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Weapon")
	void RefreshWeaponAttachment();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Animation")
	float GetGroundSpeed() const;

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
