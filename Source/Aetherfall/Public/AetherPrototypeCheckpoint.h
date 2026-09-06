#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeCheckpoint.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

/** 플레이어 진입을 체크포인트 활성화 요청으로 바꾸고 일회성 작동과 진행 초기화 후 재진입 조건을 관리한다. */
UCLASS()
class AETHERFALL_API AAetherPrototypeCheckpoint : public AActor
{
	GENERATED_BODY()

public:
	AAetherPrototypeCheckpoint();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Checkpoint")
	void ResetCheckpoint();

	/** 진행 삭제 후 이미 영역 안에 있는 플레이어는 한 번 나갔다 들어와야 다시 활성화되도록 상태를 설정한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Checkpoint")
	void ResetCheckpointAfterProgressClear();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Checkpoint")
	bool HasCheckpointActivated() const { return bHasActivated; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Checkpoint")
	void OnCheckpointActivated();

private:
	/** 플레이어와 재활성화 조건을 확인하고 게임 모드에 진행 등급 판정을 위임한 뒤 트리거와 연출 상태를 갱신한다. */
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleTriggerEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void SetTriggerActive(bool bNewActive);
	void ShowCheckpointMessage(const FString& Message, const FColor& Color) const;

	UPROPERTY(VisibleAnywhere, Category = "Aetherfall|Checkpoint")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Checkpoint", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aetherfall|Checkpoint", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Checkpoint")
	FName CheckpointLabel = TEXT("PrototypeCheckpoint");

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Checkpoint", meta = (ClampMin = "0"))
	int32 CheckpointProgressRank = 0;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Checkpoint")
	bool bActivateOnce = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Checkpoint")
	bool bDisableAfterActivation = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Checkpoint|Debug")
	bool bShowCheckpointDebugMessages = true;

	UPROPERTY(EditAnywhere, Category = "Aetherfall|Checkpoint|Debug")
	bool bRouteCheckpointMessagesToHudOnly = true;

	bool bHasActivated = false;
	bool bRequirePlayerExitBeforeReactivation = false;
};
