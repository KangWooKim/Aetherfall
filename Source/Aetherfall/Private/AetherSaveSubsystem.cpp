#include "AetherSaveSubsystem.h"

#include "AetherPrototypeSaveGame.h"
#include "AetherPrototypeSaveSchemaPolicy.h"
#include "Kismet/GameplayStatics.h"

bool UAetherSaveSubsystem::SavePrototypeCheckpointSnapshot(UAetherPrototypeSaveGame* SaveGameObject) const
{
	if (SaveGameObject)
	{
		SaveGameObject->SavedAtUtc = FDateTime::UtcNow();
		SaveGameObject->SavedMapAsset = GetWorld() ? FName(*GetWorld()->GetMapName()) : NAME_None;
	}
	return SaveGameObject && UGameplayStatics::SaveGameToSlot(SaveGameObject, PrototypeCheckpointSlotName, PrototypeCheckpointUserIndex);
}

UAetherPrototypeSaveGame* UAetherSaveSubsystem::LoadPrototypeCheckpointSnapshot() const
{
	return Cast<UAetherPrototypeSaveGame>(UGameplayStatics::LoadGameFromSlot(PrototypeCheckpointSlotName, PrototypeCheckpointUserIndex));
}

bool UAetherSaveSubsystem::HasPrototypeCheckpointSnapshot() const
{
	return UGameplayStatics::DoesSaveGameExist(PrototypeCheckpointSlotName, PrototypeCheckpointUserIndex);
}

bool UAetherSaveSubsystem::ClearPrototypeCheckpointSnapshot() const
{
	return !HasPrototypeCheckpointSnapshot() || UGameplayStatics::DeleteGameInSlot(PrototypeCheckpointSlotName, PrototypeCheckpointUserIndex);
}

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
