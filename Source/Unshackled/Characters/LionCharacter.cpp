#include "Characters/LionCharacter.h"
#include "Characters/PartyMemberCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Battle/BattleMoveRow.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Engine/DataTable.h"
ALionCharacter::ALionCharacter()
{
	DisplayName = TEXT("Lion");
}
void ALionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateSubjugationEffects();
	CheckPride();
}
void ALionCharacter::OnBattleRoundStart(ATurnBattleManager* Manager)
{
	Super::OnBattleRoundStart(Manager);
	BattleManager = Manager;
	EnsureMemberArrays(Manager);
	PlayerPhaseDamage = 0.0f;
	TickRest(Manager);
	TickChallengeMana(Manager);
}
bool ALionCharacter::DoesMemberMoveFail(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName, EBattleAction Category)
{
	bProwlFailedThisAction = false;
	if (!IsProwling(Manager))
	{
		return false;
	}
	if (!DoesEntryTargetLion(Manager, EntryName))
	{
		return false;
	}
	if (FMath::FRand() < ProwlFailChance)
	{
		bProwlFailedThisAction = true;
		return true;
	}
	return false;
}
void ALionCharacter::BeforeMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName)
{
	Super::BeforeMemberAction(Manager, MemberIndex, EntryName);
	BattleManager = Manager;
	EnsureMemberArrays(Manager);
	RecordActionStart();
	if (IsMarked(MemberIndex))
	{
		if (DoesEntryTargetLion(Manager, EntryName))
		{
			if (!bProwlFailedThisAction)
			{
				SovereignMarks[MemberIndex] = false;
				TriggerSovereignty(Manager, MemberIndex);
			}
		}
	}
}
void ALionCharacter::AfterMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName)
{
	RecordActionDamage(Manager, MemberIndex);
	bSovereignMitigation = false;
	bProwlFailedThisAction = false;
	Super::AfterMemberAction(Manager, MemberIndex, EntryName);
}
FString ALionCharacter::GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const
{
	FString Note;
	int32 Stacks = GetSubjugation(MemberIndex);
	if (Stacks > 0)
	{
		Note = Note + FString::Printf(TEXT("Subj %d "), Stacks);
	}
	if (IsMarked(MemberIndex))
	{
		Note = Note + TEXT("Sovereignty ");
	}
	if (IsHunted(Manager, MemberIndex))
	{
		Note = Note + TEXT("Hunted ");
	}
	if (IsChallenged(Manager, MemberIndex))
	{
		Note = Note + TEXT("Challenged ");
	}
	return Note;
}
FString ALionCharacter::GetStatusNote(const ATurnBattleManager* Manager) const
{
	FString Note = Super::GetStatusNote(Manager);
	if (IsSleeping())
	{
		int32 RoundsLeft = RestLastRound - Manager->GetRound() + 1;
		Note = Note + FString::Printf(TEXT("Asleep %d "), RoundsLeft);
	}
	if (IsProwling(Manager))
	{
		int32 RoundsLeft = ProwlLastRound - Manager->GetRound() + 1;
		Note = Note + FString::Printf(TEXT("Prowl %d "), RoundsLeft);
	}
	return Note;
}
void ALionCharacter::EnsureMemberArrays(const ATurnBattleManager* Manager)
{
	if (Manager == nullptr)
	{
		return;
	}
	while (SubjugationStacks.Num() < Manager->GetPartyCount())
	{
		SubjugationStacks.Add(0);
		AppliedSubjugation.Add(0);
		SovereignMarks.Add(false);
	}
}
int32 ALionCharacter::GetSubjugation(int32 MemberIndex) const
{
	if (MemberIndex < 0 || MemberIndex >= SubjugationStacks.Num())
	{
		return 0;
	}
	return SubjugationStacks[MemberIndex];
}
int32 ALionCharacter::GetTotalSubjugation() const
{
	int32 Total = 0;
	for (int32 i = 0; i < SubjugationStacks.Num(); i++)
	{
		Total = Total + SubjugationStacks[i];
	}
	return Total;
}
void ALionCharacter::AddSubjugation(ATurnBattleManager* Manager, int32 MemberIndex, int32 Amount)
{
	EnsureMemberArrays(Manager);
	if (MemberIndex < 0 || MemberIndex >= SubjugationStacks.Num())
	{
		return;
	}
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Lion_Subjugation"));
	if (Row == nullptr)
	{
		return;
	}
	int32 NewStacks = SubjugationStacks[MemberIndex] + Amount;
	if (NewStacks > Row->MaxStacks)
	{
		NewStacks = Row->MaxStacks;
	}
	if (NewStacks == SubjugationStacks[MemberIndex])
	{
		return;
	}
	SubjugationStacks[MemberIndex] = NewStacks;
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member != nullptr)
	{
		Manager->ShowMessage(FString::Printf(TEXT("%s is subjugated (%d)"), *Member->DisplayName, NewStacks), FColor::Orange);
	}
}
void ALionCharacter::UpdateSubjugationEffects()
{
	if (BattleManager == nullptr)
	{
		return;
	}
	if (Health->IsDead())
	{
		return;
	}
	const FBossMoveRow* Row = BattleManager->GetBossMove(FName("Lion_Subjugation"));
	if (Row == nullptr)
	{
		return;
	}
	for (int32 i = 0; i < SubjugationStacks.Num(); i++)
	{
		UpdateMemberSubjugation(i, Row);
	}
	UpdateLionDamage(Row);
}
void ALionCharacter::UpdateMemberSubjugation(int32 MemberIndex, const FBossMoveRow* Row)
{
	int32 Stacks = SubjugationStacks[MemberIndex];
	if (Stacks <= 0)
	{
		return;
	}
	APartyMemberCharacter* Member = BattleManager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return;
	}
	bool bMissing = false;
	if (Member->Stats->GetStacks(FName("Lion_Subjugation"), ECharacterStat::Defense, EModifierKind::Percent) == 0)
	{
		bMissing = true;
	}
	if (!bMissing)
	{
		if (AppliedSubjugation[MemberIndex] == Stacks)
		{
			return;
		}
	}
	FStatModifier DefenseLoss;
	DefenseLoss.SourceId = FName("Lion_Subjugation");
	DefenseLoss.Stat = ECharacterStat::Defense;
	DefenseLoss.Kind = EModifierKind::Percent;
	DefenseLoss.Amount = Row->ModAmount * Stacks;
	DefenseLoss.Duration = 0.0f;
	DefenseLoss.TurnsLeft = 0;
	DefenseLoss.Rule = EStackRule::RefreshSameSource;
	Member->Stats->AddModifier(DefenseLoss);
	AppliedSubjugation[MemberIndex] = Stacks;
}
void ALionCharacter::UpdateLionDamage(const FBossMoveRow* Row)
{
	int32 Total = GetTotalSubjugation();
	if (Total == AppliedLionDamageStacks)
	{
		return;
	}
	AppliedLionDamageStacks = Total;
	FStatModifier Dominance;
	Dominance.SourceId = FName("Lion_Subjugation");
	Dominance.Stat = ECharacterStat::Damage;
	Dominance.Kind = EModifierKind::Flat;
	Dominance.Amount = Row->Amount1 * Total;
	Dominance.Duration = 0.0f;
	Dominance.TurnsLeft = 0;
	Dominance.Rule = EStackRule::RefreshSameSource;
	Stats->AddModifier(Dominance);
}
void ALionCharacter::CheckPride()
{
	if (bPrideDone)
	{
		return;
	}
	if (BattleManager == nullptr)
	{
		return;
	}
	if (Health->IsDead())
	{
		return;
	}
	const FBossMoveRow* Row = BattleManager->GetBossMove(FName("Lion_Pride"));
	if (Row == nullptr)
	{
		return;
	}
	if (Health->CurrentHealth > Health->MaxHealth * Row->Amount1)
	{
		return;
	}
	bPrideDone = true;
	Health->AddShield(Health->MaxHealth * Row->Amount2);
	BattleManager->ShowMessage(TEXT("The Lion's pride will not break: it gains a great shield"), FColor::Orange);
}
bool ALionCharacter::DoesEntryTargetLion(const ATurnBattleManager* Manager, FName EntryName) const
{
	const FBattleMoveRow* Move = Manager->GetMove(EntryName);
	if (Move == nullptr)
	{
		return false;
	}
	if (Move->TargetKind == EBattleTargetKind::Enemy)
	{
		return true;
	}
	if (Move->TargetKind == EBattleTargetKind::TwoEnemies)
	{
		return true;
	}
	if (Move->TargetKind == EBattleTargetKind::AllEnemies)
	{
		return true;
	}
	return false;
}
float ALionCharacter::GetHealthPercent(APartyMemberCharacter* Member) const
{
	if (Member->Health->MaxHealth <= 0.0f)
	{
		return 100.0f;
	}
	return (Member->Health->CurrentHealth / Member->Health->MaxHealth) * 100.0f;
}
bool ALionCharacter::CanExecute(const ATurnBattleManager* Manager, int32 MemberIndex) const
{
	int32 Stacks = GetSubjugation(MemberIndex);
	if (Stacks <= 0)
	{
		return false;
	}
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return false;
	}
	if (Member->Health->IsDead())
	{
		return false;
	}
	return GetHealthPercent(Member) <= Stacks;
}
bool ALionCharacter::IsNearExecute(const ATurnBattleManager* Manager, int32 MemberIndex, float Margin) const
{
	int32 Stacks = GetSubjugation(MemberIndex);
	if (Stacks <= 0)
	{
		return false;
	}
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return false;
	}
	if (Member->Health->IsDead())
	{
		return false;
	}
	return GetHealthPercent(Member) <= Stacks + Margin;
}
bool ALionCharacter::IsMarked(int32 MemberIndex) const
{
	if (MemberIndex < 0 || MemberIndex >= SovereignMarks.Num())
	{
		return false;
	}
	return SovereignMarks[MemberIndex];
}
bool ALionCharacter::IsHuntActive(const ATurnBattleManager* Manager) const
{
	if (HuntedIndex < 0)
	{
		return false;
	}
	if (Manager->GetRound() > HuntLastRound)
	{
		return false;
	}
	APartyMemberCharacter* Member = Manager->GetPartyMember(HuntedIndex);
	if (Member == nullptr)
	{
		return false;
	}
	return !Member->Health->IsDead();
}
bool ALionCharacter::IsHunted(const ATurnBattleManager* Manager, int32 MemberIndex) const
{
	if (MemberIndex != HuntedIndex)
	{
		return false;
	}
	return IsHuntActive(Manager);
}
float ALionCharacter::GetHuntMultiplier(ATurnBattleManager* Manager, int32 MemberIndex) const
{
	if (!IsHunted(Manager, MemberIndex))
	{
		return 1.0f;
	}
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Lion_Hunt"));
	if (Row == nullptr)
	{
		return 1.0f;
	}
	return 1.0f + Row->Amount1;
}
bool ALionCharacter::IsProwling(const ATurnBattleManager* Manager) const
{
	if (ProwlLastRound < 0)
	{
		return false;
	}
	return Manager->GetRound() <= ProwlLastRound;
}
bool ALionCharacter::IsChallengeActive(const ATurnBattleManager* Manager) const
{
	if (ChallengedIndex < 0)
	{
		return false;
	}
	if (Manager->GetRound() > ChallengeLastRound)
	{
		return false;
	}
	APartyMemberCharacter* Member = Manager->GetPartyMember(ChallengedIndex);
	if (Member == nullptr)
	{
		return false;
	}
	return !Member->Health->IsDead();
}
bool ALionCharacter::IsChallenged(const ATurnBattleManager* Manager, int32 MemberIndex) const
{
	if (MemberIndex != ChallengedIndex)
	{
		return false;
	}
	return IsChallengeActive(Manager);
}
bool ALionCharacter::IsSleeping() const
{
	return !bAwake;
}
bool ALionCharacter::IsRoarOnEveryone(ATurnBattleManager* Manager) const
{
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (Member->Stats->GetStacks(FName("Lion_Roar"), ECharacterStat::Defense, EModifierKind::Percent) == 0)
				{
					return false;
				}
			}
		}
	}
	return true;
}
float ALionCharacter::GetCryHeal(const FBossMoveRow* Move) const
{
	float Strength = Move->Amount1 - (Move->Amount2 * CryUses);
	if (Strength < Move->Amount3)
	{
		Strength = Move->Amount3;
	}
	return PlayerPhaseDamage * Strength;
}
float ALionCharacter::FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker)
{
	float Filtered = Super::FilterIncomingDamage(RawDamage, bTrueDamage, Attacker);
	if (!bSovereignMitigation)
	{
		return Filtered;
	}
	return Filtered * (1.0f - SovereignMitigation);
}
float ALionCharacter::ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	if (IsSleeping())
	{
		if (MoveId == FName("Lion_Slumber"))
		{
			return 100.0f;
		}
		return 0.0f;
	}
	float BaseScore = Super::ScoreMove(Manager, MoveId, Move);
	if (BaseScore <= 0.0f)
	{
		return 0.0f;
	}
	if (MoveId == FName("Lion_Supremity"))
	{
		return ScoreSupremity(Manager, Move);
	}
	if (MoveId == FName("Lion_Sovereignty"))
	{
		return ScoreSovereignty(Manager, Move);
	}
	if (MoveId == FName("Lion_Hunt"))
	{
		return ScoreHunt(Manager, Move);
	}
	if (MoveId == FName("Lion_Prowl"))
	{
		return ScoreProwl(Manager, Move);
	}
	if (MoveId == FName("Lion_Cry"))
	{
		return ScoreCry(Move);
	}
	if (MoveId == FName("Lion_Challenge"))
	{
		return ScoreChallenge(Manager, Move);
	}
	if (MoveId == FName("Lion_Roar"))
	{
		return ScoreRoar(Manager, Move);
	}
	if (MoveId == FName("Lion_Rest"))
	{
		return ScoreRest(Move);
	}
	if (MoveId == FName("Lion_Malice"))
	{
		return ScoreMalice(Manager, MoveId, Move);
	}
	return BaseScore;
}
float ALionCharacter::ScoreSupremity(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (FindExecuteTarget(Manager) >= 0)
	{
		return Move->ScoreBonus2;
	}
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		if (IsNearExecute(Manager, i, Move->Amount1))
		{
			return Move->ScoreBase + Move->ScoreBonus1;
		}
	}
	return Move->ScoreBase;
}
float ALionCharacter::ScoreSovereignty(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (FindSovereigntyTarget(Manager) < 0)
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
float ALionCharacter::ScoreHunt(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (IsHuntActive(Manager))
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
float ALionCharacter::ScoreProwl(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (IsProwling(Manager))
	{
		return 0.0f;
	}
	float Score = Move->ScoreBase;
	if (Health->CurrentHealth < Health->MaxHealth * Move->Amount2)
	{
		Score = Score + Move->ScoreBonus1;
	}
	return Score;
}
float ALionCharacter::ScoreCry(const FBossMoveRow* Move) const
{
	if (Health->MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	float HealFraction = GetCryHeal(Move) / Health->MaxHealth;
	if (HealFraction < Move->ScoreBonus2)
	{
		return 0.0f;
	}
	return Move->ScoreBase + (Move->ScoreBonus1 * HealFraction * 100.0f);
}
float ALionCharacter::ScoreChallenge(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (IsChallengeActive(Manager))
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
float ALionCharacter::ScoreRoar(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (IsRoarOnEveryone(Manager))
	{
		return Move->Amount2;
	}
	int32 MostStacks = 0;
	for (int32 i = 0; i < SubjugationStacks.Num(); i++)
	{
		if (SubjugationStacks[i] > MostStacks)
		{
			MostStacks = SubjugationStacks[i];
		}
	}
	float Score = Move->ScoreBase + (Move->ScoreBonus1 * MostStacks);
	if (Score > Move->ScoreBonus2)
	{
		Score = Move->ScoreBonus2;
	}
	return Score;
}
float ALionCharacter::ScoreRest(const FBossMoveRow* Move) const
{
	if (bRestUsed)
	{
		return 0.0f;
	}
	if (Health->CurrentHealth > Health->MaxHealth * Move->Amount1)
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
int32 ALionCharacter::FindExecuteTarget(ATurnBattleManager* Manager) const
{
	int32 BestIndex = -1;
	float BestPercent = 101.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		if (CanExecute(Manager, i))
		{
			float HealthPercent = GetHealthPercent(Manager->GetPartyMember(i));
			if (HealthPercent < BestPercent)
			{
				BestPercent = HealthPercent;
				BestIndex = i;
			}
		}
	}
	return BestIndex;
}
int32 ALionCharacter::FindSupremityTarget(ATurnBattleManager* Manager) const
{
	int32 ExecuteIndex = FindExecuteTarget(Manager);
	if (ExecuteIndex >= 0)
	{
		return ExecuteIndex;
	}
	if (IsHuntActive(Manager))
	{
		return HuntedIndex;
	}
	int32 BestIndex = -1;
	float BestGap = 1000.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				float Gap = GetHealthPercent(Member) - GetSubjugation(i);
				if (Gap < BestGap)
				{
					BestGap = Gap;
					BestIndex = i;
				}
			}
		}
	}
	return BestIndex;
}
int32 ALionCharacter::FindSovereigntyTarget(ATurnBattleManager* Manager) const
{
	int32 BestIndex = -1;
	float BestDamage = -1.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (!IsMarked(i))
				{
					float DamageValue = Member->Stats->GetFinal(ECharacterStat::Damage);
					if (DamageValue > BestDamage)
					{
						BestDamage = DamageValue;
						BestIndex = i;
					}
				}
			}
		}
	}
	return BestIndex;
}
int32 ALionCharacter::FindMostSubjugatedMember(ATurnBattleManager* Manager) const
{
	int32 BestIndex = -1;
	int32 BestStacks = -1;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (GetSubjugation(i) > BestStacks)
				{
					BestStacks = GetSubjugation(i);
					BestIndex = i;
				}
			}
		}
	}
	return BestIndex;
}
int32 ALionCharacter::FindLeastSubjugatedMember(ATurnBattleManager* Manager) const
{
	int32 BestIndex = -1;
	int32 BestStacks = 1000;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (GetSubjugation(i) < BestStacks)
				{
					BestStacks = GetSubjugation(i);
					BestIndex = i;
				}
			}
		}
	}
	return BestIndex;
}
int32 ALionCharacter::PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	if (MoveId == FName("Lion_Supremity"))
	{
		return FindSupremityTarget(Manager);
	}
	if (MoveId == FName("Lion_Sovereignty"))
	{
		return FindSovereigntyTarget(Manager);
	}
	if (MoveId == FName("Lion_Hunt"))
	{
		return FindMostSubjugatedMember(Manager);
	}
	if (MoveId == FName("Lion_Challenge"))
	{
		return FindLeastSubjugatedMember(Manager);
	}
	return Super::PickMoveTarget(Manager, MoveId, Move);
}
void ALionCharacter::PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex)
{
	EnsureMemberArrays(Manager);
	if (MoveId == FName("Lion_Supremity"))
	{
		CastSupremity(Manager, Move, TargetIndex);
	}
	else if (MoveId == FName("Lion_Sovereignty"))
	{
		CastSovereignty(Manager, TargetIndex);
	}
	else if (MoveId == FName("Lion_Hunt"))
	{
		CastHunt(Manager, Move, TargetIndex);
	}
	else if (MoveId == FName("Lion_Prowl"))
	{
		CastProwl(Manager, Move);
	}
	else if (MoveId == FName("Lion_Cry"))
	{
		CastCry(Manager, Move);
	}
	else if (MoveId == FName("Lion_Challenge"))
	{
		CastChallenge(Manager, MoveId, Move, TargetIndex);
	}
	else if (MoveId == FName("Lion_Roar"))
	{
		CastRoar(Manager, MoveId, Move);
	}
	else if (MoveId == FName("Lion_Rest"))
	{
		CastRest(Manager, MoveId, Move);
	}
	else if (MoveId == FName("Lion_Malice"))
	{
		CastMalice(Manager, MoveId);
	}
	else if (MoveId == FName("Lion_Slumber"))
	{
		Slumber(Manager);
	}
	else
	{
		Super::PerformMove(Manager, MoveId, Move, TargetIndex);
	}
}
void ALionCharacter::CastSupremity(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex)
{
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	if (CanExecute(Manager, TargetIndex))
	{
		ExecuteMember(Manager, TargetIndex);
		return;
	}
	float DamageDealt = HitMemberWithMove(Manager, TargetIndex, Move, GetHuntMultiplier(Manager, TargetIndex));
	Manager->ShowMessage(FString::Printf(TEXT("The Lion bites %s for %d"), *Target->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Orange);
}
void ALionCharacter::ExecuteMember(ATurnBattleManager* Manager, int32 TargetIndex)
{
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	bool bKilled = Target->TakeExecute(this);
	if (!bKilled)
	{
		Manager->ShowMessage(FString::Printf(TEXT("%s refuses to fall to the Lion's jaws"), *Target->DisplayName), FColor::Cyan);
		return;
	}
	Manager->ShowMessage(FString::Printf(TEXT("The Lion EXECUTES %s and hunts again!"), *Target->DisplayName), FColor::Red);
	Manager->AddBossTurns(1);
}
void ALionCharacter::CastSovereignty(ATurnBattleManager* Manager, int32 TargetIndex)
{
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	if (TargetIndex >= SovereignMarks.Num())
	{
		return;
	}
	SovereignMarks[TargetIndex] = true;
	Manager->ShowMessage(FString::Printf(TEXT("The Lion claims sovereignty over %s: their next attack on it will be punished"), *Target->DisplayName), FColor::Orange);
}
void ALionCharacter::CastHunt(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex)
{
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	HuntedIndex = TargetIndex;
	HuntLastRound = Manager->GetRound() + Move->Turns;
	Manager->ShowMessage(FString::Printf(TEXT("The Lion hunts %s"), *Target->DisplayName), FColor::Orange);
}
void ALionCharacter::CastProwl(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	ProwlLastRound = Manager->GetRound() + Move->Turns;
	ProwlFailChance = Move->Amount1;
	Manager->ShowMessage(TEXT("The Lion prowls with bloodlust: moves aimed at it may fail"), FColor::Orange);
}
void ALionCharacter::CastCry(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	float HealAmount = GetCryHeal(Move);
	CryUses = CryUses + 1;
	float HealthBefore = Health->CurrentHealth;
	Health->Heal(HealAmount);
	float Healed = Health->CurrentHealth - HealthBefore;
	Manager->ShowMessage(FString::Printf(TEXT("The Lion cries out and recovers %d health"), FMath::RoundToInt(Healed)), FColor::Orange);
}
void ALionCharacter::CastChallenge(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex)
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
	ChallengedIndex = TargetIndex;
	ChallengeLastRound = Manager->GetRound() + Move->Turns;
	AddChallengeModifier(Target, MoveId, ECharacterStat::Damage, Move->ModAmount, Move->Turns);
	AddChallengeModifier(Target, MoveId, ECharacterStat::Defense, Move->Amount1, Move->Turns);
	Manager->ShowMessage(FString::Printf(TEXT("The Lion challenges %s: more power and mana, less defense"), *Target->DisplayName), FColor::Orange);
}
void ALionCharacter::AddChallengeModifier(APartyMemberCharacter* Member, FName MoveId, ECharacterStat Stat, float Amount, int32 Turns)
{
	FStatModifier Dare;
	Dare.SourceId = MoveId;
	Dare.Stat = Stat;
	Dare.Kind = EModifierKind::Percent;
	Dare.Amount = Amount;
	Dare.Duration = 0.0f;
	Dare.TurnsLeft = Turns + 1;
	Dare.Rule = EStackRule::RefreshSameSource;
	Member->Stats->AddModifier(Dare);
}
void ALionCharacter::CastRoar(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				FStatModifier Fear;
				Fear.SourceId = MoveId;
				Fear.Stat = ECharacterStat::Defense;
				Fear.Kind = EModifierKind::Percent;
				Fear.Amount = Move->ModAmount + (Move->Amount1 * GetSubjugation(i));
				Fear.Duration = 0.0f;
				Fear.TurnsLeft = Move->Turns + 1;
				Fear.Rule = EStackRule::RefreshSameSource;
				Member->Stats->AddModifier(Fear);
			}
		}
	}
	Manager->ShowMessage(TEXT("The Lion roars: the party's defenses crumble"), FColor::Orange);
}
void ALionCharacter::CastRest(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	bRestUsed = true;
	bAwake = false;
	RestLastRound = Manager->GetRound() + Move->Turns;
	FStatModifier Slumbering;
	Slumbering.SourceId = MoveId;
	Slumbering.Stat = ECharacterStat::IncomingDamage;
	Slumbering.Kind = EModifierKind::Percent;
	Slumbering.Amount = Move->ModAmount;
	Slumbering.Duration = 0.0f;
	Slumbering.TurnsLeft = 0;
	Slumbering.Rule = EStackRule::RefreshSameSource;
	Stats->AddModifier(Slumbering);
	Manager->ShowMessage(TEXT("The Lion lies down to rest: it takes far less damage while it sleeps"), FColor::Orange);
}
void ALionCharacter::Slumber(ATurnBattleManager* Manager)
{
	Manager->ShowMessage(TEXT("The Lion slumbers"), FColor::Orange);
}
void ALionCharacter::TickRest(ATurnBattleManager* Manager)
{
	if (bAwake)
	{
		return;
	}
	if (Manager->GetRound() > RestLastRound)
	{
		WakeFromRest(Manager);
		return;
	}
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Lion_Rest"));
	if (Row == nullptr)
	{
		return;
	}
	float HealthBefore = Health->CurrentHealth;
	Health->Heal(Health->MaxHealth * Row->Amount2);
	float Healed = Health->CurrentHealth - HealthBefore;
	Manager->ShowMessage(FString::Printf(TEXT("The Lion heals %d in its sleep"), FMath::RoundToInt(Healed)), FColor::Orange);
}
void ALionCharacter::WakeFromRest(ATurnBattleManager* Manager)
{
	bAwake = true;
	Stats->RemoveModifiersFromSource(FName("Lion_Rest"));
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Lion_Rest"));
	if (Row == nullptr)
	{
		return;
	}
	AddAwakenedModifier(ECharacterStat::Defense, Row->Amount3);
	AddAwakenedModifier(ECharacterStat::Damage, Row->Amount3);
	Manager->ShowMessage(TEXT("The Lion awakens, stronger than before"), FColor::Red);
}
void ALionCharacter::AddAwakenedModifier(ECharacterStat Stat, float Amount)
{
	FStatModifier Awakened;
	Awakened.SourceId = FName("Lion_Awakened");
	Awakened.Stat = Stat;
	Awakened.Kind = EModifierKind::Percent;
	Awakened.Amount = Amount;
	Awakened.Duration = 0.0f;
	Awakened.TurnsLeft = 0;
	Awakened.Rule = EStackRule::RefreshSameSource;
	Stats->AddModifier(Awakened);
}
void ALionCharacter::TickChallengeMana(ATurnBattleManager* Manager)
{
	if (!IsChallengeActive(Manager))
	{
		return;
	}
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Lion_Challenge"));
	if (Row == nullptr)
	{
		return;
	}
	APartyMemberCharacter* Member = Manager->GetPartyMember(ChallengedIndex);
	float MaxMana = Member->Stats->GetFinal(ECharacterStat::MaxMana);
	Member->Stats->RestoreMana(MaxMana * Row->Amount2);
}
void ALionCharacter::TriggerSovereignty(ATurnBattleManager* Manager, int32 MemberIndex)
{
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Lion_Sovereignty"));
	if (Row == nullptr)
	{
		return;
	}
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return;
	}
	AddSubjugation(Manager, MemberIndex, 1);
	float DamageDealt = HitMemberWithMove(Manager, MemberIndex, Row, GetHuntMultiplier(Manager, MemberIndex));
	Manager->ShowMessage(FString::Printf(TEXT("Sovereignty! The Lion punishes %s for %d"), *Member->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Orange);
	bSovereignMitigation = true;
	SovereignMitigation = Row->Amount1;
}
void ALionCharacter::RecordActionStart()
{
	HealthBeforeAction = Health->CurrentHealth;
	GuardBeforeAction = Health->CurrentHealth + Health->Shield;
}
void ALionCharacter::RecordActionDamage(ATurnBattleManager* Manager, int32 MemberIndex)
{
	float GuardAfter = Health->CurrentHealth + Health->Shield;
	float HealthLost = HealthBeforeAction - Health->CurrentHealth;
	if (HealthLost > 0.0f)
	{
		PlayerPhaseDamage = PlayerPhaseDamage + HealthLost;
	}
	if (GuardBeforeAction - GuardAfter <= 0.01f)
	{
		return;
	}
	int32 Amount = 1;
	if (IsChallenged(Manager, MemberIndex))
	{
		const FBossMoveRow* Row = Manager->GetBossMove(FName("Lion_Challenge"));
		if (Row != nullptr)
		{
			Amount = Amount + FMath::RoundToInt(Row->Amount3);
		}
	}
	AddSubjugation(Manager, MemberIndex, Amount);
}
float ALionCharacter::GetHitMultiplierFor(ATurnBattleManager* Manager, int32 MemberIndex) const
{
	return GetHuntMultiplier(Manager, MemberIndex);
}
