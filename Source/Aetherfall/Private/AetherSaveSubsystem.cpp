/** 체크포인트 저장 슬롯의 입출력과 메뉴용 요약을 제공하는 게임 인스턴스 서브시스템이다. */
#include "AetherSaveSubsystem.h"

#include "AetherPrototypeSaveGame.h"
#include "AetherPrototypeSaveSchemaPolicy.h"
#include "Kismet/GameplayStatics.h"

/** 저장 시각과 현재 맵 이름을 갱신한 뒤 지정된 슬롯에 스냅샷을 기록한다. */
bool UAetherSaveSubsystem::SavePrototypeCheckpointSnapshot(UAetherPrototypeSaveGame* SaveGameObject) const
{
	if (SaveGameObject)
	{
		SaveGameObject->SavedAtUtc = FDateTime::UtcNow();
		SaveGameObject->SavedMapAsset = GetWorld() ? FName(*GetWorld()->GetMapName()) : NAME_None;
	}
	return SaveGameObject && UGameplayStatics::SaveGameToSlot(SaveGameObject, PrototypeCheckpointSlotName, PrototypeCheckpointUserIndex);
}

/** 슬롯을 읽어 예상 저장 클래스인지 확인한다. 버전 호환성 판단은 별도 정책에서 수행한다. */
UAetherPrototypeSaveGame* UAetherSaveSubsystem::LoadPrototypeCheckpointSnapshot() const
{
	return Cast<UAetherPrototypeSaveGame>(UGameplayStatics::LoadGameFromSlot(PrototypeCheckpointSlotName, PrototypeCheckpointUserIndex));
}

bool UAetherSaveSubsystem::HasPrototypeCheckpointSnapshot() const
{
	return UGameplayStatics::DoesSaveGameExist(PrototypeCheckpointSlotName, PrototypeCheckpointUserIndex);
}

/** 저장 슬롯이 없으면 이미 삭제된 상태로 취급하고 있으면 해당 슬롯만 삭제한다. */
bool UAetherSaveSubsystem::ClearPrototypeCheckpointSnapshot() const
{
	return !HasPrototypeCheckpointSnapshot() || UGameplayStatics::DeleteGameInSlot(PrototypeCheckpointSlotName, PrototypeCheckpointUserIndex);
}

/** 슬롯 존재, 읽기 성공, 버전 호환성과 체크포인트 유효성을 구분해 이어하기 가능 여부를 반환한다. */
FAetherSaveSlotSummary UAetherSaveSubsystem::GetPrototypeCheckpointSummary() const
{
	FAetherSaveSlotSummary Summary;
	Summary.SlotName = PrototypeCheckpointSlotName;
	Summary.UserIndex = PrototypeCheckpointUserIndex;
	Summary.bExists = HasPrototypeCheckpointSnapshot();
	if (!Summary.bExists)
	{
		Summary.StatusText = NSLOCTEXT("AetherSave", "NoSaveData", "No journey has been recorded.");
		return Summary;
	}

	const UAetherPrototypeSaveGame* SaveGameObject = LoadPrototypeCheckpointSnapshot();
	if (!SaveGameObject)
	{
		Summary.StatusText = NSLOCTEXT("AetherSave", "UnreadableSaveData", "The recorded journey could not be read.");
		return Summary;
	}

	const FAetherPrototypeSaveSchemaLoadPlan LoadPlan = FAetherPrototypeSaveSchemaPolicy::BuildLoadPlan(*SaveGameObject);
	Summary.bLoadable = LoadPlan.bCanLoad && SaveGameObject->bHasActiveCheckpoint;
	Summary.CheckpointLabel = SaveGameObject->ActiveCheckpointLabel;
	Summary.CheckpointProgressRank = SaveGameObject->ActiveCheckpointProgressRank;
	Summary.SaveSchemaVersion = SaveGameObject->SaveSchemaVersion;
	Summary.SavedAtUtc = SaveGameObject->SavedAtUtc;
	Summary.SavedMapAsset = SaveGameObject->SavedMapAsset;

	if (!LoadPlan.bCanLoad)
	{
		Summary.StatusText = NSLOCTEXT("AetherSave", "UnsupportedSaveData", "This save was created by a newer version.");
	}
	else if (!SaveGameObject->bHasActiveCheckpoint)
	{
		Summary.StatusText = NSLOCTEXT("AetherSave", "IncompleteSaveData", "No valid checkpoint was found in this save.");
	}
	else
	{
		Summary.StatusText = FText::Format(
			NSLOCTEXT("AetherSave", "CheckpointSaveSummary", "Checkpoint {0}  |  Rank {1}"),
			FText::FromName(Summary.CheckpointLabel),
			FText::AsNumber(Summary.CheckpointProgressRank));
	}
	return Summary;
}

TArray<FAetherSaveSlotSummary> UAetherSaveSubsystem::GetSaveSlotSummaries() const
{
	return {GetPrototypeCheckpointSummary()};
}
