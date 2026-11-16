#include "Characters/BattleEnemyCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Characters/PartyMemberCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Engine/DataTable.h"
ABattleEnemyCharacter::ABattleEnemyCharacter()
{
	bIsEnemy = true;
}
FName ABattleEnemyCharacter::GetMoveSetKey() const
{
	if (!MoveSetId.IsNone())
	{
		return MoveSetId;
	}
	return StatsRowName;
}
void ABattleEnemyCharacter::OnBattleRoundStart(ATurnBattleManager* Manager)
{
	MovesUsedThisRound.Empty();
}
bool ABattleEnemyCharacter::DoesMemberMoveFail(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName, EBattleAction Category)
{
	return false;
}
bool ABattleEnemyCharacter::IsMemberMoveLocked(const ATurnBattleManager* Manager, int32 MemberIndex, FName MoveId) const
{
	return false;
}
FString ABattleEnemyCharacter::GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const
{
	return FString();
}
void ABattleEnemyCharacter::BeforeMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName)
{
	CheckMeditation(Manager, MemberIndex);
	if (MaliceRound == Manager->GetRound())
	{
		StartMaliceWatch(Manager);
	}
}
void ABattleEnemyCharacter::AfterMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName)
{
	if (bMaliceWatching)
	{
		bool bCaught = FinishMaliceWatch(Manager);
		if (bCaught)
		{
			MaliceRound = -1;
			TriggerMalice(Manager, MemberIndex);
		}
	}
	CheckCycle(Manager);
	if (IsGrailActive(Manager))
	{
		SpillGrail(Manager);
	}
}
FString ABattleEnemyCharacter::GetStatusNote(const ATurnBattleManager* Manager) const
{
	FString Note;
	if (IsMeditationPending(Manager))
	{
		Note = Note + TEXT("Meditation ");
	}
	int32 Charges = GetPendingCycleCharges(Manager);
	if (Charges > 0)
	{
		Note = Note + FString::Printf(TEXT("Cycle x%d "), Charges);
	}
	if (GrailLastRound >= Manager->GetRound())
	{
		Note = Note + TEXT("Grail ");
	}
	if (IsMaliceArmed(Manager))
	{
		Note = Note + TEXT("Malice ");
	}
	return Note;
}
void ABattleEnemyCharacter::OnBossPhaseStart(ATurnBattleManager* Manager)
{
}
void ABattleEnemyCharacter::OnRetributionCompleted(ATurnBattleManager* Manager, int32 MemberIndex)
{
}
int32 ABattleEnemyCharacter::GetBossTurnCount(ATurnBattleManager* Manager, int32 DefaultTurns)
{
	return DefaultTurns;
}
float ABattleEnemyCharacter::GetHitMultiplierFor(ATurnBattleManager* Manager, int32 MemberIndex) const
{
	return 1.0f;
}
TArray<FName> ABattleEnemyCharacter::GatherMoveIds(ATurnBattleManager* Manager) const
{
	TArray<FName> MoveIds;
	if (Manager->BossMoveTable == nullptr)
	{
		return MoveIds;
	}
	FName SetKey = GetMoveSetKey();
	TArray<FName> AllNames = Manager->BossMoveTable->GetRowNames();
	for (int32 i = 0; i < AllNames.Num(); i++)
	{
		const FBossMoveRow* Move = Manager->GetBossMove(AllNames[i]);
		if (Move != nullptr)
		{
			if (Move->MoveSetId == SetKey)
			{
				MoveIds.Add(AllNames[i]);
			}
		}
	}
	return MoveIds;
}
bool ABattleEnemyCharacter::TakeTurnFromMoveTable(ATurnBattleManager* Manager)
{
	if (Manager == nullptr)
	{
		return false;
	}
	TArray<FName> MoveIds = GatherMoveIds(Manager);
	if (MoveIds.Num() == 0)
	{
		return false;
	}
	FName MoveId = ChooseMove(Manager, MoveIds);
	if (MoveId.IsNone())
	{
		Manager->ShowMessage(FString::Printf(TEXT("%s hesitates"), *DisplayName), FColor::Orange);
		return true;
	}
	const FBossMoveRow* Move = Manager->GetBossMove(MoveId);
	int32 TargetIndex = -1;
	if (bLastChoiceWasBlunder)
	{
		TargetIndex = PickRandomTarget(Manager, MoveId, Move);
		if (!bPicksMovesRandomly)
		{
			UE_LOG(LogTemp, Log, TEXT("%s blunders into %s"), *DisplayName, *MoveId.ToString());
		}
	}
	else
	{
		TargetIndex = PickMoveTarget(Manager, MoveId, Move);
	}
	MovesUsedThisRound.Add(MoveId);
	LastMoveId = MoveId;
	ShowMoveMessage(Manager, Move, TargetIndex);
	PerformMove(Manager, MoveId, Move, TargetIndex);
	return true;
}
FName ABattleEnemyCharacter::ChooseMove(ATurnBattleManager* Manager, TArray<FName> MoveIds)
{
	bLastChoiceWasBlunder = false;
	TArray<FName> UsableMoves;
	TArray<float> Scores;
	for (int32 i = 0; i < MoveIds.Num(); i++)
	{
		const FBossMoveRow* Move = Manager->GetBossMove(MoveIds[i]);
		if (Move != nullptr)
		{
			float Score = ScoreMove(Manager, MoveIds[i], Move);
			if (Score > 0.0f)
			{
				UsableMoves.Add(MoveIds[i]);
				Scores.Add(Score);
			}
		}
	}
	if (UsableMoves.Num() == 0)
	{
		return NAME_None;
	}
	bool bBlunder = bPicksMovesRandomly;
	if (!bBlunder)
	{
		if (FMath::FRand() < BlunderChance)
		{
			bBlunder = true;
		}
	}
	if (bBlunder)
	{
		bLastChoiceWasBlunder = true;
		int32 RandomPick = FMath::RandRange(0, UsableMoves.Num() - 1);
		return UsableMoves[RandomPick];
	}
	float BestScore = Scores[0];
	for (int32 i = 1; i < Scores.Num(); i++)
	{
		if (Scores[i] > BestScore)
		{
			BestScore = Scores[i];
		}
	}
	TArray<FName> BestMoves;
	for (int32 i = 0; i < Scores.Num(); i++)
	{
		if (Scores[i] >= BestScore - 0.001f)
		{
			BestMoves.Add(UsableMoves[i]);
		}
	}
	int32 BestPick = FMath::RandRange(0, BestMoves.Num() - 1);
	return BestMoves[BestPick];
}
float ABattleEnemyCharacter::ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	if (Move->UsesPerRound > 0)
	{
		if (CountUsesThisRound(MoveId) >= Move->UsesPerRound)
		{
			return 0.0f;
		}
	}
	if (IsModifierOnlyMove(Move))
	{
		if (Move->TargetKind != EBossMoveTargetKind::Self)
		{
			if (PickRandomMemberBelowMaxStacks(Manager, MoveId, Move) < 0)
			{
				return 0.0f;
			}
		}
	}
	return Move->ScoreBase;
}
int32 ABattleEnemyCharacter::PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	return PickRandomTarget(Manager, MoveId, Move);
}
int32 ABattleEnemyCharacter::PickRandomTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const
{
	if (Move->TargetKind != EBossMoveTargetKind::OneMember)
	{
		return -1;
	}
	if (IsModifierOnlyMove(Move))
	{
		int32 FreshTarget = PickRandomMemberBelowMaxStacks(Manager, MoveId, Move);
		if (FreshTarget >= 0)
		{
			return FreshTarget;
		}
	}
	return Manager->PickRandomLivingMember();
}
void ABattleEnemyCharacter::PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex)
{
	if (Move->TargetKind == EBossMoveTargetKind::Self)
	{
		ApplyMoveModifier(this, MoveId, Move);
		return;
	}
	if (Move->TargetKind == EBossMoveTargetKind::WholeParty)
	{
		for (int32 i = 0; i < Manager->GetPartyCount(); i++)
		{
			HitMemberWithMove(Manager, i, Move, 1.0f);
			ApplyMoveModifier(Manager->GetPartyMember(i), MoveId, Move);
		}
		return;
	}
	HitMemberWithMove(Manager, TargetIndex, Move, 1.0f);
	ApplyMoveModifier(Manager->GetPartyMember(TargetIndex), MoveId, Move);
}
void ABattleEnemyCharacter::ShowMoveMessage(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex)
{
	FString MoveName = Move->DisplayName.ToString();
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Move->TargetKind == EBossMoveTargetKind::OneMember)
	{
		if (Target != nullptr)
		{
			Manager->ShowMessage(FString::Printf(TEXT("%s uses %s on %s"), *DisplayName, *MoveName, *Target->DisplayName), FColor::Orange);
			return;
		}
	}
	Manager->ShowMessage(FString::Printf(TEXT("%s uses %s"), *DisplayName, *MoveName), FColor::Orange);
}
float ABattleEnemyCharacter::HitMemberWithMove(ATurnBattleManager* Manager, int32 MemberIndex, const FBossMoveRow* Move, float Multiplier)
{
	return HitMemberWithBonus(Manager, MemberIndex, Move, Multiplier, 0.0f);
}
float ABattleEnemyCharacter::HitMemberWithBonus(ATurnBattleManager* Manager, int32 MemberIndex, const FBossMoveRow* Move, float Multiplier, float BonusDamage)
{
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return 0.0f;
	}
	if (Member->Health->IsDead())
	{
		return 0.0f;
	}
	float Scale = Multiplier;
	if (Manager->IsMemberBlocking(MemberIndex))
	{
		Scale = Scale * BlockMultiplier;
	}
	if (Scale <= 0.0f)
	{
		return 0.0f;
	}
	float BaseDamage = 0.0f;
	if (Move->Power > 0.0f)
	{
		float RawDamage = Stats->GetFinal(ECharacterStat::Damage) * Move->Power * Scale;
		BaseDamage = RawDamage * GetDefenseMitigation(Member);
	}
	float PercentDamage = 0.0f;
	if (Move->Percent > 0.0f)
	{
		PercentDamage = GetPercentBase(Member, Move) * Move->Percent * Scale * GetAttackScale();
	}
	if (Move->OwnMissingPercent > 0.0f)
	{
		float OwnMissingHealth = Health->MaxHealth - Health->CurrentHealth;
		if (OwnMissingHealth > 0.0f)
		{
			PercentDamage = PercentDamage + (OwnMissingHealth * Move->OwnMissingPercent * Scale * GetAttackScale());
		}
	}
	float TotalDamage = BaseDamage + PercentDamage;
	if (BonusDamage > 0.0f)
	{
		TotalDamage = TotalDamage + (BonusDamage * Scale);
	}
	if (TotalDamage <= 0.0f)
	{
		return 0.0f;
	}
	return Member->ApplyHit(TotalDamage, true, this);
}
void ABattleEnemyCharacter::ApplyMoveModifier(AUnshackledCharacterBase* Target, FName MoveId, const FBossMoveRow* Move)
{
	if (Target == nullptr)
	{
		return;
	}
	if (Target->Health->IsDead())
	{
		return;
	}
	if (Move->ModStat == EBossModStat::None)
	{
		return;
	}
	if (Move->ModAmount == 0.0f)
	{
		return;
	}
	FStatModifier NewModifier;
	NewModifier.SourceId = MoveId;
	NewModifier.Stat = ToCharacterStat(Move->ModStat);
	NewModifier.Kind = GetModKind(Move);
	NewModifier.Amount = Move->ModAmount;
	NewModifier.Duration = 0.0f;
	NewModifier.TurnsLeft = 0;
	if (Move->Turns > 0)
	{
		NewModifier.TurnsLeft = Move->Turns + 1;
	}
	NewModifier.Rule = EStackRule::RefreshSameSource;
	NewModifier.MaxStacks = 1;
	if (Move->MaxStacks > 1)
	{
		NewModifier.Rule = EStackRule::StackToMax;
		NewModifier.MaxStacks = Move->MaxStacks;
	}
	Target->Stats->AddModifier(NewModifier);
}
float ABattleEnemyCharacter::GetPercentBase(AUnshackledCharacterBase* Target, const FBossMoveRow* Move) const
{
	if (Move->PercentOf == EBossPercentOf::MaxHealth)
	{
		return Target->Health->MaxHealth;
	}
	if (Move->PercentOf == EBossPercentOf::CurrentHealth)
	{
		return Target->Health->CurrentHealth;
	}
	if (Move->PercentOf == EBossPercentOf::MissingHealth)
	{
		float MissingHealth = Target->Health->MaxHealth - Target->Health->CurrentHealth;
		if (MissingHealth < 0.0f)
		{
			return 0.0f;
		}
		return MissingHealth;
	}
	return 0.0f;
}
float ABattleEnemyCharacter::GetAttackScale() const
{
	if (Stats->BaseDamage <= 0.0f)
	{
		return 1.0f;
	}
	return Stats->GetFinal(ECharacterStat::Damage) / Stats->BaseDamage;
}
float ABattleEnemyCharacter::GetDefenseMitigation(AUnshackledCharacterBase* Target) const
{
	float DefenseValue = Target->Stats->GetFinal(ECharacterStat::Defense);
	float DefenseScale = Target->Stats->DefenseScale;
	if (DefenseScale + DefenseValue <= 0.0f)
	{
		return 1.0f;
	}
	return DefenseScale / (DefenseScale + DefenseValue);
}
int32 ABattleEnemyCharacter::CountUsesThisRound(FName MoveId) const
{
	int32 Count = 0;
	for (int32 i = 0; i < MovesUsedThisRound.Num(); i++)
	{
		if (MovesUsedThisRound[i] == MoveId)
		{
			Count = Count + 1;
		}
	}
	return Count;
}
bool ABattleEnemyCharacter::IsModifierOnlyMove(const FBossMoveRow* Move) const
{
	if (Move->ModStat == EBossModStat::None)
	{
		return false;
	}
	if (Move->Power > 0.0f)
	{
		return false;
	}
	if (Move->Percent > 0.0f)
	{
		return false;
	}
	return true;
}
bool ABattleEnemyCharacter::IsAtMaxStacks(AUnshackledCharacterBase* Target, FName MoveId, const FBossMoveRow* Move) const
{
	int32 MaxStacks = Move->MaxStacks;
	if (MaxStacks < 1)
	{
		MaxStacks = 1;
	}
	int32 Stacks = Target->Stats->GetStacks(MoveId, ToCharacterStat(Move->ModStat), GetModKind(Move));
	return Stacks >= MaxStacks;
}
int32 ABattleEnemyCharacter::PickRandomMemberBelowMaxStacks(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const
{
	TArray<int32> Candidates;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (!IsAtMaxStacks(Member, MoveId, Move))
				{
					Candidates.Add(i);
				}
			}
		}
	}
	if (Candidates.Num() == 0)
	{
		return -1;
	}
	int32 Pick = FMath::RandRange(0, Candidates.Num() - 1);
	return Candidates[Pick];
}
ECharacterStat ABattleEnemyCharacter::ToCharacterStat(EBossModStat ModStat) const
{
	if (ModStat == EBossModStat::Defense)
	{
		return ECharacterStat::Defense;
	}
	if (ModStat == EBossModStat::IncomingDamage)
	{
		return ECharacterStat::IncomingDamage;
	}
	if (ModStat == EBossModStat::Poison)
	{
		return ECharacterStat::Poison;
	}
	return ECharacterStat::Damage;
}
EModifierKind ABattleEnemyCharacter::GetModKind(const FBossMoveRow* Move) const
{
	if (Move->ModStat == EBossModStat::Poison)
	{
		return EModifierKind::Flat;
	}
	return EModifierKind::Percent;
}
float ABattleEnemyCharacter::TakeNextHitMultiplier()
{
	return 1.0f;
}
int32 ABattleEnemyCharacter::CountLivingMembers(const ATurnBattleManager* Manager) const
{
	int32 Living = 0;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				Living = Living + 1;
			}
		}
	}
	return Living;
}
bool ABattleEnemyCharacter::IsMeditationPending(const ATurnBattleManager* Manager) const
{
	if (MeditationRound < 0)
	{
		return false;
	}
	return MeditationRound >= Manager->GetRound();
}
float ABattleEnemyCharacter::ScoreMeditation(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const
{
	if (IsMeditationPending(Manager))
	{
		return 0.0f;
	}
	if (LastMoveId == MoveId)
	{
		return 0.0f;
	}
	if (CountLivingMembers(Manager) < 2)
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
void ABattleEnemyCharacter::CastMeditation(ATurnBattleManager* Manager, FName MoveId)
{
	MeditationMoveId = MoveId;
	MeditationRound = Manager->GetRound() + 1;
	MeditationActionsSeen = 0;
	Manager->ShowMessage(FString::Printf(TEXT("%s foresees a demise: the second member to act next round will be struck"), *DisplayName), FColor::Orange);
}
void ABattleEnemyCharacter::TriggerMeditation(ATurnBattleManager* Manager, int32 MemberIndex)
{
	const FBossMoveRow* Move = Manager->GetBossMove(MeditationMoveId);
	if (Move == nullptr)
	{
		return;
	}
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return;
	}
	float Multiplier = TakeNextHitMultiplier();
	float DamageDealt = HitMemberWithMove(Manager, MemberIndex, Move, Multiplier);
	Manager->ShowMessage(FString::Printf(TEXT("Meditation strikes %s for %d"), *Member->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Orange);
}
int32 ABattleEnemyCharacter::GetPendingCycleCharges(const ATurnBattleManager* Manager) const
{
	if (CycleRound < 0)
	{
		return 0;
	}
	if (CycleRound < Manager->GetRound())
	{
		return 0;
	}
	return CycleCharges;
}
float ABattleEnemyCharacter::ScoreCycle(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const
{
	int32 Charges = GetPendingCycleCharges(Manager);
	int32 MaxCharges = Move->MaxStacks;
	if (MaxCharges < 1)
	{
		MaxCharges = 1;
	}
	if (Charges >= MaxCharges)
	{
		return 0.0f;
	}
	float Score = Move->ScoreBase - (Move->ScoreBonus1 * Charges);
	if (Score < 1.0f)
	{
		Score = 1.0f;
	}
	return Score;
}
void ABattleEnemyCharacter::CastCycle(ATurnBattleManager* Manager, FName MoveId)
{
	int32 Charges = GetPendingCycleCharges(Manager);
	CycleMoveId = MoveId;
	CycleRound = Manager->GetRound() + 1;
	CycleCharges = Charges + 1;
	Manager->ShowMessage(FString::Printf(TEXT("%s readies a rain of poison for after the first member acts"), *DisplayName), FColor::Orange);
}
void ABattleEnemyCharacter::TriggerCycle(ATurnBattleManager* Manager)
{
	const FBossMoveRow* Move = Manager->GetBossMove(CycleMoveId);
	if (Move == nullptr)
	{
		return;
	}
	for (int32 Charge = 0; Charge < CycleCharges; Charge++)
	{
		Manager->ShowMessage(TEXT("Poison rain falls on the party"), FColor::Orange);
		float Multiplier = TakeNextHitMultiplier();
		for (int32 i = 0; i < Manager->GetPartyCount(); i++)
		{
			HitMemberWithMove(Manager, i, Move, Multiplier);
			ApplyMoveModifier(Manager->GetPartyMember(i), CycleMoveId, Move);
		}
	}
}
void ABattleEnemyCharacter::CheckMeditation(ATurnBattleManager* Manager, int32 MemberIndex)
{
	if (MeditationRound != Manager->GetRound())
	{
		return;
	}
	if (MeditationActionsSeen < 1)
	{
		MeditationActionsSeen = MeditationActionsSeen + 1;
		return;
	}
	MeditationRound = -1;
	TriggerMeditation(Manager, MemberIndex);
}
void ABattleEnemyCharacter::CheckCycle(ATurnBattleManager* Manager)
{
	if (CycleRound != Manager->GetRound())
	{
		return;
	}
	TriggerCycle(Manager);
	CycleRound = -1;
	CycleCharges = 0;
}
bool ABattleEnemyCharacter::IsGrailActive(const ATurnBattleManager* Manager) const
{
	if (GrailFirstRound < 0)
	{
		return false;
	}
	if (Manager->GetRound() < GrailFirstRound)
	{
		return false;
	}
	return Manager->GetRound() <= GrailLastRound;
}
float ABattleEnemyCharacter::ScoreGrail(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (GrailLastRound >= Manager->GetRound())
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
void ABattleEnemyCharacter::CastGrail(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	int32 Rounds = Move->Turns;
	if (Rounds < 1)
	{
		Rounds = 1;
	}
	GrailMoveId = MoveId;
	GrailFirstRound = Manager->GetRound() + 1;
	GrailLastRound = Manager->GetRound() + Rounds;
	Manager->ShowMessage(TEXT("A grail of flame hovers over the party"), FColor::Red);
}
void ABattleEnemyCharacter::SpillGrail(ATurnBattleManager* Manager)
{
	const FBossMoveRow* Row = Manager->GetBossMove(GrailMoveId);
	if (Row == nullptr)
	{
		return;
	}
	int32 TargetIndex = Manager->PickRandomLivingMember();
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	float DamageDealt = HitMemberWithMove(Manager, TargetIndex, Row, 1.0f);
	Manager->ShowMessage(FString::Printf(TEXT("The grail spills on %s for %d"), *Target->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Red);
}
bool ABattleEnemyCharacter::IsMaliceArmed(const ATurnBattleManager* Manager) const
{
	if (MaliceRound < 0)
	{
		return false;
	}
	return MaliceRound >= Manager->GetRound();
}
float ABattleEnemyCharacter::ScoreMalice(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const
{
	if (IsMaliceArmed(Manager))
	{
		return 0.0f;
	}
	if (LastMoveId == MoveId)
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
void ABattleEnemyCharacter::CastMalice(ATurnBattleManager* Manager, FName MoveId)
{
	MaliceMoveId = MoveId;
	MaliceRound = Manager->GetRound() + 1;
	Manager->ShowMessage(FString::Printf(TEXT("%s lies in wait: the next help given next round will be punished"), *DisplayName), FColor::Orange);
}
void ABattleEnemyCharacter::StartMaliceWatch(ATurnBattleManager* Manager)
{
	const FBossMoveRow* Row = Manager->GetBossMove(MaliceMoveId);
	if (Row == nullptr)
	{
		return;
	}
	SavedBuffCounts.Empty();
	SavedDivertedHealing.Empty();
	SavedShieldMultipliers.Empty();
	SavedShields.Empty();
	SavedStatsDivert.Empty();
	SavedHealthDivert.Empty();
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		SavedBuffCounts.Add(Member->Stats->DivertedBuffs.Num());
		SavedDivertedHealing.Add(Member->Health->DivertedHealing);
		SavedShieldMultipliers.Add(Member->Health->ShieldMultiplier);
		SavedShields.Add(Member->Health->Shield);
		SavedStatsDivert.Add(Member->Stats->DivertFraction);
		SavedHealthDivert.Add(Member->Health->DivertFraction);
		Member->Stats->DivertFraction = Row->Amount1;
		Member->Health->DivertFraction = Row->Amount1;
		Member->Health->ShieldMultiplier = Member->Health->ShieldMultiplier * (1.0f - Row->Amount1);
	}
	bMaliceWatching = true;
}
bool ABattleEnemyCharacter::FinishMaliceWatch(ATurnBattleManager* Manager)
{
	bMaliceWatching = false;
	bool bCaught = false;
	for (int32 i = 0; i < SavedBuffCounts.Num(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member->Stats->DivertedBuffs.Num() > SavedBuffCounts[i])
		{
			bCaught = true;
		}
		if (Member->Health->DivertedHealing > SavedDivertedHealing[i] + 0.01f)
		{
			bCaught = true;
		}
		if (Member->Health->Shield > SavedShields[i] + 0.01f)
		{
			bCaught = true;
		}
		Member->Stats->DivertedBuffs.SetNum(SavedBuffCounts[i]);
		Member->Health->DivertedHealing = SavedDivertedHealing[i];
		Member->Health->ShieldMultiplier = SavedShieldMultipliers[i];
		Member->Stats->DivertFraction = SavedStatsDivert[i];
		Member->Health->DivertFraction = SavedHealthDivert[i];
	}
	return bCaught;
}
void ABattleEnemyCharacter::TriggerMalice(ATurnBattleManager* Manager, int32 MemberIndex)
{
	const FBossMoveRow* Row = Manager->GetBossMove(MaliceMoveId);
	if (Row == nullptr)
	{
		return;
	}
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return;
	}
	float DamageDealt = HitMemberWithMove(Manager, MemberIndex, Row, GetHitMultiplierFor(Manager, MemberIndex));
	Manager->ShowMessage(FString::Printf(TEXT("Malice! %s leaps in, halves %s's help and mauls them for %d"), *DisplayName, *Member->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Red);
}
