#include "Characters/DragonCharacter.h"
#include "Characters/PartyMemberCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Battle/BattleMoveRow.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Engine/DataTable.h"
ADragonCharacter::ADragonCharacter()
{
	DisplayName = TEXT("Dragon");
}
void ADragonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateScales();
	CheckHellfire();
}
void ADragonCharacter::OnBattleRoundStart(ATurnBattleManager* Manager)
{
	Super::OnBattleRoundStart(Manager);
	BattleManager = Manager;
	LandFireballs(Manager);
	SpewFireballs(Manager);
	TickFlameHorizon(Manager);
}
void ADragonCharacter::BeforeMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName)
{
	Super::BeforeMemberAction(Manager, MemberIndex, EntryName);
	if (!IsRuined(MemberIndex))
	{
		return;
	}
	if (RuinedMoves[MemberIndex] != EntryName)
	{
		return;
	}
	RuinedMoves[MemberIndex] = NAME_None;
	TriggerRuination(Manager, MemberIndex);
}
void ADragonCharacter::AfterMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName)
{
	Super::AfterMemberAction(Manager, MemberIndex, EntryName);
	if (IsGrailActive(Manager))
	{
		SpillGrail(Manager);
	}
}
FString ADragonCharacter::GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const
{
	FString Note;
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return Note;
	}
	if (HasBlaze(Member))
	{
		Note = Note + TEXT("Blaze ");
	}
	if (IsRuined(MemberIndex))
	{
		FString MoveName = RuinedMoves[MemberIndex].ToString();
		const FBattleMoveRow* Row = Manager->GetMove(RuinedMoves[MemberIndex]);
		if (Row != nullptr)
		{
			MoveName = Row->DisplayName.ToString();
		}
		Note = Note + FString::Printf(TEXT("Ruined: %s"), *MoveName);
	}
	return Note;
}
FString ADragonCharacter::GetStatusNote(const ATurnBattleManager* Manager) const
{
	FString Note = Super::GetStatusNote(Manager);
	int32 Fireballs = 0;
	for (int32 i = 0; i < VolleyCounts.Num(); i++)
	{
		Fireballs = Fireballs + VolleyCounts[i];
	}
	if (Fireballs > 0)
	{
		Note = Note + FString::Printf(TEXT("Fireballs %d "), Fireballs);
	}
	if (FuryCharges > 0)
	{
		Note = Note + FString::Printf(TEXT("Fury x%d "), FuryCharges);
	}
	if (FlameRoundsLeft > 0)
	{
		Note = Note + FString::Printf(TEXT("Horizon %d "), FlameRoundsLeft);
	}
	if (GrailLastRound >= Manager->GetRound())
	{
		Note = Note + TEXT("Grail ");
	}
	return Note;
}
float ADragonCharacter::GetOwnMissingPercent() const
{
	if (Health->MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	float MissingHealth = Health->MaxHealth - Health->CurrentHealth;
	if (MissingHealth < 0.0f)
	{
		return 0.0f;
	}
	return (MissingHealth / Health->MaxHealth) * 100.0f;
}
void ADragonCharacter::UpdateScales()
{
	if (Health->IsDead())
	{
		return;
	}
	int32 Steps = FMath::FloorToInt(GetOwnMissingPercent() / 10.0f);
	if (Steps == ScaleSteps)
	{
		return;
	}
	ScaleSteps = Steps;
	FStatModifier Scales;
	Scales.SourceId = FName("DragonScales");
	Scales.Stat = ECharacterStat::Defense;
	Scales.Kind = EModifierKind::Percent;
	Scales.Amount = Steps * DefensePerTenPercent;
	Scales.Duration = 0.0f;
	Scales.TurnsLeft = 0;
	Scales.Rule = EStackRule::RefreshSameSource;
	Stats->AddModifier(Scales);
}
void ADragonCharacter::CheckHellfire()
{
	if (bHellfireDone)
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
	const FBossMoveRow* Row = BattleManager->GetBossMove(FName("Dragon_Hellfire"));
	if (Row == nullptr)
	{
		return;
	}
	if (Health->CurrentHealth > Health->MaxHealth * Row->Amount1)
	{
		return;
	}
	bHellfireDone = true;
	CastHellfire(BattleManager);
}
void ADragonCharacter::CastHellfire(ATurnBattleManager* Manager)
{
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Dragon_Hellfire"));
	if (Row == nullptr)
	{
		return;
	}
	Manager->ShowMessage(TEXT("The Dragon unleashes HELLFIRE!"), FColor::Red);
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				float DamageDealt = HitMemberWithMove(Manager, i, Row, 1.0f);
				Manager->ShowMessage(FString::Printf(TEXT("Hellfire burns %s for %d"), *Member->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Red);
			}
		}
	}
}
void ADragonCharacter::SpewFireballs(ATurnBattleManager* Manager)
{
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Dragon_Fireball"));
	if (Row == nullptr)
	{
		return;
	}
	if (Manager->GetRound() % 2 == 0)
	{
		return;
	}
	int32 MinBalls = FMath::RoundToInt(Row->Amount1);
	int32 MaxBalls = FMath::RoundToInt(Row->Amount2);
	if (MaxBalls < MinBalls)
	{
		MaxBalls = MinBalls;
	}
	int32 Count = FMath::RandRange(MinBalls, MaxBalls);
	if (Count <= 0)
	{
		return;
	}
	int32 Delay = FMath::RoundToInt(Row->Amount3);
	VolleyLandRounds.Add(Manager->GetRound() + Delay);
	VolleyCounts.Add(Count);
	Manager->ShowMessage(FString::Printf(TEXT("The Dragon spews %d fireball(s) into the sky"), Count), FColor::Red);
}
void ADragonCharacter::LandFireballs(ATurnBattleManager* Manager)
{
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Dragon_Fireball"));
	if (Row == nullptr)
	{
		return;
	}
	TArray<int32> KeptRounds;
	TArray<int32> KeptCounts;
	for (int32 v = 0; v < VolleyLandRounds.Num(); v++)
	{
		if (VolleyLandRounds[v] > Manager->GetRound())
		{
			KeptRounds.Add(VolleyLandRounds[v]);
			KeptCounts.Add(VolleyCounts[v]);
		}
		else
		{
			for (int32 Ball = 0; Ball < VolleyCounts[v]; Ball++)
			{
				int32 TargetIndex = Manager->PickRandomLivingMember();
				APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
				if (Target != nullptr)
				{
					float DamageDealt = HitMemberWithMove(Manager, TargetIndex, Row, 1.0f);
					Manager->ShowMessage(FString::Printf(TEXT("A fireball lands on %s for %d"), *Target->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Red);
				}
			}
		}
	}
	VolleyLandRounds = KeptRounds;
	VolleyCounts = KeptCounts;
}
void ADragonCharacter::TickFlameHorizon(ATurnBattleManager* Manager)
{
	if (FlameRoundsLeft <= 0)
	{
		return;
	}
	FlameRoundsLeft = FlameRoundsLeft - 1;
	float OwnMissingHealth = Health->MaxHealth - Health->CurrentHealth;
	if (OwnMissingHealth < 0.0f)
	{
		OwnMissingHealth = 0.0f;
	}
	float TotalDamage = 0.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				float RawDamage = Stats->GetFinal(ECharacterStat::Damage) * FlamePower;
				float BaseDamage = RawDamage * GetDefenseMitigation(Member);
				float FireDamage = OwnMissingHealth * FlameOwnMissingPercent * GetAttackScale();
				TotalDamage = TotalDamage + Member->ApplyHit(BaseDamage + FireDamage, true, nullptr);
			}
		}
	}
	Manager->ShowMessage(FString::Printf(TEXT("The sea of fire burns the party for %d (%d rounds left)"), FMath::RoundToInt(TotalDamage), FlameRoundsLeft), FColor::Red);
}
float ADragonCharacter::FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker)
{
	float Filtered = Super::FilterIncomingDamage(RawDamage, bTrueDamage, Attacker);
	if (FuryCharges <= 0)
	{
		return Filtered;
	}
	if (Filtered <= 0.0f)
	{
		return Filtered;
	}
	if (BattleManager == nullptr)
	{
		return Filtered;
	}
	const FBossMoveRow* Row = BattleManager->GetBossMove(FName("Dragon_Fury"));
	if (Row == nullptr)
	{
		return Filtered;
	}
	FuryCharges = FuryCharges - 1;
	float Reduction = Row->Amount1 + (Row->Amount2 * GetOwnMissingPercent() / 100.0f);
	if (Reduction > 0.9f)
	{
		Reduction = 0.9f;
	}
	BattleManager->ShowMessage(FString::Printf(TEXT("The Dragon's fury softens the blow by %d%%"), FMath::RoundToInt(Reduction * 100.0f)), FColor::Red);
	return Filtered * (1.0f - Reduction);
}
float ADragonCharacter::ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	float BaseScore = Super::ScoreMove(Manager, MoveId, Move);
	if (BaseScore <= 0.0f)
	{
		return 0.0f;
	}
	if (MoveId == FName("Dragon_Scorch"))
	{
		return ScoreScorch(Move);
	}
	if (MoveId == FName("Dragon_Ruination"))
	{
		return ScoreRuination(Manager, Move);
	}
	if (MoveId == FName("Dragon_Plume"))
	{
		return ScorePlume(Manager, Move);
	}
	if (MoveId == FName("Dragon_Almighty"))
	{
		return ScoreAlmighty(Move);
	}
	if (MoveId == FName("Dragon_Fury"))
	{
		return ScoreFury(Move);
	}
	if (MoveId == FName("Dragon_FlameHorizon"))
	{
		return ScoreFlameHorizon(Move);
	}
	if (MoveId == FName("Dragon_Grail"))
	{
		return ScoreGrail(Manager, Move);
	}
	return BaseScore;
}
float ADragonCharacter::ScoreScorch(const FBossMoveRow* Move) const
{
	return Move->ScoreBase + (Move->ScoreBonus1 * GetOwnMissingPercent());
}
float ADragonCharacter::ScoreRuination(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (FindRuinationTarget(Manager) < 0)
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
float ADragonCharacter::ScorePlume(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	float Score = Move->ScoreBase;
	APartyMemberCharacter* Target = Manager->GetPartyMember(FindLowestHealthMember(Manager));
	if (Target != nullptr)
	{
		if (Target->Health->MaxHealth > 0.0f)
		{
			float HealthFraction = Target->Health->CurrentHealth / Target->Health->MaxHealth;
			if (HealthFraction < Move->Amount1)
			{
				Score = Score + Move->ScoreBonus1;
			}
		}
	}
	return Score;
}
float ADragonCharacter::ScoreAlmighty(const FBossMoveRow* Move) const
{
	int32 Stacks = Stats->GetStacks(FName("Dragon_Almighty"), ECharacterStat::Damage, EModifierKind::Percent);
	if (Stacks >= Move->MaxStacks)
	{
		return 0.0f;
	}
	if (Health->MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	float HealthFraction = Health->CurrentHealth / Health->MaxHealth;
	if (HealthFraction > Move->Amount2)
	{
		return Move->ScoreBase;
	}
	return Move->ScoreBonus1;
}
float ADragonCharacter::ScoreFury(const FBossMoveRow* Move) const
{
	if (FuryCharges >= Move->MaxStacks)
	{
		return 0.0f;
	}
	return Move->ScoreBase + (Move->ScoreBonus1 * GetOwnMissingPercent());
}
float ADragonCharacter::ScoreFlameHorizon(const FBossMoveRow* Move) const
{
	if (FlameRoundsLeft > 0)
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
float ADragonCharacter::ScoreGrail(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (GrailLastRound >= Manager->GetRound())
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
int32 ADragonCharacter::PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	if (MoveId == FName("Dragon_Plume"))
	{
		return FindLowestHealthMember(Manager);
	}
	if (MoveId == FName("Dragon_Blaze"))
	{
		return FindHighestDefenseWithoutBlaze(Manager);
	}
	if (MoveId == FName("Dragon_Ruination"))
	{
		return FindRuinationTarget(Manager);
	}
	return Super::PickMoveTarget(Manager, MoveId, Move);
}
void ADragonCharacter::PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex)
{
	if (MoveId == FName("Dragon_Scorch"))
	{
		CastScorch(Manager, Move);
	}
	else if (MoveId == FName("Dragon_Ruination"))
	{
		CastRuination(Manager, TargetIndex);
	}
	else if (MoveId == FName("Dragon_Almighty"))
	{
		CastAlmighty(Manager, MoveId, Move);
	}
	else if (MoveId == FName("Dragon_Fury"))
	{
		CastFury(Manager, Move);
	}
	else if (MoveId == FName("Dragon_FlameHorizon"))
	{
		CastFlameHorizon(Manager, Move);
	}
	else if (MoveId == FName("Dragon_Grail"))
	{
		GrailMoveId = MoveId;
		CastGrail(Manager, Move);
	}
	else
	{
		Super::PerformMove(Manager, MoveId, Move, TargetIndex);
	}
}
int32 ADragonCharacter::FindLowestHealthMember(ATurnBattleManager* Manager) const
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
bool ADragonCharacter::HasBlaze(APartyMemberCharacter* Member) const
{
	int32 Stacks = Member->Stats->GetStacks(FName("Dragon_Blaze"), ECharacterStat::Defense, EModifierKind::Percent);
	return Stacks > 0;
}
int32 ADragonCharacter::FindHighestDefenseWithoutBlaze(ATurnBattleManager* Manager) const
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
				if (!HasBlaze(Member))
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
	if (BestIndex < 0)
	{
		return Manager->PickRandomLivingMember();
	}
	return BestIndex;
}
bool ADragonCharacter::IsRuined(int32 MemberIndex) const
{
	if (MemberIndex < 0 || MemberIndex >= RuinedMoves.Num())
	{
		return false;
	}
	return !RuinedMoves[MemberIndex].IsNone();
}
TArray<FName> ADragonCharacter::GetRuinableMoves(const ATurnBattleManager* Manager, APartyMemberCharacter* Member) const
{
	TArray<FName> Ruinable;
	if (Manager->MoveTable == nullptr)
	{
		return Ruinable;
	}
	TArray<FName> AllNames = Manager->MoveTable->GetRowNames();
	for (int32 i = 0; i < AllNames.Num(); i++)
	{
		const FBattleMoveRow* Row = Manager->GetMove(AllNames[i]);
		if (Row != nullptr)
		{
			if (Row->MemberId == Member->StatsRowName)
			{
				if (Manager->IsMoveUnlocked(Row))
				{
					Ruinable.Add(AllNames[i]);
				}
			}
		}
	}
	return Ruinable;
}
int32 ADragonCharacter::FindRuinationTarget(ATurnBattleManager* Manager) const
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
				if (!IsRuined(i))
				{
					if (GetRuinableMoves(Manager, Member).Num() > 0)
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
	}
	return BestIndex;
}
void ADragonCharacter::CastScorch(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			float Multiplier = 1.0f;
			if (HasBlaze(Member))
			{
				Multiplier = 1.0f + Move->Amount1;
			}
			HitMemberWithMove(Manager, i, Move, Multiplier);
		}
	}
}
void ADragonCharacter::CastRuination(ATurnBattleManager* Manager, int32 TargetIndex)
{
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	TArray<FName> Ruinable = GetRuinableMoves(Manager, Target);
	if (Ruinable.Num() == 0)
	{
		Manager->ShowMessage(TEXT("The curse finds nothing to mark"), FColor::Red);
		return;
	}
	while (RuinedMoves.Num() < Manager->GetPartyCount())
	{
		RuinedMoves.Add(NAME_None);
	}
	int32 Pick = FMath::RandRange(0, Ruinable.Num() - 1);
	RuinedMoves[TargetIndex] = Ruinable[Pick];
	FString MoveName = Ruinable[Pick].ToString();
	const FBattleMoveRow* Row = Manager->GetMove(Ruinable[Pick]);
	if (Row != nullptr)
	{
		MoveName = Row->DisplayName.ToString();
	}
	Manager->ShowMessage(FString::Printf(TEXT("%s's %s is cursed with ruination"), *Target->DisplayName, *MoveName), FColor::Red);
}
void ADragonCharacter::TriggerRuination(ATurnBattleManager* Manager, int32 MemberIndex)
{
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Dragon_Ruination"));
	if (Row == nullptr)
	{
		return;
	}
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return;
	}
	float DamageDealt = HitMemberWithMove(Manager, MemberIndex, Row, 1.0f);
	Manager->ShowMessage(FString::Printf(TEXT("Ruination burns %s for %d"), *Member->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Red);
}
void ADragonCharacter::CastAlmighty(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	float Cost = Health->CurrentHealth * Move->Amount1;
	float NewHealth = Health->CurrentHealth - Cost;
	if (NewHealth < 1.0f)
	{
		NewHealth = 1.0f;
	}
	Health->CurrentHealth = NewHealth;
	AddAlmightyModifier(MoveId, ECharacterStat::Defense, Move);
	AddAlmightyModifier(MoveId, ECharacterStat::Damage, Move);
	Manager->ShowMessage(FString::Printf(TEXT("The Dragon burns %d of its own health to grow stronger"), FMath::RoundToInt(Cost)), FColor::Red);
}
void ADragonCharacter::AddAlmightyModifier(FName MoveId, ECharacterStat Stat, const FBossMoveRow* Move)
{
	FStatModifier Might;
	Might.SourceId = MoveId;
	Might.Stat = Stat;
	Might.Kind = EModifierKind::Percent;
	Might.Amount = Move->ModAmount;
	Might.Duration = 0.0f;
	Might.TurnsLeft = 0;
	Might.Rule = EStackRule::StackToMax;
	Might.MaxStacks = Move->MaxStacks;
	Stats->AddModifier(Might);
}
void ADragonCharacter::CastFury(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	FuryCharges = FuryCharges + 1;
	if (FuryCharges > Move->MaxStacks)
	{
		FuryCharges = Move->MaxStacks;
	}
	Manager->ShowMessage(FString::Printf(TEXT("The Dragon seethes with fury (%d charge(s))"), FuryCharges), FColor::Red);
}
void ADragonCharacter::CastFlameHorizon(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	FlameRoundsLeft = Move->Turns;
	FlamePower = Move->Power;
	FlameOwnMissingPercent = Move->OwnMissingPercent;
	Manager->ShowMessage(TEXT("A sea of fire spreads beneath the party"), FColor::Red);
}
bool ADragonCharacter::IsGrailActive(const ATurnBattleManager* Manager) const
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
void ADragonCharacter::CastGrail(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	int32 Rounds = Move->Turns;
	if (Rounds < 1)
	{
		Rounds = 1;
	}
	GrailFirstRound = Manager->GetRound() + 1;
	GrailLastRound = Manager->GetRound() + Rounds;
	Manager->ShowMessage(TEXT("A grail of flame hovers over the party"), FColor::Red);
}
void ADragonCharacter::SpillGrail(ATurnBattleManager* Manager)
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
