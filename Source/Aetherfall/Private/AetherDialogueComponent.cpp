#include "AetherDialogueComponent.h"

#include "AetherDialogueDataAsset.h"
#include "AetherDialogueTtsService.h"
#include "AetherSettingsSubsystem.h"
#include "Engine/GameInstance.h"
#include "AetherPrototypeLabels.h"

namespace
{
	FName NameFromString(const FString& Value)
	{
		return FName(*Value);
	}

	FName EventLabel(const TCHAR* Prefix, FName Label)
	{
		return Label.IsNone()
			? NAME_None
			: NameFromString(FString::Printf(TEXT("%s_%s"), Prefix, *Label.ToString()));
	}

	FText TextLiteral(const TCHAR* Value)
	{
		return FText::FromString(Value);
	}

	FAetherDialogueLine MakeLine(
		const TCHAR* LineLabel,
		const TCHAR* SpeakerLabel,
		const TCHAR* LineText,
		const TCHAR* ObjectiveHint = TEXT(""),
		const TCHAR* GameplayLabel = TEXT(""))
	{
		FAetherDialogueLine Line;
		Line.LineLabel = FName(LineLabel);
		Line.SpeakerLabel = FName(SpeakerLabel);
		Line.SpeakerDisplayName = TextLiteral(SpeakerLabel);
		Line.Text = TextLiteral(LineText);
		Line.TtsId = Line.LineLabel;
		Line.VoiceId = FName(SpeakerLabel);
		Line.GameplayLabel = FName(GameplayLabel);
		Line.ObjectiveHint = TextLiteral(ObjectiveHint);
		Line.AutoAdvancePolicy = EAetherDialogueAutoAdvancePolicy::TtsComplete;
		return Line;
	}

	FAetherDialogueSequence MakeSequence(
		const TCHAR* SequenceLabel,
		FName TriggerLabel,
		const TArray<FAetherDialogueLine>& Lines,
		bool bBlocksGameplayInput = true)
	{
		FAetherDialogueSequence Sequence;
		Sequence.SequenceLabel = FName(SequenceLabel);
		Sequence.TriggerLabel = TriggerLabel;
		Sequence.Lines = Lines;
		Sequence.bPlayOnce = true;
		Sequence.bSaveWhenPlayed = true;
		Sequence.bBlocksGameplayInput = bBlocksGameplayInput;
		return Sequence;
	}
}

UAetherDialogueComponent::UAetherDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	TtsServiceClass = UAetherDialogueMockTtsService::StaticClass();
}

void UAetherDialogueComponent::BeginPlay()
{
	Super::BeginPlay();

	EnsureTtsService();

	RebuildRuntimeSequences();
}

void UAetherDialogueComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PendingDialogueTriggerLabels.Reset();
	ForceReplayPendingTriggerLabels.Reset();
	FinishActiveSequence();
	Super::EndPlay(EndPlayReason);
}

void UAetherDialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsDialogueActive())
	{
		return;
	}

	ActiveLineElapsedSeconds += FMath::Max(0.0f, DeltaTime);
	if (TtsService)
	{
		TtsService->TickService(DeltaTime);
	}

	const FAetherDialogueLine* ActiveLine = GetActiveLine();
	if (!ActiveLine)
	{
		FinishActiveSequence();
		return;
	}

	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UAetherSettingsSubsystem* Settings = GameInstance ? GameInstance->GetSubsystem<UAetherSettingsSubsystem>() : nullptr;
	if (Settings && !Settings->GetCurrentSettings().Custom.bDialogueAutoAdvance)
	{
		return;
	}

	const bool bTimedComplete =
		ActiveLine->AutoAdvancePolicy == EAetherDialogueAutoAdvancePolicy::Timed &&
		ActiveLineElapsedSeconds >= ActiveLineDurationSeconds;
	const bool bTtsComplete =
		ActiveLine->AutoAdvancePolicy == EAetherDialogueAutoAdvancePolicy::TtsComplete &&
		(!TtsService || TtsService->HasLineFinished());

	if (bTimedComplete || bTtsComplete)
	{
		AdvanceToNextLine();
	}
}

bool UAetherDialogueComponent::TryStartDialogue(FName TriggerLabel, bool bForceReplay)
{
	if (TriggerLabel.IsNone())
	{
		return false;
	}

	EnsureTtsService();

	RebuildRuntimeSequences();

	for (const FAetherDialogueSequence& Sequence : RuntimeSequences)
	{
		if (Sequence.TriggerLabel != TriggerLabel || Sequence.Lines.Num() <= 0)
		{
			continue;
		}

		if (!bForceReplay && Sequence.bPlayOnce && PlayedDialogueLabels.Contains(Sequence.SequenceLabel))
		{
			return false;
		}

		if (IsDialogueActive())
		{
			if (ActiveSequenceLabel == Sequence.SequenceLabel || PendingDialogueTriggerLabels.Contains(TriggerLabel))
			{
				return false;
			}

			PendingDialogueTriggerLabels.Add(TriggerLabel);
			if (bForceReplay)
			{
				ForceReplayPendingTriggerLabels.Add(TriggerLabel);
			}
			return true;
		}

		return StartSequence(Sequence);
	}

	return false;
}

void UAetherDialogueComponent::SkipOrAdvanceDialogue()
{
	if (!IsDialogueActive())
	{
		return;
	}

	if (TtsService && TtsService->IsLinePlaying())
	{
		TtsService->SkipLine();
	}

	AdvanceToNextLine();
}

void UAetherDialogueComponent::ResetPlayedDialogueLabels()
{
	PlayedDialogueLabels.Reset();
	PendingDialogueTriggerLabels.Reset();
	ForceReplayPendingTriggerLabels.Reset();
	FinishActiveSequence();
}

bool UAetherDialogueComponent::ShouldBlockGameplayInput() const
{
	const FAetherDialogueLine* ActiveLine = GetActiveLine();
	return IsDialogueActive() && (bActiveSequenceBlocksGameplayInput || (ActiveLine && ActiveLine->bBlocksGameplayInput));
}

FText UAetherDialogueComponent::GetCurrentSpeakerName() const
{
	const FAetherDialogueLine* ActiveLine = GetActiveLine();
	if (!ActiveLine)
	{
		return FText::GetEmpty();
	}

	return ActiveLine->SpeakerDisplayName.IsEmpty()
		? FText::FromName(ActiveLine->SpeakerLabel)
		: ActiveLine->SpeakerDisplayName;
}

FText UAetherDialogueComponent::GetCurrentDialogueText() const
{
	const FAetherDialogueLine* ActiveLine = GetActiveLine();
	return ActiveLine ? ActiveLine->Text : FText::GetEmpty();
}

FText UAetherDialogueComponent::GetCurrentObjectiveHint() const
{
	const FAetherDialogueLine* ActiveLine = GetActiveLine();
	return ActiveLine ? ActiveLine->ObjectiveHint : FText::GetEmpty();
}

bool UAetherDialogueComponent::HasPlayedDialogueLabel(FName DialogueLabel) const
{
	return !DialogueLabel.IsNone() && PlayedDialogueLabels.Contains(DialogueLabel);
}

void UAetherDialogueComponent::SetPlayedDialogueLabels(const TSet<FName>& DialogueLabels)
{
	PendingDialogueTriggerLabels.Reset();
	ForceReplayPendingTriggerLabels.Reset();
	FinishActiveSequence();
	PlayedDialogueLabels = DialogueLabels;
}

void UAetherDialogueComponent::RebuildRuntimeSequences()
{
	if (bRuntimeSequencesBuilt)
	{
		return;
	}

	RuntimeSequences.Reset();
	if (DialogueDataAsset)
	{
		RuntimeSequences.Append(DialogueDataAsset->DialogueSequences);
	}

	if (bUsePrototypeFallbackDialogue)
	{
		BuildPrototypeFallbackDialogue(RuntimeSequences);
	}

	bRuntimeSequencesBuilt = true;
}

void UAetherDialogueComponent::EnsureTtsService()
{
	if (TtsService)
	{
		return;
	}

	UClass* ResolvedServiceClass = TtsServiceClass
		? TtsServiceClass.Get()
		: UAetherDialogueMockTtsService::StaticClass();
	TtsService = NewObject<UAetherDialogueTtsService>(this, ResolvedServiceClass, TEXT("DialogueTtsService"));
}

void UAetherDialogueComponent::BuildPrototypeFallbackDialogue(TArray<FAetherDialogueSequence>& OutSequences) const
{
	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_OpeningWake"), FName(TEXT("Story_OpeningWake")), {
		MakeLine(TEXT("DLG_Elian_Opening_001"), TEXT("Elian"), TEXT("깨어났다면, 아직 균열이 너를 삼키지 못한 것이다.")),
		MakeLine(TEXT("DLG_Elian_Opening_002"), TEXT("Elian"), TEXT("푸른 빛을 따라가라. 빛의 끝에는 왕국의 상처가 있다."), TEXT("폐허의 빛을 따라가라"), TEXT("OBJ_Opening_Move")),
		MakeLine(TEXT("DLG_Elian_Opening_003"), TEXT("Elian"), TEXT("그 검은 네 것이 아니다. 하지만 지금은 네 손을 빌릴 것이다."))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_ForestIntro_Start"), EventLabel(TEXT("EncounterStart"), AetherPrototypeLabels::ForestIntro()), {
		MakeLine(TEXT("DLG_Elian_Combat_001"), TEXT("Elian"), TEXT("빈 갑옷을 두려워하지 마라. 안에 남은 것은 명령뿐이다."), TEXT("숲길을 막은 오염 기사를 쓰러뜨려라"), TEXT("OBJ_Forest_Clear"))
	}, false));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_FallenHamlet_Checkpoint"), EventLabel(TEXT("Checkpoint"), FName(TEXT("CP_FallenHamlet"))), {
		MakeLine(TEXT("DLG_Elian_Hamlet_001"), TEXT("Elian"), TEXT("이 마을은 성을 믿었다. 성은 마을을 닫아버렸다.")),
		MakeLine(TEXT("DLG_Elian_Hamlet_002"), TEXT("Elian"), TEXT("나는 Elian. 왕국의 마지막 기록관... 혹은 그 기록의 남은 소리다."), TEXT("잠긴 문을 열 열쇠를 찾아라"), TEXT("OBJ_Hamlet_FindKey"))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_HamletDoor_Locked"), EventLabel(TEXT("KeyGateLocked"), AetherPrototypeLabels::HamletDoor()), {
		MakeLine(TEXT("DLG_Elian_Hamlet_003"), TEXT("Elian"), TEXT("문은 아직 기억한다. 열쇠를 찾아라. 죽은 자들이 놓지 못한 것들 사이에 있을 것이다."), TEXT("잠긴 문을 열 열쇠를 찾아라"), TEXT("OBJ_Hamlet_FindKey"))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_HamletKey_Collected"), EventLabel(TEXT("KeyCollected"), AetherPrototypeLabels::HamletKey()), {
		MakeLine(TEXT("DLG_OBJ_Hamlet_OpenDoor"), TEXT("Elian"), TEXT("그 열쇠가 아직 문을 기억한다면, 성벽으로 가는 길도 열릴 것이다."), TEXT("마을 문을 열어 성벽으로 향하라"), TEXT("OBJ_Hamlet_OpenDoor"))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_HamletDoor_Unlocked"), EventLabel(TEXT("KeyGateUnlocked"), AetherPrototypeLabels::HamletDoor()), {
		MakeLine(TEXT("DLG_Elian_Hamlet_Unlock_001"), TEXT("Elian"), TEXT("문이 열렸다. 무너진 성벽 너머에서 대성당의 숨이 들린다."), TEXT("성벽을 넘어 지하 묘지로 향하라"), TEXT("OBJ_Wall_Reach"))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_HamletEntry_Start"), EventLabel(TEXT("EncounterStart"), AetherPrototypeLabels::HamletEntry()), {
		MakeLine(TEXT("DLG_Elian_Hamlet_004"), TEXT("Elian"), TEXT("이곳의 기사들은 주민을 지키려 했다. 이제는 지나가는 모든 것을 막는다."))
	}, false));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_HamletEntry_Complete"), EventLabel(TEXT("EncounterComplete"), AetherPrototypeLabels::HamletEntry()), {
		MakeLine(TEXT("DLG_Elian_Hamlet_005"), TEXT("Elian"), TEXT("길이 열렸다. 하지만 대성당에 가까워질수록 검은 더 굶주릴 것이다."), TEXT("성벽을 넘어 지하 묘지로 향하라"))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_BrokenWall_Checkpoint"), EventLabel(TEXT("Checkpoint"), FName(TEXT("CP_BrokenWall"))), {
		MakeLine(TEXT("DLG_Elian_Wall_001"), TEXT("Elian"), TEXT("성벽은 밖을 막기 위해 세웠다. 마지막 날에는 안을 막기 위해 닫혔다."), TEXT("성벽의 숏컷을 열어 퇴로를 확보하라"), TEXT("OBJ_Wall_OpenShortcut")),
		MakeLine(TEXT("DLG_Elian_Wall_002"), TEXT("Elian"), TEXT("하나를 바라보되, 나머지를 잊지 마라. 오염된 기사들은 아직 진형을 기억한다."))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_BrokenWall_Lever"), EventLabel(TEXT("LeverActivated"), FName(TEXT("PrototypeLever"))), {
		MakeLine(TEXT("DLG_Elian_Wall_003"), TEXT("Elian"), TEXT("오래된 방어로가 열렸다. 왕국은 무너졌지만 길은 아직 쓸 수 있다."))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_ForestShortcutReward_Collected"), EventLabel(TEXT("RewardCollected"), AetherPrototypeLabels::ForestShortcutChestReward()), {
		MakeLine(TEXT("DLG_Elian_Wall_Reward_001"), TEXT("Elian"), TEXT("남겨진 물자는 우연이 아니다. 누군가는 마지막까지 이 길로 돌아올 사람을 기다렸다."))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_AetherShard_Collected"), EventLabel(TEXT("RewardCollected"), AetherPrototypeLabels::AetherResonanceShard()), {
		MakeLine(TEXT("DLG_WorldEcho_Shard_001"), TEXT("World Echo"), TEXT("조각난 에테르가 대검에 스민다. 균열은 너를 알아보고 있다."))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_LowerCrypt_Checkpoint"), EventLabel(TEXT("Checkpoint"), FName(TEXT("CP_LowerCrypt"))), {
		MakeLine(TEXT("DLG_Elian_Crypt_001"), TEXT("Elian"), TEXT("이 아래에는 왕국의 죄보다, 왕국의 맹세가 더 깊이 묻혀 있다."), TEXT("대성당 문을 지키는 봉인을 찾아라"), TEXT("OBJ_Crypt_FindSeal")),
		MakeLine(TEXT("DLG_Elian_Crypt_003"), TEXT("Elian"), TEXT("문을 지키는 자는 아직 자신의 이름을 기억할지도 모른다. 그게 더 잔인한 일이다."))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_CryptLore_Collected"), EventLabel(TEXT("LoreCollected"), AetherPrototypeLabels::LoreCrypt001()), {
		MakeLine(TEXT("DLG_Elian_Crypt_002"), TEXT("Elian"), TEXT("Aurel은 도망치지 않았다. 그래서 더 위험해졌다."))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_LowerCryptElite_Start"), EventLabel(TEXT("EncounterStart"), AetherPrototypeLabels::LowerCryptElite()), {
		MakeLine(TEXT("DLG_Cael_Intro_001"), TEXT("Cael"), TEXT("대성당은 닫혔다. 산 자도, 죽은 자도 지나갈 수 없다."), TEXT("묘지의 수호기사를 쓰러뜨려라"), TEXT("OBJ_Crypt_DefeatCael")),
		MakeLine(TEXT("DLG_Cael_Intro_002"), TEXT("Cael"), TEXT("맹세는 무덤보다 오래간다."))
	}, false));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_LowerCryptElite_Complete"), EventLabel(TEXT("EncounterComplete"), AetherPrototypeLabels::LowerCryptElite()), {
		MakeLine(TEXT("DLG_Cael_Death_001"), TEXT("Cael"), TEXT("Aurel... 문을... 열지 마십시오...")),
		MakeLine(TEXT("DLG_Elian_Cael_Death_001"), TEXT("Elian"), TEXT("그는 실패한 수호자가 아니었다. 너무 오래 성공한 문지기였을 뿐이다."), TEXT("대성당의 균열로 향하라"), TEXT("OBJ_Cathedral_Enter"))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_CryptEliteReward_Collected"), EventLabel(TEXT("RewardCollected"), AetherPrototypeLabels::CryptEliteReward()), {
		MakeLine(TEXT("DLG_Elian_Crypt_Reward_001"), TEXT("Elian"), TEXT("Cael의 봉인이 네 손에 남았다. 이제 대성당의 문도 너를 외면하지 못할 것이다."), TEXT("대성당으로 진입하라"), TEXT("OBJ_Cathedral_Enter"))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_CryptGate_Unlocked"), EventLabel(TEXT("ProgressGateUnlocked"), AetherPrototypeLabels::GateLowerCryptElite()), {
		MakeLine(TEXT("DLG_WorldEcho_CathedralGate_001"), TEXT("World Echo"), TEXT("오래 닫혀 있던 길이 봉인의 주인을 기억한다."))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_Cathedral_Checkpoint"), EventLabel(TEXT("Checkpoint"), FName(TEXT("CP_CathedralEntry"))), {
		MakeLine(TEXT("DLG_Elian_Cathedral_001"), TEXT("Elian"), TEXT("여기부터는 기록이 끊긴다. 내가 남긴 글도, 내가 기억한 나도.")),
		MakeLine(TEXT("DLG_Elian_Cathedral_002"), TEXT("Elian"), TEXT("Aurel은 왕국의 방패였다. 방패는 부서진 뒤에도 막는 법을 잊지 못한다."))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_CathedralBoss_Start"), EventLabel(TEXT("EncounterStart"), AetherPrototypeLabels::CathedralBoss()), {
		MakeLine(TEXT("DLG_Aurel_Intro_001"), TEXT("Aurel"), TEXT("멈춰라. 이 문 너머에는 구원받을 왕국이 없다."), TEXT("Aurel, The Hollow Warden을 쓰러뜨려라"), TEXT("OBJ_Boss_DefeatAurel")),
		MakeLine(TEXT("DLG_Aurel_Intro_002"), TEXT("Aurel"), TEXT("그 검... 아직도 굶주리는군.")),
		MakeLine(TEXT("DLG_Aurel_Intro_003"), TEXT("Aurel"), TEXT("또 한 명의 봉인이 되러 왔나.")),
		MakeLine(TEXT("DLG_Aurel_Intro_004"), TEXT("Aurel"), TEXT("그렇다면 증명해라. 네 의무가 내 맹세보다 무거운지."))
	}, false));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_CathedralBoss_Phase2"), FName(TEXT("BossPhase_CathedralBoss_Phase2")), {
		MakeLine(TEXT("DLG_Aurel_Phase2_001"), TEXT("Aurel"), TEXT("봉인이 갈라진다... 그렇다면 남은 맹세까지 태워 너를 막겠다."))
	}, false));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_CathedralBoss_LowHealth"), FName(TEXT("BossHealth_CathedralBoss_Low")), {
		MakeLine(TEXT("DLG_Aurel_LowHealth_001"), TEXT("Aurel"), TEXT("끝이 보인다고 믿지 마라. 균열은 패배한 자의 틈을 가장 먼저 삼킨다."))
	}, false));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_CathedralBoss_Complete"), EventLabel(TEXT("EncounterComplete"), AetherPrototypeLabels::CathedralBoss()), {
		MakeLine(TEXT("DLG_Aurel_Defeat_001"), TEXT("Aurel"), TEXT("이번에는... 끝내라."), TEXT("대검으로 균열을 봉인하라"), TEXT("OBJ_Ending_SealRift")),
		MakeLine(TEXT("DLG_Aurel_Defeat_002"), TEXT("Aurel"), TEXT("왕국은 이미 죽었다. 그 죽음만은 밖으로 보내지 마라."))
	}));

	OutSequences.Add(MakeSequence(TEXT("DLGSEQ_CathedralEnding_Complete"), EventLabel(TEXT("LevelComplete"), AetherPrototypeLabels::CathedralEnding()), {
		MakeLine(TEXT("DLG_Elian_Ending_001"), TEXT("Elian"), TEXT("대검을 균열에 꽂아라. 네 안의 오염이 길을 기억할 것이다.")),
		MakeLine(TEXT("DLG_Elian_Ending_002"), TEXT("Elian"), TEXT("Eldran은 돌아오지 않는다. 하지만 오늘, 더 무너지지도 않는다.")),
		MakeLine(TEXT("DLG_Elian_Ending_003"), TEXT("Elian"), TEXT("기록은 여기서 끝난다. 네 길은... 아직 끝나지 않았다."))
	}));
}

bool UAetherDialogueComponent::StartSequence(const FAetherDialogueSequence& Sequence)
{
	if (Sequence.SequenceLabel.IsNone() || Sequence.Lines.Num() <= 0)
	{
		return false;
	}

	ActiveSequenceLabel = Sequence.SequenceLabel;
	ActiveLines = Sequence.Lines;
	ActiveLineIndex = 0;
	bActiveSequenceBlocksGameplayInput = Sequence.bBlocksGameplayInput;

	if (Sequence.bSaveWhenPlayed)
	{
		PlayedDialogueLabels.Add(Sequence.SequenceLabel);
	}

	StartCurrentLine();
	return true;
}

void UAetherDialogueComponent::StartCurrentLine()
{
	const FAetherDialogueLine* ActiveLine = GetActiveLine();
	if (!ActiveLine)
	{
		FinishActiveSequence();
		return;
	}

	ActiveLineElapsedSeconds = 0.0f;
	ActiveLineDurationSeconds = EstimateLineDurationSeconds(*ActiveLine);
	if (TtsService)
	{
		TtsService->StartLine(*ActiveLine, ActiveLineDurationSeconds);
	}
}

void UAetherDialogueComponent::AdvanceToNextLine()
{
	if (!IsDialogueActive())
	{
		return;
	}

	if (const FAetherDialogueLine* CompletedLine = GetActiveLine(); CompletedLine && !CompletedLine->CompletionEventLabel.IsNone())
	{
		OnDialogueGameplayEvent.Broadcast(CompletedLine->CompletionEventLabel);
	}
	if (TtsService && TtsService->IsLinePlaying())
	{
		TtsService->SkipLine();
	}

	++ActiveLineIndex;
	if (!ActiveLines.IsValidIndex(ActiveLineIndex))
	{
		FinishActiveSequence();
		return;
	}

	StartCurrentLine();
}

void UAetherDialogueComponent::FinishActiveSequence()
{
	ActiveLines.Reset();
	ActiveSequenceLabel = NAME_None;
	ActiveLineIndex = INDEX_NONE;
	ActiveLineElapsedSeconds = 0.0f;
	ActiveLineDurationSeconds = 0.0f;
	bActiveSequenceBlocksGameplayInput = false;
	if (TtsService)
	{
		TtsService->SkipLine();
	}

	StartNextPendingDialogue();
}

void UAetherDialogueComponent::StartNextPendingDialogue()
{
	while (!IsDialogueActive() && PendingDialogueTriggerLabels.Num() > 0)
	{
		const FName TriggerLabel = PendingDialogueTriggerLabels[0];
		PendingDialogueTriggerLabels.RemoveAt(0);
		const bool bForceReplay = ForceReplayPendingTriggerLabels.Remove(TriggerLabel) > 0;
		if (TryStartDialogue(TriggerLabel, bForceReplay))
		{
			return;
		}
	}
}

float UAetherDialogueComponent::EstimateLineDurationSeconds(const FAetherDialogueLine& DialogueLine) const
{
	if (DialogueLine.AutoAdvanceDelay > 0.0f)
	{
		return DialogueLine.AutoAdvanceDelay;
	}

	const int32 CharacterCount = DialogueLine.Text.ToString().Len();
	const float EstimatedDuration = static_cast<float>(CharacterCount) / FMath::Max(1.0f, MockTtsCharactersPerSecond);
	return FMath::Clamp(EstimatedDuration, MockTtsMinimumLineDuration, MockTtsMaximumLineDuration);
}

const FAetherDialogueLine* UAetherDialogueComponent::GetActiveLine() const
{
	return ActiveLines.IsValidIndex(ActiveLineIndex) ? &ActiveLines[ActiveLineIndex] : nullptr;
}
