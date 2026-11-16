#include "Characters/SerpentCharacter.h"
#include "Characters/BattleSummonCharacter.h"
#include "Characters/PartyMemberCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
ASerpentCharacter::ASerpentCharacter()
{
	DisplayName = TEXT("Serpent");
}
void ASerpentCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateShedSkin();
}
void ASerpentCharacter::OnBattleRoundStart(ATurnBattleManager* Manager)
{
	Super::OnBattleRoundStart(Manager);
	BattleManager = Manager;
	ApplyVenom(Manager);
	TickMiasma(Manager);
}
float ASerpentCharacter::GetMissingFraction(const AUnshackledCharacterBase* Target) const
{
	if (Target->Health->MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	float MissingHealth = Target->Health->MaxHealth - Target->Health->CurrentHealth;
	if (MissingHealth < 0.0f)
	{
		return 0.0f;
	}
	return MissingHealth / Target->Health->MaxHealth;
}
void ASerpentCharacter::UpdateShedSkin()
{
	if (Health->IsDead())
	{
		return;
	}
	if (!bShed)
	{
		if (GetMissingFraction(this) <= 1.0f - ShedHealthFraction)
		{
			return;
		}
		bShed = true;
		if (BattleManager != nullptr)
		{
			BattleManager->ShowMessage(TEXT("The Serpent sheds its skin!"), FColor::Purple);
		}
	}
	int32 Steps = FMath::FloorToInt(GetMissingFraction(this) * 10.0f);
	if (Steps == ShedSteps)
	{
		return;
	}
	ShedSteps = Steps;
	FStatModifier Hardened;
	Hardened.SourceId = FName("SerpentShedSkin");
	Hardened.Stat = ECharacterStat::Defense;
	Hardened.Kind = EModifierKind::Percent;
	Hardened.Amount = Steps * ShedDefensePerTenPercent;
	Hardened.Duration = 0.0f;
	Hardened.TurnsLeft = 0;
	Hardened.Rule = EStackRule::RefreshSameSource;
	Stats->AddModifier(Hardened);
}
void ASerpentCharacter::ApplyVenom(ATurnBattleManager* Manager)
{
	float VenomFraction = VenomMissingFraction;
	if (bShed)
	{
		VenomFraction = ShedVenomMissingFraction;
	}
	float TotalDamage = 0.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				float MissingHealth = Member->Health->MaxHealth - Member->Health->CurrentHealth;
				float VenomDamage = MissingHealth * VenomFraction;
				if (VenomDamage > 0.0f)
				{
					TotalDamage = TotalDamage + Member->ApplyHit(VenomDamage, true, nullptr);
				}
				ApplyWeakness(Member);
			}
		}
	}
	if (TotalDamage > 0.0f)
	{
		Manager->ShowMessage(FString::Printf(TEXT("The Serpent's venom saps the party for %d"), FMath::RoundToInt(TotalDamage)), FColor::Purple);
	}
}
void ASerpentCharacter::ApplyWeakness(APartyMemberCharacter* Member)
{
	if (Member->Health->IsDead())
	{
		return;
	}
	int32 Steps = FMath::FloorToInt(GetMissingFraction(Member) * 10.0f);
	FName WeaknessId = FName("SerpentWeakness");
	if (Steps <= 0)
	{
		Member->Stats->RemoveModifiersFromSource(WeaknessId);
		return;
	}
	FStatModifier Weakness;
	Weakness.SourceId = WeaknessId;
	Weakness.Stat = ECharacterStat::Damage;
	Weakness.Kind = EModifierKind::Percent;
	Weakness.Amount = 0.0f - (Steps * WeaknessPerTenPercent);
	Weakness.Duration = 0.0f;
	Weakness.TurnsLeft = 0;
	Weakness.Rule = EStackRule::RefreshSameSource;
	Member->Stats->AddModifier(Weakness);
}
void ASerpentCharacter::TickMiasma(ATurnBattleManager* Manager)
{
	if (MiasmaRoundsLeft <= 0)
	{
		return;
	}
	MiasmaRoundsLeft = MiasmaRoundsLeft - 1;
	float TotalDamage = 0.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				float Multiplier = 1.0f;
				if (i < MiasmaMultipliers.Num())
				{
					Multiplier = MiasmaMultipliers[i];
				}
				float RawDamage = Stats->GetFinal(ECharacterStat::Damage) * MiasmaPower * Multiplier;
				float MiasmaDamage = RawDamage * GetDefenseMitigation(Member);
				TotalDamage = TotalDamage + Member->ApplyHit(MiasmaDamage, true, nullptr);
			}
		}
	}
	Manager->ShowMessage(FString::Printf(TEXT("The miasma chokes the party for %d (%d rounds left)"), FMath::RoundToInt(TotalDamage), MiasmaRoundsLeft), FColor::Purple);
}
FString ASerpentCharacter::GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const
{
	FString Note;
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return Note;
	}
	int32 Stacks = GetDecayStacks(Member);
	if (Stacks > 0)
	{
		Note = Note + FString::Printf(TEXT("Poison %d "), Stacks);
	}
	int32 RainStacks = Member->Stats->GetStacks(FName("Serpent_Cycle"), ECharacterStat::Poison, EModifierKind::Flat);
	if (RainStacks > 0)
	{
		Note = Note + FString::Printf(TEXT("Rain %d "), RainStacks);
	}
	if (MiasmaRoundsLeft > 0)
	{
		Note = Note + TEXT("Miasma");
	}
	return Note;
}
int32 ASerpentCharacter::GetDecayStacks(APartyMemberCharacter* Member) const
{
	return Member->Stats->GetStacks(FName("Serpent_Decay"), ECharacterStat::Poison, EModifierKind::Flat);
}
int32 ASerpentCharacter::GetLowestDecayStacks(ATurnBattleManager* Manager) const
{
	int32 Lowest = 99;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				int32 Stacks = GetDecayStacks(Member);
				if (Stacks < Lowest)
				{
					Lowest = Stacks;
				}
			}
		}
	}
	return Lowest;
}
float ASerpentCharacter::GetAverageDecayStacks(ATurnBattleManager* Manager) const
{
	int32 Living = 0;
	int32 TotalStacks = 0;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				Living = Living + 1;
				TotalStacks = TotalStacks + GetDecayStacks(Member);
			}
		}
	}
	if (Living == 0)
	{
		return 0.0f;
	}
	return (float)TotalStacks / (float)Living;
}
float ASerpentCharacter::GetDrainFactor(const FBossMoveRow* Move) const
{
	float Factor = 1.0f - (DrainUses * Move->Amount3);
	if (Factor < DrainMinFactor)
	{
		Factor = DrainMinFactor;
	}
	return Factor;
}
float ASerpentCharacter::ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	float BaseScore = Super::ScoreMove(Manager, MoveId, Move);
	if (BaseScore <= 0.0f)
	{
		return 0.0f;
	}
	if (MoveId == FName("Serpent_Pounce"))
	{
		return ScorePounce(Manager, Move);
	}
	if (MoveId == FName("Serpent_Drain"))
	{
		return ScoreDrain(Manager, Move);
	}
	if (MoveId == FName("Serpent_Decay"))
	{
		return ScoreDecay(Manager, Move);
	}
	if (MoveId == FName("Serpent_Conjure"))
	{
		return ScoreConjure(Manager, Move);
	}
	if (MoveId == FName("Serpent_Beckon"))
	{
		return ScoreBeckon(Manager, Move);
	}
	if (MoveId == FName("Serpent_Miasma"))
	{
		return ScoreMiasma(Manager, Move);
	}
	if (MoveId == FName("Serpent_Cycle"))
	{
		return ScoreCycle(Manager, MoveId, Move);
	}
	return BaseScore;
}
float ASerpentCharacter::ScorePounce(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	APartyMemberCharacter* Target = Manager->GetPartyMember(FindMostWoundedMember(Manager));
	if (Target == nullptr)
	{
		return 0.0f;
	}
	float MissingPercent = GetMissingFraction(Target) * 100.0f;
	return Move->ScoreBase + (Move->ScoreBonus1 * MissingPercent);
}
float ASerpentCharacter::ScoreDrain(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	float MissingFraction = GetMissingFraction(this);
	if (1.0f - MissingFraction > Move->ScoreBonus2)
	{
		return 0.0f;
	}
	float MissingPercent = MissingFraction * 100.0f;
	return Move->ScoreBase + (Move->ScoreBonus1 * MissingPercent * GetDrainFactor(Move));
}
float ASerpentCharacter::ScoreDecay(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	int32 LowestStacks = GetLowestDecayStacks(Manager);
	if (LowestStacks <= 0)
	{
		return Move->ScoreBase;
	}
	if (LowestStacks < Move->MaxStacks)
	{
		return Move->ScoreBonus1;
	}
	return 0.0f;
}
float ASerpentCharacter::ScoreConjure(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (SnakeClass == nullptr)
	{
		return 0.0f;
	}
	int32 Snakes = Manager->CountLivingSummons();
	int32 MaxSnakes = FMath::RoundToInt(Move->Amount2);
	if (Snakes >= MaxSnakes)
	{
		return 0.0f;
	}
	return Move->ScoreBase - (Move->ScoreBonus1 * Snakes);
}
float ASerpentCharacter::ScoreBeckon(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	int32 Snakes = Manager->CountLivingSummons();
	if (Snakes <= 0)
	{
		return 0.0f;
	}
	float Score = Move->ScoreBase * Snakes;
	if (MiasmaRoundsLeft > 0)
	{
		Score = Score + Move->ScoreBonus1;
	}
	if (LastConjureRound == Manager->GetRound())
	{
		Score = Score - Move->ScoreBonus2;
	}
	if (Score < 1.0f)
	{
		Score = 1.0f;
	}
	return Score;
}
float ASerpentCharacter::ScoreMiasma(ATurnBattleManager* Manager, const FBossMoveRow* Move) const
{
	if (MiasmaRoundsLeft > 0)
	{
		return 0.0f;
	}
	float Count = Manager->CountLivingSummons() + GetAverageDecayStacks(Manager);
	return Move->ScoreBase + (Move->ScoreBonus1 * Count);
}
int32 ASerpentCharacter::PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	if (MoveId == FName("Serpent_Pounce"))
	{
		return FindMostWoundedMember(Manager);
	}
	if (MoveId == FName("Serpent_Drain"))
	{
		return FindHealthiestMember(Manager);
	}
	if (MoveId == FName("Serpent_Rasp"))
	{
		return FindStrongestMemberBelowMax(Manager, MoveId, Move);
	}
	return Super::PickMoveTarget(Manager, MoveId, Move);
}
int32 ASerpentCharacter::FindMostWoundedMember(ATurnBattleManager* Manager) const
{
	int32 BestIndex = -1;
	float BestMissing = -1.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				float MissingHealth = Member->Health->MaxHealth - Member->Health->CurrentHealth;
				if (MissingHealth > BestMissing)
				{
					BestMissing = MissingHealth;
					BestIndex = i;
				}
			}
		}
	}
	return BestIndex;
}
int32 ASerpentCharacter::FindHealthiestMember(ATurnBattleManager* Manager) const
{
	int32 BestIndex = -1;
	float BestHealth = -1.0f;
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			if (!Member->Health->IsDead())
			{
				if (Member->Health->CurrentHealth > BestHealth)
				{
					BestHealth = Member->Health->CurrentHealth;
					BestIndex = i;
				}
			}
		}
	}
	return BestIndex;
}
int32 ASerpentCharacter::FindStrongestMemberBelowMax(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const
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
				if (!IsAtMaxStacks(Member, MoveId, Move))
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
void ASerpentCharacter::PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex)
{
	if (MoveId == FName("Serpent_Drain"))
	{
		CastDrain(Manager, Move, TargetIndex);
	}
	else if (MoveId == FName("Serpent_Conjure"))
	{
		CastConjure(Manager, Move);
	}
	else if (MoveId == FName("Serpent_Beckon"))
	{
		CastBeckon(Manager, Move);
	}
	else if (MoveId == FName("Serpent_Miasma"))
	{
		CastMiasma(Manager, Move);
	}
	else if (MoveId == FName("Serpent_Cycle"))
	{
		CastCycle(Manager, MoveId);
	}
	else
	{
		Super::PerformMove(Manager, MoveId, Move, TargetIndex);
	}
}
void ASerpentCharacter::CastDrain(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex)
{
	HitMemberWithMove(Manager, TargetIndex, Move, 1.0f);
	float MissingHealth = Health->MaxHealth - Health->CurrentHealth;
	float HealAmount = ((Health->MaxHealth * Move->Amount1) + (MissingHealth * Move->Amount2)) * GetDrainFactor(Move);
	Health->Heal(HealAmount);
	DrainUses = DrainUses + 1;
	Manager->ShowMessage(FString::Printf(TEXT("The Serpent drinks in %d health"), FMath::RoundToInt(HealAmount)), FColor::Purple);
}
void ASerpentCharacter::CastConjure(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	LastConjureRound = Manager->GetRound();
	float SnakeHealth = Health->CurrentHealth * Move->Amount1;
	Manager->SpawnSummon(SnakeClass, SnakeHealth);
}
void ASerpentCharacter::CastBeckon(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	for (int32 i = 0; i < Manager->GetEnemyCount(); i++)
	{
		if (Manager->IsEnemyAlive(i))
		{
			ABattleSummonCharacter* Snake = Cast<ABattleSummonCharacter>(Manager->GetEnemy(i));
			if (Snake != nullptr)
			{
				int32 TargetIndex = Manager->PickRandomLivingMember();
				APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
				if (Target != nullptr)
				{
					float DamageDealt = HitMemberWithMove(Manager, TargetIndex, Move, 1.0f);
					Manager->ShowMessage(FString::Printf(TEXT("%s explodes on %s for %d"), *Snake->DisplayName, *Target->DisplayName, FMath::RoundToInt(DamageDealt)), FColor::Purple);
				}
				Snake->DieInBattle();
			}
		}
	}
}
void ASerpentCharacter::CastMiasma(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	MiasmaRoundsLeft = Move->Turns;
	MiasmaPower = Move->Power;
	MiasmaMultipliers.Empty();
	int32 Snakes = Manager->CountLivingSummons();
	for (int32 i = 0; i < Manager->GetPartyCount(); i++)
	{
		float Multiplier = 1.0f;
		APartyMemberCharacter* Member = Manager->GetPartyMember(i);
		if (Member != nullptr)
		{
			int32 Count = Snakes + GetDecayStacks(Member) + 1;
			float Bonus = Move->Amount1 + (Move->Amount2 * Count);
			Multiplier = 1.0f + Bonus;
		}
		MiasmaMultipliers.Add(Multiplier);
	}
	Manager->ShowMessage(TEXT("A choking miasma settles over the party"), FColor::Purple);
}