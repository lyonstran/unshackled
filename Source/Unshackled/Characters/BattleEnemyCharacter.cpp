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
bool ABattleEnemyCharacter::DoesMemberMoveFail(ATurnBattleManager* Manager, int32 MemberIndex, EBattleAction Category)
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
	float TotalDamage = BaseDamage + PercentDamage;
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
	NewModifier.Kind = EModifierKind::Percent;
	NewModifier.Amount = Move->ModAmount;
	NewModifier.Duration = 0.0f;
	NewModifier.TurnsLeft = Move->Turns;
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
	int32 Stacks = Target->Stats->GetStacks(MoveId, ToCharacterStat(Move->ModStat), EModifierKind::Percent);
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
	return ECharacterStat::Damage;
}