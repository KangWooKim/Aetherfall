#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AetherSaveSubsystem.generated.h"

class UAetherPrototypeSaveGame;

#if !UE_BUILD_SHIPPING
struct FAetherSaveSubsystemRuntimeValidationAccess;
struct FAetherPauseMenuRuntimeValidationAccess;
#endif

/** 슬롯 존재와 실제 불러오기 가능 여부를 구분하여 메뉴에 표시한다. */
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

/** 체크포인트 저장 슬롯의 입출력과 메뉴용 요약을 제공하는 게임 인스턴스 서브시스템이다. */
UCLASS()
class AETHERFALL_API UAetherSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 저장 시각과 현재 맵 이름을 갱신한 뒤 지정된 슬롯에 스냅샷을 기록한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Save")
	bool SavePrototypeCheckpointSnapshot(UAetherPrototypeSaveGame* SaveGameObject) const;

	/** 슬롯을 읽어 예상 저장 클래스인지 확인한다. 버전 호환성 판단은 별도 정책에서 수행한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Save")
	UAetherPrototypeSaveGame* LoadPrototypeCheckpointSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Save")
	bool HasPrototypeCheckpointSnapshot() const;

	/** 저장 슬롯이 없으면 이미 삭제된 상태로 취급하고 있으면 해당 슬롯만 삭제한다. */
	UFUNCTION(BlueprintCallable, Category = "Aetherfall|Save")
	bool ClearPrototypeCheckpointSnapshot() const;

	/** 슬롯 존재, 읽기 성공, 버전 호환성과 체크포인트 유효성을 구분해 이어하기 가능 여부를 반환한다. */
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
