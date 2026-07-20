#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AetherSaveSubsystem.generated.h"

class UAetherPrototypeSaveGame;

#if !UE_BUILD_SHIPPING
struct FAetherSaveSubsystemRuntimeValidationAccess;
struct FAetherPauseMenuRuntimeValidationAccess;
#endif

USTRUCT(BlueprintType)
struct AETHERFALL_API FAetherSaveSlotSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Save")
	bool bExists = false;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Save")
	FString SlotName;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Save")
	int32 UserIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Save")
	bool bLoadable = false;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Save")
	FName CheckpointLabel = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Save")
	int32 CheckpointProgressRank = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Save")
	int32 SaveSchemaVersion = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Save")
	FDateTime SavedAtUtc;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Save")
	FName SavedMapAsset = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Aetherfall|Save")
	FText StatusText;
};

UCLASS()
class AETHERFALL_API UAetherSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Save")
	bool SavePrototypeCheckpointSnapshot(UAetherPrototypeSaveGame* SaveGameObject) const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Save")
	UAetherPrototypeSaveGame* LoadPrototypeCheckpointSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Save")
	bool HasPrototypeCheckpointSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Save")
	bool ClearPrototypeCheckpointSnapshot() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Save")
	FAetherSaveSlotSummary GetPrototypeCheckpointSummary() const;

	UFUNCTION(BlueprintPure, Category = "Aetherfall|Save")
	TArray<FAetherSaveSlotSummary> GetSaveSlotSummaries() const;

private:
#if !UE_BUILD_SHIPPING
	friend struct FAetherSaveSubsystemRuntimeValidationAccess;
	friend struct FAetherPauseMenuRuntimeValidationAccess;
#endif

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Save")
	FString PrototypeCheckpointSlotName = TEXT("PrototypeCheckpoint");

	UPROPERTY(EditDefaultsOnly, Category = "Aetherfall|Save")
	int32 PrototypeCheckpointUserIndex = 0;
};
