#include "AetherPrototypeCheckpointSnapshot.h"

#include "AetherPrototypeSaveGame.h"
#include "AetherPrototypeSaveSchemaPolicy.h"

int32 FAetherPrototypeCheckpointSnapshot::ResolveProgressRank(FName CheckpointLabel, int32 ExplicitCheckpointProgressRank)
{
	if (ExplicitCheckpointProgressRank > 0)
	{
		return ExplicitCheckpointProgressRank;
	}

	const FString CheckpointLabelString = CheckpointLabel.ToString();
	int32 UnderscoreIndex = INDEX_NONE;
	if (!CheckpointLabelString.FindLastChar(TEXT('_'), UnderscoreIndex))
	{
		return FMath::Max(0, ExplicitCheckpointProgressRank);
	}

	const FString Suffix = CheckpointLabelString.Mid(UnderscoreIndex + 1);
	if (Suffix.IsNumeric())
	{
		return FMath::Max(0, FCString::Atoi(*Suffix));
	}

	if (Suffix.Len() == 1)
	{
		const TCHAR UpperLetter = FChar::ToUpper(Suffix[0]);
		if (UpperLetter >= TEXT('A') && UpperLetter <= TEXT('Z'))
		{
			return static_cast<int32>(UpperLetter - TEXT('A')) + 1;
		}
	}

	return FMath::Max(0, ExplicitCheckpointProgressRank);
}

void FAetherPrototypeCheckpointSnapshot::WriteSaveGame(UAetherPrototypeSaveGame& SaveGameObject, const FAetherPrototypeCheckpointSnapshotState& SnapshotState)
{
	FAetherPrototypeSaveSchemaPolicy::StampCurrentSchema(SaveGameObject);
	SaveGameObject.bHasActiveCheckpoint = SnapshotState.bHasActiveCheckpoint;
	SaveGameObject.ActiveCheckpointTransform = SnapshotState.ActiveCheckpointTransform;
	SaveGameObject.ActiveCheckpointLabel = SnapshotState.ActiveCheckpointLabel;
	SaveGameObject.ActiveCheckpointProgressRank = SnapshotState.ActiveCheckpointProgressRank;
	SaveGameObject.PlayerCurrentHealth = SnapshotState.PlayerCurrentHealth;
	SaveGameObject.PrototypeHealingItemCount = SnapshotState.PrototypeHealingItemCount;
	SaveGameObject.ActivePrototypeEncounterLabel = SnapshotState.ActivePrototypeEncounterLabel;
	SaveGameObject.LastCompletedPrototypeEncounterLabel = SnapshotState.LastCompletedPrototypeEncounterLabel;
	SaveGameObject.CompletedPrototypeEncounterLabels = SnapshotState.CompletedPrototypeEncounterLabels.Array();
	SaveGameObject.bPrototypeCombatRoundComplete = SnapshotState.bPrototypeCombatRoundComplete;
	SaveGameObject.PrototypeRoundDefeatCount = SnapshotState.PrototypeRoundDefeatCount;
	SaveGameObject.PrototypeRoundKillGoal = SnapshotState.PrototypeRoundKillGoal;
	SaveGameObject.bPrototypeLevelComplete = SnapshotState.bPrototypeLevelComplete;
	SaveGameObject.CompletedPrototypeLevelGoalLabel = SnapshotState.CompletedPrototypeLevelGoalLabel;
	SaveGameObject.bCathedralEndingComplete = SnapshotState.bCathedralEndingComplete;
	SaveGameObject.CollectedPrototypeKeyLabels = SnapshotState.CollectedPrototypeKeyLabels.Array();
	SaveGameObject.CollectedPrototypeRewardLabels = SnapshotState.CollectedPrototypeRewardLabels.Array();
	SaveGameObject.CollectedPrototypeLoreLabels = SnapshotState.CollectedPrototypeLoreLabels.Array();
	SaveGameObject.ActivatedPrototypeLeverLabels = SnapshotState.ActivatedPrototypeLeverLabels.Array();
	SaveGameObject.UnlockedPrototypeProgressGateLabels = SnapshotState.UnlockedPrototypeProgressGateLabels.Array();
	SaveGameObject.UnlockedPrototypeKeyGateLabels = SnapshotState.UnlockedPrototypeKeyGateLabels.Array();
	SaveGameObject.OpenedPrototypeChestLabels = SnapshotState.OpenedPrototypeChestLabels.Array();
	SaveGameObject.PlayedPrototypeDialogueLabels = SnapshotState.PlayedPrototypeDialogueLabels.Array();
}

FAetherPrototypeCheckpointSnapshotState FAetherPrototypeCheckpointSnapshot::ReadSaveGame(
	const UAetherPrototypeSaveGame& SaveGameObject,
	FName CathedralEndingGoalLabel)
{
	FAetherPrototypeCheckpointSnapshotState SnapshotState;
	const FAetherPrototypeSaveSchemaLoadPlan LoadPlan = FAetherPrototypeSaveSchemaPolicy::BuildLoadPlan(SaveGameObject);
	SnapshotState.SaveSchemaVersion = LoadPlan.TargetSchemaVersion;
	SnapshotState.SaveSchemaLabel = FAetherPrototypeSaveSchemaPolicy::GetCurrentSchemaLabel();
	SnapshotState.bLoadedFromLegacySaveSchema = LoadPlan.bNeedsMigration;
	SnapshotState.bHasActiveCheckpoint = SaveGameObject.bHasActiveCheckpoint;
	SnapshotState.ActiveCheckpointTransform = SaveGameObject.ActiveCheckpointTransform;
	SnapshotState.ActiveCheckpointLabel = SaveGameObject.ActiveCheckpointLabel;
	SnapshotState.ActiveCheckpointProgressRank = ResolveProgressRank(
		SnapshotState.ActiveCheckpointLabel,
		SaveGameObject.ActiveCheckpointProgressRank);
	SnapshotState.PlayerCurrentHealth = SaveGameObject.PlayerCurrentHealth;
	SnapshotState.PrototypeHealingItemCount = SaveGameObject.PrototypeHealingItemCount;
	SnapshotState.ActivePrototypeEncounterLabel = SaveGameObject.ActivePrototypeEncounterLabel;
	SnapshotState.LastCompletedPrototypeEncounterLabel = SaveGameObject.LastCompletedPrototypeEncounterLabel;
	SnapshotState.CompletedPrototypeEncounterLabels = TSet<FName>(SaveGameObject.CompletedPrototypeEncounterLabels);
	if (!SnapshotState.LastCompletedPrototypeEncounterLabel.IsNone())
	{
		SnapshotState.CompletedPrototypeEncounterLabels.Add(SnapshotState.LastCompletedPrototypeEncounterLabel);
	}

	SnapshotState.bPrototypeCombatRoundComplete = SaveGameObject.bPrototypeCombatRoundComplete;
	SnapshotState.PrototypeRoundDefeatCount = FMath::Max(0, SaveGameObject.PrototypeRoundDefeatCount);
	SnapshotState.PrototypeRoundKillGoal = FMath::Max(1, SaveGameObject.PrototypeRoundKillGoal);
	SnapshotState.bPrototypeRoundGoalMetCuePlayed =
		SnapshotState.bPrototypeCombatRoundComplete ||
		SnapshotState.PrototypeRoundDefeatCount >= SnapshotState.PrototypeRoundKillGoal;

	SnapshotState.bPrototypeLevelComplete = SaveGameObject.bPrototypeLevelComplete;
	SnapshotState.CompletedPrototypeLevelGoalLabel = SaveGameObject.CompletedPrototypeLevelGoalLabel;
	SnapshotState.bCathedralEndingComplete =
		SaveGameObject.bCathedralEndingComplete ||
		(SnapshotState.bPrototypeLevelComplete && SnapshotState.CompletedPrototypeLevelGoalLabel == CathedralEndingGoalLabel);

	SnapshotState.CollectedPrototypeKeyLabels = TSet<FName>(SaveGameObject.CollectedPrototypeKeyLabels);
	SnapshotState.CollectedPrototypeRewardLabels = TSet<FName>(SaveGameObject.CollectedPrototypeRewardLabels);
	SnapshotState.CollectedPrototypeLoreLabels = TSet<FName>(SaveGameObject.CollectedPrototypeLoreLabels);
	SnapshotState.ActivatedPrototypeLeverLabels = TSet<FName>(SaveGameObject.ActivatedPrototypeLeverLabels);
	SnapshotState.UnlockedPrototypeProgressGateLabels = TSet<FName>(SaveGameObject.UnlockedPrototypeProgressGateLabels);
	SnapshotState.UnlockedPrototypeKeyGateLabels = TSet<FName>(SaveGameObject.UnlockedPrototypeKeyGateLabels);
	SnapshotState.OpenedPrototypeChestLabels = TSet<FName>(SaveGameObject.OpenedPrototypeChestLabels);
	SnapshotState.PlayedPrototypeDialogueLabels = TSet<FName>(SaveGameObject.PlayedPrototypeDialogueLabels);
	return SnapshotState;
}

FString FAetherPrototypeCheckpointSnapshot::BuildQASummary(const FString& Context, const FAetherPrototypeCheckpointSnapshotState& SnapshotState)
{
	const FString CheckpointLabel = SnapshotState.bHasActiveCheckpoint ? SnapshotState.ActiveCheckpointLabel.ToString() : TEXT("none");
	const FString ActiveEncounterLabel = SnapshotState.ActivePrototypeEncounterLabel.IsNone() ? TEXT("none") : SnapshotState.ActivePrototypeEncounterLabel.ToString();
	const FString LastEncounterLabel = SnapshotState.LastCompletedPrototypeEncounterLabel.IsNone() ? TEXT("none") : SnapshotState.LastCompletedPrototypeEncounterLabel.ToString();
	const FString LevelGoalLabel = SnapshotState.bPrototypeLevelComplete ? SnapshotState.CompletedPrototypeLevelGoalLabel.ToString() : TEXT("none");
	const FString EndingState = SnapshotState.bCathedralEndingComplete ? TEXT("true") : TEXT("false");

	return FString::Printf(
		TEXT("Checkpoint QA snapshot %s / schema %d %s legacy %s / checkpoint %s rank %d / active %s / last %s / completed %s / rewards %s / keys %s / lore %s / levers %s / progress gates %s / key gates %s / chests %s / dialogues %s / level %s / ending %s"),
		*Context,
		SnapshotState.SaveSchemaVersion,
		*SnapshotState.SaveSchemaLabel.ToString(),
		SnapshotState.bLoadedFromLegacySaveSchema ? TEXT("true") : TEXT("false"),
		*CheckpointLabel,
		SnapshotState.ActiveCheckpointProgressRank,
		*ActiveEncounterLabel,
		*LastEncounterLabel,
		*FormatNameSetForQA(SnapshotState.CompletedPrototypeEncounterLabels),
		*FormatNameSetForQA(SnapshotState.CollectedPrototypeRewardLabels),
		*FormatNameSetForQA(SnapshotState.CollectedPrototypeKeyLabels),
		*FormatNameSetForQA(SnapshotState.CollectedPrototypeLoreLabels),
		*FormatNameSetForQA(SnapshotState.ActivatedPrototypeLeverLabels),
		*FormatNameSetForQA(SnapshotState.UnlockedPrototypeProgressGateLabels),
		*FormatNameSetForQA(SnapshotState.UnlockedPrototypeKeyGateLabels),
		*FormatNameSetForQA(SnapshotState.OpenedPrototypeChestLabels),
		*FormatNameSetForQA(SnapshotState.PlayedPrototypeDialogueLabels),
		*LevelGoalLabel,
		*EndingState);
}

FString FAetherPrototypeCheckpointSnapshot::FormatNameSetForQA(const TSet<FName>& Labels)
{
	if (Labels.Num() <= 0)
	{
		return TEXT("none");
	}

	TArray<FString> Values;
	Values.Reserve(Labels.Num());
	for (const FName& Label : Labels)
	{
		if (!Label.IsNone())
		{
			Values.Add(Label.ToString());
		}
	}

	if (Values.Num() <= 0)
	{
		return TEXT("none");
	}

	Values.Sort();
	return FString::Join(Values, TEXT(","));
}
