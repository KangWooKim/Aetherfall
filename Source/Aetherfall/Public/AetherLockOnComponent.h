#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AetherLockOnComponent.generated.h"

class AAetherEnemyBase;
class AAetherfallCharacter;

/** 락온 대상 선택과 좌우 전환, 대상 상실 처리 및 캐릭터·카메라 회전을 담당한다. */
UCLASS(ClassGroup = (Aetherfall), meta = (BlueprintSpawnableComponent))
class AETHERFALL_API UAetherLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAetherLockOnComponent();

	/** 현재 참조가 유효할 때 거리·생존을 재검사하고 설정에 따라 대체 대상 탐색 또는 해제를 수행한다. */
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
	/** 유효한 적 중 시야 뒤쪽을 제외하고 평면 거리와 시선 정렬 점수로 초기 락온 대상을 고른다. */
	AAetherEnemyBase* FindBestTarget(const AAetherEnemyBase* ExcludedTarget = nullptr) const;
	/** 시선 전방 벡터와의 외적으로 요청한 좌우 후보를 찾고, 해당 방향에 없으면 거리·시선 점수가 좋은 다른 적을 사용한다. */
	AAetherEnemyBase* FindSwitchTarget(float Direction) const;
	/** 대상 생존과 평면 거리만 확인한다. 시야 가림이나 벽 충돌은 이 함수에서 검사하지 않는다. */
	bool IsValidLockOnTarget(const AAetherEnemyBase* Candidate, float MaxRange) const;
	FVector GetTargetFocusLocation(const AAetherEnemyBase* Target) const;
	/** 캐릭터 방향을 대상 쪽으로 보간하고, 카메라 회전 사용 시 기존 피치를 유지한 채 수평 시선만 맞춘다. */
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
