#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherLockOnComponent.generated.h"

class AAetherEnemyBase;
class AAetherfallCharacter;

UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherLockOnComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|LockOn")
	void ToggleLockOn();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|LockOn")
	void ClearLockOn();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|LockOn")
	void SwitchTarget(float Direction);

	UFUNCTION(BlueprintPure, Category = "Aetherfall|LockOn")
	bool IsLockedOn() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|LockOn")
	AAetherEnemyBase* GetLockedTarget() const;

protected:
	virtual void BeginPlay() override;

private:
	void SetLockedTarget(AAetherEnemyBase* NewTarget, const FString& Message);
	AAetherEnemyBase* FindBestTarget(const AAetherEnemyBase* ExcludedTarget = nullptr) const;
	AAetherEnemyBase* FindSwitchTarget(float Direction) const;
	bool IsValidLockOnTarget(const AAetherEnemyBase* Candidate, float MaxRange) const;
	FVector GetTargetFocusLocation(const AAetherEnemyBase* Target) const;
	void UpdateRotation(float DeltaTime);
	void ShowLockOnDebugMessage(const FString& Message, const FColor& Color) const;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn", meta = (ClampMin = "0.0"))
	float LockOnRange = 1400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn", meta = (ClampMin = "0.0"))
	float BreakRange = 1800.0f;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn", meta = (ClampMin = "0.0"))
	float CharacterRotationInterpSpeed = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn", meta = (ClampMin = "0.0"))
	float CameraRotationInterpSpeed = 7.0f;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	bool bRotateCameraToTarget = true;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	bool bAutoSwitchOnTargetLost = true;

	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Debug")
	bool bShowLockOnScreenDebugMessages = false;

	TWeakObjectPtr<AAetherfallCharacter> OwnerCharacter;
	TWeakObjectPtr<AAetherEnemyBase> LockedTarget;
};
