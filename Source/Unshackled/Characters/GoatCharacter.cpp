#include "Characters/GoatCharacter.h"
#include "Characters/BattleSummonCharacter.h"
#include "Characters/PartyMemberCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Battle/BattleMoveRow.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Engine/DataTable.h"
AGoatCharacter::AGoatCharacter()
{
	DisplayName = TEXT("Goat");
}
void AGoatCharacter::OnBattleRoundStart(ATurnBattleManager* Manager)
{
	Super::OnBattleRoundStart(Manager);
	CollectTetherShares(Manager);
	UpdateTether(Manager);
	AddDoubtToParty(Manager);
}
void AGoatCharacter::Die()
{
	ClearTetherDivert();
	TetherMember = nullptr;
	Super::Die();
}
int32 AGoatCharacter::GetDoubtStacks(int32 MemberIndex) const
{
	if (MemberIndex < 0 || MemberIndex >= DoubtStacks.Num())
	{
		return 0;
	}
	return DoubtStacks[MemberIndex];
}
void AGoatCharacter::AddDoubtToParty(ATurnBattleManager* Manager)
{
	int32 PartyCount = Manager->GetPartyCount();
	while (DoubtStacks.Num() < PartyCount)
	{
		DoubtStacks.Add(0);
	}
	for (int32 i = 0; i < PartyCount; i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (DoubtStacks[i] < DoubtMaxStacks)
				{
					DoubtStacks[i] = DoubtStacks[i] + 1;
				}
			}
		}
	}
}
bool AGoatCharacter::DoesMemberMoveFail(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName, EBattleAction Category)
{
	bool bCanFail = false;
	if (Category == EBattleAction::Attack)
	{
		bCanFail = true;
	}
	if (Category == EBattleAction::Special)
	{
		bCanFail = true;
	}
	if (Category == EBattleAction::Ultimate)
	{
		bCanFail = true;
	}
	if (!bCanFail)
	{
		return false;
	}
	float FailChance = GetDoubtStacks(MemberIndex) * DoubtChancePerStack;
	if (FailChance <= 0.0f)
	{
		return false;
	}
	float Roll = FMath::FRand();
	if (Roll >= FailChance)
	{
		return false;
	}
	DoubtStacks[MemberIndex] = 0;
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member != nullptr)
	{
		Manager->ShowMessage(FString::Printf(TEXT("%s is overcome by doubt"), *Member->DisplayName), FColor::Magenta);
	}
	return true;
}
bool AGoatCharacter::IsMemberMoveLocked(const ATurnBattleManager* Manager, int32 MemberIndex, FName MoveId) const
{
	if (CagedMemberIndex < 0)
	{
		return false;
	}
	if (MemberIndex != CagedMemberIndex)
	{
		return false;
	}
	if (MoveId != CagedMoveId)
	{
		return false;
	}
	return Manager->GetRound() <= CageRound;
}
FString AGoatCharacter::GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const
{
	FString Note;
	int32 Stacks = GetDoubtStacks(MemberIndex);
	if (Stacks > 0)
	{
		Note = Note + FString::Printf(TEXT("Doubt %d%% "), FMath::RoundToInt(Stacks * DoubtChancePerStack * 100.0f));
	}
	if (TetherMember != nullptr)
	{
		if (Manager->GetPartyMember(MemberIndex) == TetherMember)
		{
			Note = Note + TEXT("Tether ");
		}
	}
	if (MemberIndex == CagedMemberIndex)
	{
		if (Manager->GetRound() <= CageRound)
		{
			Note = Note + TEXT("Caged");
		}
	}
	return Note;
}
float AGoatCharacter::ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	float BaseScore = Super::ScoreMove(Manager, MoveId, Move);
	if (BaseScore <= 0.0f)
	{
		return 0.0f;
	}
	if (MoveId == FName("Goat_Pummel"))
	{
		return ScorePummel(Manager, Move);
	}
	if (MoveId == FName("Goat_Channel"))
	{
		return ScoreChannel(Manager, Move);
	}
	if (MoveId == FName("Goat_Whispers"))
	{
		return ScoreWhispers(Manager, MoveId, Move);
	}
	if (MoveId == FName("Goat_Evoke"))
	{
		return ScoreEvoke(Manager, Move);
	}
	if (MoveId == FName("Goat_Grasp"))
	{
		return ScoreGrasp(Manager, Move);
	}
	if (MoveId == FName("Goat_Imprison"))
	{
		return ScoreImprison(Manager, Move);
	}
	if (MoveId == FName("Goat_Tether"))
	{
		return ScoreTether(Manager, Move);
	}
	if (MoveId == FName("Goat_Meditation"))
	{
		return ScoreMeditation(Manager, MoveId, Move);
	}
	return BaseScore;
}
float AGoatCharacter::ScorePummel(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	float Score = Move->ScoreBase;
	if (bChannelReady)
	{
		Score = Score + Move->ScoreBonus1;
	}
	int32 TargetIndex = FindLowestHealthMember(Manager);
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target != nullptr)
	{
		if (Target->Health->MaxHealth > 0.0f)
		{
			float HealthFraction = Target->Health->CurrentHealth / Target->Health->MaxHealth;
			if (HealthFraction < Move->Amount1)
			{
				Score = Score + Move->ScoreBonus2;
			}
		}
	}
	return Score;
}
float AGoatCharacter::ScoreChannel(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (bChannelReady)
	{
		return 0.0f;
	}
	if (Manager->GetBossTurnsLeft() > 1)
	{
		return Move->ScoreBase;
	}
	return Move->ScoreBonus1;
}
float AGoatCharacter::ScoreWhispers(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const
{
	int32 TargetIndex = FindHighestDefenseMemberBelowMax(Manager, MoveId, Move);
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return 0.0f;
	}
	int32 Stacks = Target->Stats->GetStacks(MoveId, ECharacterStat::Defense, EModifierKind::Percent);
	if (Stacks <= 0)
	{
		return Move->ScoreBase;
	}
	return Move->ScoreBonus1;
}
float AGoatCharacter::ScoreEvoke(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (GuardClass == nullptr)
	{
		return 0.0f;
	}
	int32 GuardsUp = Manager->CountLivingSummons();
	int32 MaxGuards = FMath::RoundToInt(Move->Amount2);
	if (GuardsUp >= MaxGuards)
	{
		return 0.0f;
	}
	float Score = Move->ScoreBonus1;
	if (GuardsUp == 0)
	{
		Score = Move->ScoreBase;
	}
	if (Health->MaxHealth > 0.0f)
	{
		float HealthFraction = Health->CurrentHealth / Health->MaxHealth;
		if (HealthFraction < 0.5f)
		{
			Score = Score + Move->ScoreBonus2;
		}
	}
	return Score;
}
float AGoatCharacter::ScoreGrasp(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	int32 TargetIndex = FindHighestManaMember(Manager);
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return 0.0f;
	}
	float Score = Move->ScoreBase + (Move->ScoreBonus1 * Target->Stats->CurrentMana);
	if (Health->Shield <= 0.0f)
	{
		Score = Score + Move->ScoreBonus2;
	}
	return Score;
}
bool AGoatCharacter::IsCageWaiting(ATurnBattleManager* Manager) const
{
	if (CagedMemberIndex < 0)
	{
		return false;
	}
	return CageRound > Manager->GetRound();
}
float AGoatCharacter::ScoreImprison(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (IsCageWaiting(Manager))
	{
		return 0.0f;
	}
	int32 TargetIndex = FindImprisonTarget(Manager);
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return 0.0f;
	}
	float Score = Move->ScoreBase;
	if (CanAffordUltimate(Manager, Target))
	{
		Score = Score + Move->ScoreBonus1;
	}
	return Score;
}
float AGoatCharacter::ScoreTether(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (TetherMember != nullptr)
	{
		return 0.0f;
	}
	int32 TargetIndex = FindMostBuffedMember(Manager);
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return 0.0f;
	}
	int32 Buffs = Target->Stats->CountBeneficialModifiers();
	float Score = Move->ScoreBase + (Move->ScoreBonus1 * Buffs);
	if (Score > Move->ScoreBonus2)
	{
		Score = Move->ScoreBonus2;
	}
	return Score;
}
int32 AGoatCharacter::PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	if (MoveId == FName("Goat_Pummel"))
	{
		return FindLowestHealthMember(Manager);
	}
	if (MoveId == FName("Goat_Whispers"))
	{
		return FindHighestDefenseMemberBelowMax(Manager, MoveId, Move);
	}
	if (MoveId == FName("Goat_Grasp"))
	{
		return FindHighestManaMember(Manager);
	}
	if (MoveId == FName("Goat_Imprison"))
	{
		return FindImprisonTarget(Manager);
	}
	if (MoveId == FName("Goat_Tether"))
	{
		return FindMostBuffedMember(Manager);
	}
	return Super::PickMoveTarget(Manager, MoveId, Move);
}
void AGoatCharacter::PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex)
{
	if (MoveId == FName("Goat_Pummel"))
	{
		HitWithChannel(Manager, TargetIndex, Move);
	}
	else if (MoveId == FName("Goat_Channel"))
	{
		CastChannel(Manager, Move);
	}
	else if (MoveId == FName("Goat_Evoke"))
	{
		CastEvoke(Manager, Move);
	}
	else if (MoveId == FName("Goat_Grasp"))
	{
		CastGrasp(Manager, Move, TargetIndex);
	}
	else if (MoveId == FName("Goat_Imprison"))
	{
		CastImprison(Manager, Move, TargetIndex);
	}
	else if (MoveId == FName("Goat_Tether"))
	{
		CastTether(Manager, MoveId, Move, TargetIndex);
	}
	else if (MoveId == FName("Goat_Meditation"))
	{
		CastMeditation(Manager, MoveId);
	}
	else
	{
		Super::PerformMove(Manager, MoveId, Move, TargetIndex);
	}
}
int32 AGoatCharacter::FindLowestHealthMember(ATurnBattleManager* Manager) const
{
	int32 BestIndex = -1;
	float BestFraction = 2.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (Member->Health->MaxHealth > 0.0f)
				{
					float HealthFraction = Member->Health->CurrentHealth / Member->Health->MaxHealth;
					if (HealthFraction < BestFraction)
					{
						BestFraction = HealthFraction;
						BestIndex = i;
					}
				}
			}
		}
	}
	return BestIndex;
}
int32 AGoatCharacter::FindHighestManaMember(ATurnBattleManager* Manager) const
{
	int32 BestIndex = -1;
	float BestMana = -1.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (Member->Stats->CurrentMana > BestMana)
				{
					BestMana = Member->Stats->CurrentMana;
					BestIndex = i;
				}
			}
		}
	}
	return BestIndex;
}
int32 AGoatCharacter::FindHighestDefenseMemberBelowMax(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const
{
	int32 BestIndex = -1;
	float BestDefense = -1.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (!IsAtMaxStacks(Member, MoveId, Move))
				{
					float DefenseValue = Member->Stats->GetFinal(ECharacterStat::Defense);
					if (DefenseValue > BestDefense)
					{
						BestDefense = DefenseValue;
						BestIndex = i;
					}
				}
			}
		}
	}
	return BestIndex;
}
int32 AGoatCharacter::FindMostBuffedMember(ATurnBattleManager* Manager) const
{
	int32 BestIndex = -1;
	int32 BestBuffs = -1;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				int32 Buffs = Member->Stats->CountBeneficialModifiers();
				if (Buffs > BestBuffs)
				{
					BestBuffs = Buffs;
					BestIndex = i;
				}
			}
		}
	}
	return BestIndex;
}
int32 AGoatCharacter::FindImprisonTarget(ATurnBattleManager* Manager) const
{
	int32 BestIndex = -1;
	float BestMana = -1.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (GetLockableMoves(Manager, Member).Num() > 0)
				{
					if (Member->Stats->CurrentMana > BestMana)
					{
						BestMana = Member->Stats->CurrentMana;
						BestIndex = i;
					}
				}
			}
		}
	}
	return BestIndex;
}
TArray<FName> AGoatCharacter::GetLockableMoves(const ATurnBattleManager* Manager, APartyMemberCharacter* Member) const
{
	TArray<FName> Lockable;
	if (Manager->MoveTable == nullptr)
	{
		return Lockable;
	}
	TArray<FName> AllNames = Manager->MoveTable->GetRowNames();
	for (int32 i = 0; i < AllNames.Num(); i++)
	{
		const FBattleMoveRow* Row = Manager->GetMove(AllNames[i]);
		if (Row != nullptr)
		{
			if (Row->MemberId == Member->StatsRowName)
			{
				if (Row->Category != EBattleAction::Attack)
				{
					if (Manager->IsMoveUnlocked(Row))
					{
						Lockable.Add(AllNames[i]);
					}
				}
			}
		}
	}
	return Lockable;
}
bool AGoatCharacter::CanAffordUltimate(ATurnBattleManager* Manager, APartyMemberCharacter* Member) const
{
	TArray<FName> Lockable = GetLockableMoves(Manager, Member);
	for (int32 i = 0; i < Lockable.Num(); i++)
	{
		const FBattleMoveRow* Row = Manager->GetMove(Lockable[i]);
		if (Row != nullptr)
		{
			if (Row->Category == EBattleAction::Ultimate)
			{
				if (Member->Stats->CurrentMana >= Row->ManaCost)
				{
					return true;
				}
			}
		}
	}
	return false;
}
float AGoatCharacter::TakeNextHitMultiplier()
{
	if (!bChannelReady)
	{
		return 1.0f;
	}
	bChannelReady = false;
	return ChannelMultiplier;
}
float AGoatCharacter::HitWithChannel(ATurnBattleManager* Manager, int32 MemberIndex, const FBossMoveRow* Move)
{
	float Multiplier = TakeNextHitMultiplier();
	return HitMemberWithMove(Manager, MemberIndex, Move, Multiplier);
}
void AGoatCharacter::CastChannel(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	bChannelReady = true;
	ChannelMultiplier = Move->Amount1;
	Manager->ShowMessage(TEXT("The Goat gathers its power"), FColor::Orange);
}
void AGoatCharacter::CastEvoke(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	float GuardHealth = Health->CurrentHealth * Move->Amount1;
	Manager->SpawnSummon(GuardClass, GuardHealth);
}
void AGoatCharacter::CastGrasp(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex)
{
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	if (Target->Health->IsDead())
	{
		return;
	}
	float ManaDrained = Target->Stats->CurrentMana * Move->Amount1;
	if (ManaDrained > 0.0f)
	{
		Target->Stats->SpendMana(ManaDrained);
	}
	float ShieldAmount = ManaDrained + (Health->MaxHealth * Move->Amount2);
	Health->AddShield(ShieldAmount);
	Manager->ShowMessage(FString::Printf(TEXT("The Goat drains %d mana from %s and gains a %d shield"), FMath::RoundToInt(ManaDrained), *Target->DisplayName, FMath::RoundToInt(ShieldAmount)), FColor::Orange);
}
void AGoatCharacter::CastImprison(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex)
{
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	TArray<FName> Lockable = GetLockableMoves(Manager, Target);
	if (Lockable.Num() == 0)
	{
		Manager->ShowMessage(TEXT("The cage finds nothing to hold"), FColor::Orange);
		return;
	}
	int32 Pick = FMath::RandRange(0, Lockable.Num() - 1);
	CagedMemberIndex = TargetIndex;
	CagedMoveId = Lockable[Pick];
	int32 Rounds = Move->Turns;
	if (Rounds < 1)
	{
		Rounds = 1;
	}
	CageRound = Manager->GetRound() + Rounds;
	FString MoveName = CagedMoveId.ToString();
	const FBattleMoveRow* Row = Manager->GetMove(CagedMoveId);
	if (Row != nullptr)
	{
		MoveName = Row->DisplayName.ToString();
	}
	Manager->ShowMessage(FString::Printf(TEXT("%s's %s is caged"), *Target->DisplayName, *MoveName), FColor::Orange);
}
void AGoatCharacter::CastTether(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex)
{
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	if (Target->Health->IsDead())
	{
		return;
	}
	TetherMember = Target;
	TetherMoveId = MoveId;
	TetherBreakRound = Manager->GetRound() + Move->Turns + 1;
	Target->Health->DivertFraction = Move->Amount1;
	Target->Stats->DivertFraction = Move->Amount1;
	Manager->ShowMessage(FString::Printf(TEXT("%s is tethered to the Goat"), *Target->DisplayName), FColor::Orange);
}
void AGoatCharacter::CollectTetherShares(ATurnBattleManager* Manager)
{
	if (TetherMember == nullptr)
	{
		return;
	}
	float HealingTaken = TetherMember->Health->DivertedHealing;
	TetherMember->Health->DivertedHealing = 0.0f;
	if (HealingTaken > 0.0f)
	{
		Health->Heal(HealingTaken);
		Manager->ShowMessage(FString::Printf(TEXT("The tether feeds the Goat %d health"), FMath::RoundToInt(HealingTaken)), FColor::Orange);
	}
	int32 BuffCount = TetherMember->Stats->DivertedBuffs.Num();
	for (int32 i = 0; i < BuffCount; i++)
	{
		Stats->AddModifier(TetherMember->Stats->DivertedBuffs[i]);
	}
	TetherMember->Stats->DivertedBuffs.Empty();
	if (BuffCount > 0)
	{
		Manager->ShowMessage(FString::Printf(TEXT("The tether steals %d buff(s) for the Goat"), BuffCount), FColor::Orange);
	}
}
void AGoatCharacter::UpdateTether(ATurnBattleManager* Manager)
{
	if (TetherMember == nullptr)
	{
		return;
	}
	if (TetherMember->Health->IsDead())
	{
		BreakTether(Manager, false);
		return;
	}
	if (Manager->GetRound() >= TetherBreakRound)
	{
		BreakTether(Manager, true);
	}
}
void AGoatCharacter::ClearTetherDivert()
{
	if (TetherMember == nullptr)
	{
		return;
	}
	TetherMember->Health->DivertFraction = 0.0f;
	TetherMember->Health->DivertedHealing = 0.0f;
	TetherMember->Stats->DivertFraction = 0.0f;
	TetherMember->Stats->DivertedBuffs.Empty();
}
void AGoatCharacter::BreakTether(ATurnBattleManager* Manager, bool bDealDamage)
{
	CollectTetherShares(Manager);
	ClearTetherDivert();
	APartyMemberCharacter* Broken = TetherMember;
	TetherMember = nullptr;
	if (!bDealDamage)
	{
		return;
	}
	const FBossMoveRow* Row = Manager->GetBossMove(TetherMoveId);
	if (Row == nullptr)
	{
		return;
	}
	int32 MemberIndex = Manager->FindPartyIndex(Broken);
	float DamageDealt = HitWithChannel(Manager, MemberIndex, Row);
	Manager->ShowMessage(FString::Printf(TEXT("The tether snaps on %s for %d"), *Broken->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Orange);
}