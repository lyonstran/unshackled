#include "Characters/RavanaCharacter.h"
#include "Characters/ChimeraCharacter.h"
#include "Characters/PartyMemberCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
ARavanaCharacter::ARavanaCharacter()
{
	DisplayName = TEXT("S. Ravana");
	BlunderChance = 0.1f;
	ExcludedMemberId = FName("Sirius");
}
void ARavanaCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateSaturnEffects();
	TrackChimeraDeath();
}
void ARavanaCharacter::OnBattleRoundStart(ATurnBattleManager* Manager)
{
	Super::OnBattleRoundStart(Manager);
	BattleManager = Manager;
	EnsureMemberArrays(Manager);
	SpawnChimeraIfNeeded(Manager);
	ReviveChimeraIfReady(Manager);
	TickWindFunnel(Manager);
	TickGale(Manager);
}
void ARavanaCharacter::BeforeMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName)
{
	Super::BeforeMemberAction(Manager, MemberIndex, EntryName);
	BattleManager = Manager;
	EnsureMemberArrays(Manager);
	if (LonginusTarget < 0)
	{
		return;
	}
	if (LonginusRound != Manager->GetRound())
	{
		return;
	}
	ResolveLonginus(Manager, MemberIndex);
}
void ARavanaCharacter::OnBossPhaseStart(ATurnBattleManager* Manager)
{
	Super::OnBossPhaseStart(Manager);
	for (int32 i = 0; i < PendingAbsolutions.Num(); i++)
	{
		ApplyAbsolution(Manager, PendingAbsolutions[i]);
	}
	PendingAbsolutions.Empty();
}
void ARavanaCharacter::OnRetributionCompleted(ATurnBattleManager* Manager, int32 MemberIndex)
{
	Super::OnRetributionCompleted(Manager, MemberIndex);
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return;
	}
	PendingAbsolutions.Add(Member->StatsRowName);
	Manager->ShowMessage(TEXT("S. Ravana watches the retribution... and will answer with Absolution"), FColor::Purple);
}
int32 ARavanaCharacter::GetBossTurnCount(ATurnBattleManager* Manager, int32 DefaultTurns)
{
	int32 Turns = BaseTurnsPerRound;
	if (Health->CurrentHealth <= Health->MaxHealth * ExtraTurnHealthFraction)
	{
		Turns = Turns + 1;
	}
	Turns = Turns + AbsolutionTurns;
	AbsolutionTurns = 0;
	return Turns;
}
FString ARavanaCharacter::GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const
{
	FString Note;
	int32 Stacks = GetSaturn(MemberIndex);
	if (Stacks > 0)
	{
		Note = Note + FString::Printf(TEXT("Saturn %d "), Stacks);
	}
	if (MemberIndex == LonginusTarget)
	{
		Note = Note + TEXT("Longinus ");
	}
	return Note;
}
FString ARavanaCharacter::GetStatusNote(const ATurnBattleManager* Manager) const
{
	FString Note = Super::GetStatusNote(Manager);
	if (bWallUp)
	{
		Note = Note + TEXT("Wind Wall ");
	}
	if (bDominionPending)
	{
		Note = Note + TEXT("Dominion ");
	}
	if (GaleRoundsLeft > 0)
	{
		Note = Note + FString::Printf(TEXT("Gale %d "), GaleRoundsLeft);
	}
	if (ChimeraDeathRound >= 0)
	{
		int32 RoundsLeft = ChimeraDeathRound + ChimeraReviveRounds + 1 - Manager->GetRound();
		Note = Note + FString::Printf(TEXT("Chimera rises in %d "), RoundsLeft);
	}
	return Note;
}
void ARavanaCharacter::EnsureMemberArrays(const ATurnBattleManager* Manager)
{
	if (Manager == nullptr)
	{
		return;
	}
	while (SaturnStacks.Num() < Manager->GetPartyCount())
	{
		SaturnStacks.Add(0);
		AppliedSaturn.Add(0);
		GaleTickDamage.Add(0.0f);
	}
}
int32 ARavanaCharacter::GetSaturn(int32 MemberIndex) const
{
	if (MemberIndex < 0 || MemberIndex >= SaturnStacks.Num())
	{
		return 0;
	}
	return SaturnStacks[MemberIndex];
}
int32 ARavanaCharacter::GetTotalSaturn() const
{
	int32 Total = 0;
	for (int32 i = 0; i < SaturnStacks.Num(); i++)
	{
		Total = Total + SaturnStacks[i];
	}
	return Total;
}
void ARavanaCharacter::AddSaturn(ATurnBattleManager* Manager, int32 MemberIndex, int32 Amount)
{
	EnsureMemberArrays(Manager);
	if (MemberIndex < 0 || MemberIndex >= SaturnStacks.Num())
	{
		return;
	}
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Ravana_Saturn"));
	if (Row == nullptr)
	{
		return;
	}
	int32 NewStacks = SaturnStacks[MemberIndex] + Amount;
	if (NewStacks > Row->MaxStacks)
	{
		NewStacks = Row->MaxStacks;
	}
	SaturnStacks[MemberIndex] = NewStacks;
}
void ARavanaCharacter::UpdateSaturnEffects()
{
	if (BattleManager == nullptr)
	{
		return;
	}
	if (Health->IsDead())
	{
		return;
	}
	const FBossMoveRow* Row = BattleManager->GetBossMove(FName("Ravana_Saturn"));
	if (Row == nullptr)
	{
		return;
	}
	for (int32 i = 0; i < SaturnStacks.Num(); i++)
	{
		UpdateMemberSaturn(i, Row);
	}
}
void ARavanaCharacter::UpdateMemberSaturn(int32 MemberIndex, const FBossMoveRow* Row)
{
	int32 Stacks = SaturnStacks[MemberIndex];
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
	if (Member->Stats->GetStacks(FName("Ravana_Saturn"), ECharacterStat::Damage, EModifierKind::Percent) == 0)
	{
		bMissing = true;
	}
	if (!bMissing)
	{
		if (AppliedSaturn[MemberIndex] == Stacks)
		{
			return;
		}
	}
	FStatModifier Weakness;
	Weakness.SourceId = FName("Ravana_Saturn");
	Weakness.Stat = ECharacterStat::Damage;
	Weakness.Kind = EModifierKind::Percent;
	Weakness.Amount = Row->ModAmount * Stacks;
	Weakness.Duration = 0.0f;
	Weakness.TurnsLeft = 0;
	Weakness.Rule = EStackRule::RefreshSameSource;
	Member->Stats->AddModifier(Weakness);
	AppliedSaturn[MemberIndex] = Stacks;
}
void ARavanaCharacter::CheckSaturnExecute(ATurnBattleManager* Manager, int32 MemberIndex)
{
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Ravana_Saturn"));
	if (Row == nullptr)
	{
		return;
	}
	if (GetSaturn(MemberIndex) < Row->Amount1)
	{
		return;
	}
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return;
	}
	if (Member->Health->IsDead())
	{
		return;
	}
	if (Member->Health->CurrentHealth >= Member->Health->MaxHealth * Row->Amount2)
	{
		return;
	}
	bool bKilled = Member->TakeExecute(this);
	if (bKilled)
	{
		Manager->ShowMessage(FString::Printf(TEXT("S. Ravana EXECUTES %s"), *Member->DisplayName), FColor::Red);
	}
	else
	{
		Manager->ShowMessage(FString::Printf(TEXT("%s refuses to fall to S. Ravana"), *Member->DisplayName), FColor::Cyan);
	}
}
float ARavanaCharacter::RavanaHit(ATurnBattleManager* Manager, int32 MemberIndex, const FBossMoveRow* Move, float BonusDamage)
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
	float Bonus = BonusDamage;
	if (WallBonus > 0.0f)
	{
		Bonus = Bonus + WallBonus;
		WallBonus = 0.0f;
	}
	float DamageDealt = HitMemberWithBonus(Manager, MemberIndex, Move, 1.0f, Bonus);
	AddSaturn(Manager, MemberIndex, 1);
	CheckSaturnExecute(Manager, MemberIndex);
	if (bDominionPending)
	{
		if (DamageDealt > 0.0f)
		{
			bDominionPending = false;
			float HealthBefore = Health->CurrentHealth;
			Health->Heal(Health->MaxHealth * DominionHeal);
			float Healed = Health->CurrentHealth - HealthBefore;
			Manager->ShowMessage(FString::Printf(TEXT("Dominion: S. Ravana restores %d health"), FMath::RoundToInt(Healed)), FColor::Purple);
		}
	}
	return DamageDealt;
}
bool ARavanaCharacter::IsChimeraAlive() const
{
	if (Chimera == nullptr)
	{
		return false;
	}
	return !Chimera->Health->IsDead();
}
void ARavanaCharacter::SpawnChimeraIfNeeded(ATurnBattleManager* Manager)
{
	if (bChimeraSpawned)
	{
		return;
	}
	bChimeraSpawned = true;
	if (ChimeraClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ravana: Chimera Class is not set, no Chimera this battle"));
		return;
	}
	ABattleSummonCharacter* Spawned = Manager->SpawnSummon(ChimeraClass, Health->MaxHealth * ChimeraHealthFraction);
	Chimera = Cast<AChimeraCharacter>(Spawned);
}
void ARavanaCharacter::TrackChimeraDeath()
{
	if (Chimera == nullptr)
	{
		return;
	}
	if (BattleManager == nullptr)
	{
		return;
	}
	if (!Chimera->Health->IsDead())
	{
		return;
	}
	if (ChimeraDeathRound >= 0)
	{
		return;
	}
	ChimeraDeathRound = BattleManager->GetRound();
	BattleManager->ShowMessage(FString::Printf(TEXT("The Chimera collapses... it will rise again in %d rounds"), ChimeraReviveRounds), FColor::Purple);
}
void ARavanaCharacter::ReviveChimeraIfReady(ATurnBattleManager* Manager)
{
	if (Chimera == nullptr)
	{
		return;
	}
	if (ChimeraDeathRound < 0)
	{
		return;
	}
	if (Manager->GetRound() <= ChimeraDeathRound + ChimeraReviveRounds)
	{
		return;
	}
	ChimeraDeathRound = -1;
	Chimera->ReviveInBattle();
	Manager->ShowMessage(TEXT("S. Ravana's magic drags the Chimera back to its feet"), FColor::Purple);
}
void ARavanaCharacter::TickWindFunnel(ATurnBattleManager* Manager)
{
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Ravana_Funnel"));
	if (Row == nullptr)
	{
		return;
	}
	float TotalDamage = 0.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				float WindDamage = Member->Health->CurrentHealth * Row->Percent;
				TotalDamage = TotalDamage + Member->ApplyHit(WindDamage, true, nullptr);
			}
		}
	}
	Manager->ShowMessage(FString::Printf(TEXT("Saturn's winds tear at the party for %d"), FMath::RoundToInt(TotalDamage)), FColor::Purple);
}
void ARavanaCharacter::TickGale(ATurnBattleManager* Manager)
{
	if (GaleRoundsLeft <= 0)
	{
		return;
	}
	GaleRoundsLeft = GaleRoundsLeft - 1;
	float TotalDamage = 0.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (i < GaleTickDamage.Num())
				{
					if (GaleTickDamage[i] > 0.0f)
					{
						TotalDamage = TotalDamage + Member->ApplyHit(GaleTickDamage[i], true, nullptr);
					}
				}
			}
		}
	}
	Manager->ShowMessage(FString::Printf(TEXT("The lingering gale hits the party for %d (%d rounds left)"), FMath::RoundToInt(TotalDamage), GaleRoundsLeft), FColor::Purple);
}
void ARavanaCharacter::ResolveLonginus(ATurnBattleManager* Manager, int32 ActingIndex)
{
	int32 TargetIndex = LonginusTarget;
	LonginusTarget = -1;
	LonginusRound = -1;
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Ravana_Longinus"));
	if (Row == nullptr)
	{
		return;
	}
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	if (Target->Health->IsDead())
	{
		return;
	}
	if (ActingIndex == TargetIndex)
	{
		FBossMoveRow FirstMoverRow = *Row;
		FirstMoverRow.PercentOf = EBossPercentOf::CurrentHealth;
		float StackDamage = GetSaturn(TargetIndex);
		float DamageDealt = RavanaHit(Manager, TargetIndex, &FirstMoverRow, StackDamage);
		Manager->ShowMessage(FString::Printf(TEXT("%s moves first: the spear of Longinus still strikes for %d"), *Target->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Purple);
		return;
	}
	float DamageDealt = RavanaHit(Manager, TargetIndex, Row, 0.0f);
	Manager->ShowMessage(FString::Printf(TEXT("The spear of Longinus impales %s for %d"), *Target->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Red);
}
void ARavanaCharacter::ApplyAbsolution(ATurnBattleManager* Manager, FName MemberId)
{
	if (MemberId == FName("Malkholm"))
	{
		AbsolutionTurns = AbsolutionTurns + 1;
		Manager->ShowMessage(TEXT("Absolution: S. Ravana mirrors Mal'Kholm and gains an extra turn"), FColor::Purple);
		return;
	}
	if (MemberId == FName("Hanuboy"))
	{
		MirrorBombs(Manager);
		return;
	}
	if (MemberId == FName("Kalia"))
	{
		MirrorCleanse(Manager);
		return;
	}
}
void ARavanaCharacter::MirrorBombs(ATurnBattleManager* Manager)
{
	const FBossMoveRow* Row = Manager->GetBossMove(FName("Ravana_Absolution"));
	if (Row == nullptr)
	{
		return;
	}
	float TotalDamage = 0.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				float BombDamage = Member->Health->CurrentHealth * Row->Amount1;
				TotalDamage = TotalDamage + Member->ApplyHit(BombDamage, true, nullptr);
			}
		}
	}
	Health->Heal(Health->CurrentHealth * Row->Amount2);
	if (IsChimeraAlive())
	{
		Chimera->Health->Heal(Chimera->Health->CurrentHealth * Row->Amount2);
	}
	Manager->ShowMessage(FString::Printf(TEXT("Absolution: dark flower bombs burst on the party for %d and mend the bosses"), FMath::RoundToInt(TotalDamage)), FColor::Purple);
}
void ARavanaCharacter::MirrorCleanse(ATurnBattleManager* Manager)
{
	Stats->RemoveHarmfulModifiers();
	if (IsChimeraAlive())
	{
		Chimera->Stats->RemoveHarmfulModifiers();
	}
	Manager->ShowMessage(TEXT("Absolution: S. Ravana and the Chimera shed every curse"), FColor::Purple);
}
bool ARavanaCharacter::HasOwnModifier(FName SourceId, ECharacterStat Stat, EModifierKind Kind) const
{
	return Stats->GetStacks(SourceId, Stat, Kind) > 0;
}
void ARavanaCharacter::AddTurnModifier(AUnshackledCharacterBase* Target, FName SourceId, ECharacterStat Stat, EModifierKind Kind, float Amount, int32 Turns)
{
	if (Target == nullptr)
	{
		return;
	}
	if (Target->Health->IsDead())
	{
		return;
	}
	FStatModifier NewModifier;
	NewModifier.SourceId = SourceId;
	NewModifier.Stat = Stat;
	NewModifier.Kind = Kind;
	NewModifier.Amount = Amount;
	NewModifier.Duration = 0.0f;
	NewModifier.TurnsLeft = Turns + 1;
	NewModifier.Rule = EStackRule::RefreshSameSource;
	Target->Stats->AddModifier(NewModifier);
}
float ARavanaCharacter::GetOwnMissingPercent() const
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
float ARavanaCharacter::FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker)
{
	float Filtered = Super::FilterIncomingDamage(RawDamage, bTrueDamage, Attacker);
	if (!bWallUp)
	{
		return Filtered;
	}
	if (Attacker == nullptr)
	{
		return Filtered;
	}
	if (Filtered <= 0.0f)
	{
		return Filtered;
	}
	bWallUp = false;
	WallBonus = WallBonus + (Filtered * WallConvert);
	if (BattleManager != nullptr)
	{
		BattleManager->ShowMessage(FString::Printf(TEXT("The wind wall swallows the blow; S. Ravana's next attack gains %d"), FMath::RoundToInt(Filtered * WallConvert)), FColor::Purple);
	}
	return 0.0f;
}
float ARavanaCharacter::ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	float BaseScore = Super::ScoreMove(Manager, MoveId, Move);
	if (BaseScore <= 0.0f)
	{
		return 0.0f;
	}
	if (MoveId == FName("Ravana_Kneel"))
	{
		return ScoreKneel(Manager, Move);
	}
	if (MoveId == FName("Ravana_Ravage"))
	{
		return ScoreRavage(Move);
	}
	if (MoveId == FName("Ravana_Longinus"))
	{
		return ScoreLonginus(Manager, Move);
	}
	if (MoveId == FName("Ravana_Gale"))
	{
		return ScoreGale(Move);
	}
	if (MoveId == FName("Ravana_Veil"))
	{
		return ScoreVeil(Move);
	}
	if (MoveId == FName("Ravana_Omnichannel"))
	{
		return ScoreOmnichannel(Move);
	}
	if (MoveId == FName("Ravana_WindWall"))
	{
		return ScoreWindWall(Move);
	}
	if (MoveId == FName("Ravana_WindsOfWar"))
	{
		return ScoreWindsOfWar(Move);
	}
	if (MoveId == FName("Ravana_Dominion"))
	{
		return ScoreDominion(Move);
	}
	return BaseScore;
}
float ARavanaCharacter::ScoreKneel(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	const FBossMoveRow* SaturnRow = Manager->GetBossMove(FName("Ravana_Saturn"));
	int32 TargetIndex = FindKneelTarget(Manager);
	if (SaturnRow == nullptr)
	{
		return Move->ScoreBase;
	}
	int32 Stacks = GetSaturn(TargetIndex);
	int32 Gained = 1 + FMath::RoundToInt(Move->Amount1);
	if (Stacks < SaturnRow->Amount1)
	{
		if (Stacks + Gained >= SaturnRow->Amount1)
		{
			return Move->ScoreBase + Move->ScoreBonus1;
		}
	}
	return Move->ScoreBase;
}
float ARavanaCharacter::ScoreRavage(const FBossMoveRow* Move) const
{
	float Score = Move->ScoreBase + (Move->ScoreBonus1 * GetTotalSaturn());
	if (Score > Move->ScoreBonus2)
	{
		Score = Move->ScoreBonus2;
	}
	return Score;
}
float ARavanaCharacter::ScoreLonginus(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (LonginusTarget >= 0)
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
float ARavanaCharacter::ScoreGale(const FBossMoveRow* Move) const
{
	if (GaleRoundsLeft > 0)
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
float ARavanaCharacter::ScoreVeil(const FBossMoveRow* Move) const
{
	if (HasOwnModifier(FName("Ravana_Veil"), ECharacterStat::IncomingDamage, EModifierKind::Percent))
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
float ARavanaCharacter::ScoreOmnichannel(const FBossMoveRow* Move) const
{
	if (HasOwnModifier(FName("Ravana_Omnichannel"), ECharacterStat::Damage, EModifierKind::Percent))
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
float ARavanaCharacter::ScoreWindWall(const FBossMoveRow* Move) const
{
	if (bWallUp)
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
float ARavanaCharacter::ScoreWindsOfWar(const FBossMoveRow* Move) const
{
	if (HasOwnModifier(FName("Ravana_WindsOfWar"), ECharacterStat::Damage, EModifierKind::Percent))
	{
		return 0.0f;
	}
	return Move->ScoreBase;
}
float ARavanaCharacter::ScoreDominion(const FBossMoveRow* Move) const
{
	if (bDominionPending)
	{
		return 0.0f;
	}
	return Move->ScoreBase + (Move->ScoreBonus1 * GetOwnMissingPercent());
}
int32 ARavanaCharacter::FindKneelTarget(ATurnBattleManager* Manager) const
{
	const FBossMoveRow* SaturnRow = Manager->GetBossMove(FName("Ravana_Saturn"));
	int32 Cap = 30;
	if (SaturnRow != nullptr)
	{
		Cap = SaturnRow->MaxStacks;
	}
	int32 BestIndex = -1;
	int32 BestStacks = -1;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				int32 Stacks = GetSaturn(i);
				if (Stacks < Cap)
				{
					if (Stacks > BestStacks)
					{
						BestStacks = Stacks;
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
int32 ARavanaCharacter::FindLowestHealthMember(ATurnBattleManager* Manager) const
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
int32 ARavanaCharacter::FindSubmissionTarget(ATurnBattleManager* Manager) const
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
				if (Member->Stats->GetStacks(FName("Ravana_Submission"), ECharacterStat::Damage, EModifierKind::Percent) == 0)
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
	if (BestIndex < 0)
	{
		return Manager->PickRandomLivingMember();
	}
	return BestIndex;
}
int32 ARavanaCharacter::PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	if (MoveId == FName("Ravana_Kneel"))
	{
		return FindKneelTarget(Manager);
	}
	if (MoveId == FName("Ravana_Longinus"))
	{
		return FindLowestHealthMember(Manager);
	}
	if (MoveId == FName("Ravana_Submission"))
	{
		return FindSubmissionTarget(Manager);
	}
	return Super::PickMoveTarget(Manager, MoveId, Move);
}
void ARavanaCharacter::PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex)
{
	EnsureMemberArrays(Manager);
	if (MoveId == FName("Ravana_Kneel"))
	{
		CastKneel(Manager, Move, TargetIndex);
	}
	else if (MoveId == FName("Ravana_Ravage"))
	{
		CastRavage(Manager, Move);
	}
	else if (MoveId == FName("Ravana_Longinus"))
	{
		CastLonginus(Manager, TargetIndex);
	}
	else if (MoveId == FName("Ravana_Gale"))
	{
		CastGale(Manager, Move);
	}
	else if (MoveId == FName("Ravana_Veil"))
	{
		CastVeil(Manager, MoveId, Move);
	}
	else if (MoveId == FName("Ravana_Omnichannel"))
	{
		CastOmnichannel(Manager, MoveId, Move);
	}
	else if (MoveId == FName("Ravana_WindWall"))
	{
		CastWindWall(Manager, Move);
	}
	else if (MoveId == FName("Ravana_WindsOfWar"))
	{
		CastWindsOfWar(Manager, MoveId, Move);
	}
	else if (MoveId == FName("Ravana_Submission"))
	{
		CastSubmission(Manager, MoveId, Move, TargetIndex);
	}
	else if (MoveId == FName("Ravana_Dominion"))
	{
		CastDominion(Manager, Move);
	}
	else
	{
		Super::PerformMove(Manager, MoveId, Move, TargetIndex);
	}
}
void ARavanaCharacter::CastKneel(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex)
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
	AddSaturn(Manager, TargetIndex, FMath::RoundToInt(Move->Amount1));
	float DamageDealt = RavanaHit(Manager, TargetIndex, Move, 0.0f);
	Manager->ShowMessage(FString::Printf(TEXT("%s is forced to kneel for %d"), *Target->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Purple);
}
void ARavanaCharacter::CastRavage(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	float TotalDamage = 0.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		FBossMoveRow StackedRow = *Move;
		StackedRow.Percent = Move->Percent + (Move->Amount1 * GetSaturn(i));
		TotalDamage = TotalDamage + RavanaHit(Manager, i, &StackedRow, 0.0f);
	}
	Manager->ShowMessage(FString::Printf(TEXT("Ravage pierces the party for %d"), FMath::RoundToInt(TotalDamage)), FColor::Purple);
}
void ARavanaCharacter::CastLonginus(ATurnBattleManager* Manager, int32 TargetIndex)
{
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	LonginusTarget = TargetIndex;
	LonginusRound = Manager->GetRound() + 1;
	Manager->ShowMessage(FString::Printf(TEXT("S. Ravana raises the spear of Longinus over %s: they should act first next round"), *Target->DisplayName), FColor::Purple);
}
void ARavanaCharacter::CastGale(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	float TotalDamage = 0.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		float DamageDealt = RavanaHit(Manager, i, Move, 0.0f);
		GaleTickDamage[i] = DamageDealt * Move->Amount1;
		TotalDamage = TotalDamage + DamageDealt;
	}
	GaleRoundsLeft = Move->Turns;
	Manager->ShowMessage(FString::Printf(TEXT("Gale Unleashed tears through the party for %d and lingers"), FMath::RoundToInt(TotalDamage)), FColor::Purple);
}
void ARavanaCharacter::CastVeil(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	AddTurnModifier(this, MoveId, ECharacterStat::IncomingDamage, EModifierKind::Percent, Move->ModAmount, Move->Turns);
	AddTurnModifier(this, MoveId, ECharacterStat::Reflect, EModifierKind::Flat, Move->Amount1, Move->Turns);
	if (IsChimeraAlive())
	{
		AddTurnModifier(Chimera, MoveId, ECharacterStat::IncomingDamage, EModifierKind::Percent, Move->ModAmount, Move->Turns);
		AddTurnModifier(Chimera, MoveId, ECharacterStat::Reflect, EModifierKind::Flat, Move->Amount1, Move->Turns);
	}
	Manager->ShowMessage(TEXT("A veil of wind wraps S. Ravana and the Chimera"), FColor::Purple);
}
void ARavanaCharacter::CastOmnichannel(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	AddTurnModifier(this, MoveId, ECharacterStat::Damage, EModifierKind::Percent, Move->ModAmount, Move->Turns);
	AddTurnModifier(this, MoveId, ECharacterStat::Defense, EModifierKind::Percent, Move->Amount1, Move->Turns);
	Manager->ShowMessage(TEXT("S. Ravana channels every wind at once: attack and defense rise"), FColor::Purple);
}
void ARavanaCharacter::CastWindWall(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	float SuccessChance = 1.0f - (Move->Amount2 * WallUses);
	if (SuccessChance < Move->Amount3)
	{
		SuccessChance = Move->Amount3;
	}
	WallUses = WallUses + 1;
	if (FMath::FRand() >= SuccessChance)
	{
		Manager->ShowMessage(TEXT("The wind wall falls apart before it forms"), FColor::Green);
		return;
	}
	bWallUp = true;
	WallConvert = Move->Amount1;
	Manager->ShowMessage(TEXT("A wall of wind rises around S. Ravana"), FColor::Purple);
}
void ARavanaCharacter::CastWindsOfWar(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	AddTurnModifier(this, MoveId, ECharacterStat::Damage, EModifierKind::Percent, Move->ModAmount, Move->Turns);
	if (IsChimeraAlive())
	{
		AddTurnModifier(Chimera, MoveId, ECharacterStat::Damage, EModifierKind::Percent, Move->ModAmount, Move->Turns);
	}
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		AddTurnModifier(Manager->GetPartyMember(i), MoveId, ECharacterStat::Damage, EModifierKind::Percent, Move->ModAmount, Move->Turns);
	}
	Manager->ShowMessage(TEXT("Winds of war: everyone on the field hits harder"), FColor::Purple);
}
void ARavanaCharacter::CastSubmission(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex)
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
	AddTurnModifier(Target, MoveId, ECharacterStat::Damage, EModifierKind::Percent, Move->ModAmount, Move->Turns);
	AddTurnModifier(Target, MoveId, ECharacterStat::Defense, EModifierKind::Percent, Move->Amount1, Move->Turns);
	AddSaturn(Manager, TargetIndex, 1);
	Manager->ShowMessage(FString::Printf(TEXT("%s is forced into submission: attack and defense fall"), *Target->DisplayName), FColor::Purple);
}
void ARavanaCharacter::CastDominion(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	float HealFraction = Move->Amount1 - (Move->Amount2 * DominionUses);
	if (HealFraction < Move->Amount3)
	{
		HealFraction = Move->Amount3;
	}
	DominionUses = DominionUses + 1;
	DominionHeal = HealFraction;
	bDominionPending = true;
	Manager->ShowMessage(TEXT("S. Ravana claims dominion: his next attack will restore him"), FColor::Purple);
}
