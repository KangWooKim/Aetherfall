#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AetherPrototypeCheckpoint.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class AETHERFALL_API AAetherPrototypeCheckpoint : public AActor
{
	GENERATED_BODY()

public:
	AAetherPrototypeCheckpoint();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Checkpoint")
	void ResetCheckpoint();

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Checkpoint")
	void ResetCheckpointAfterProgressClear();

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Checkpoint")
	bool HasCheckpointActivated() const { return bHasActivated; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Aetherfall|Checkpoint")
	void OnCheckpointActivated();

private:
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
