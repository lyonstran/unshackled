#include "HanuboyCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Battle/BattleMoveRow.h"
#include "Characters/BossCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
void AHanuboyCharacter::PerformBattleMove(ATurnBattleManager* Manager, FName MoveId, const FBattleMoveRow* Move, int32 TargetIndex)
{
        if (Manager == nullptr)
        {
                return;
        }
        if (Move == nullptr)
        {
                return;
        }
        if (MoveId == FName("Hanuboy_Attack"))
        {
                CastChainBall(Manager, Move);
        }
        else if (MoveId == FName("Hanuboy_Sprout"))
        {
                CastSprout(Move);
        }
        else if (MoveId == FName("Hanuboy_Blossom"))
        {
                CastBlossom(Manager, Move);
        }
        else if (MoveId == FName("Hanuboy_Harvest"))
        {
                CastHarvest(Manager, Move);
        }
        else if (MoveId == FName("Hanuboy_Territory"))
        {
                CastTerritory(Move);
        }
        else if (MoveId == FName("Hanuboy_Squeak"))
        {
                CastSqueak(Manager, Move, TargetIndex);
        }
        else if (MoveId == FName("Hanuboy_LoudSqueak"))
        {
                CastLoudSqueak(Manager, Move);
        }
        else if (MoveId == FName("Hanuboy_Shelter"))
        {
                CastShelter(Manager, Move, TargetIndex);
        }
        else
        {
                Super::PerformBattleMove(Manager, MoveId, Move, TargetIndex);
        }
}
bool AHanuboyCharacter::CanUseBattleMove(FName MoveId) const
{
        if (MoveId == FName("Hanuboy_Harvest"))
        {
                if (CountAllSpawns() <= 0)
                {
                        return false;
                }
        }
        return Super::CanUseBattleMove(MoveId);
}
FString AHanuboyCharacter::GetBattleCardNote() const
{
        FString Note = Super::GetBattleCardNote();
        int32 Flowers = CountSpawnsOfKind(EHanuboySpawnKind::Flower);
        int32 Mushrooms = CountSpawnsOfKind(EHanuboySpawnKind::Mushroom);
        if (Flowers > 0)
        {
                Note = Note + FString::Printf(TEXT("Flower %d "), Flowers);
        }
        if (Mushrooms > 0)
        {
                Note = Note + FString::Printf(TEXT("Shroom %d "), Mushrooms);
        }
        if (TreeRoundsLeft > 0)
        {
                Note = Note + FString::Printf(TEXT("Tree %d"), TreeRoundsLeft);
        }
        return Note;
}
void AHanuboyCharacter::CastChainBall(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        HitBattleBoss(Manager, Move->Power, false);
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss == nullptr)
        {
                return;
        }
        if (Boss->Health->IsDead())
        {
                return;
        }
        float Roll = FMath::FRand();
        if (Roll < Move->Amount1)
        {
                Manager->SkipBossTurns(1);
                ShowBattleMessage(TEXT("The chain ball dazes the boss"), FColor::Green);
        }
}
void AHanuboyCharacter::CastSprout(const FBattleMoveRow* Move)
{
        SproutHealFraction = Move->Amount1;
        AddFragileSpawn(EHanuboySpawnKind::Flower, FMath::RoundToInt(Move->Amount2), Move->Turns);
        int32 Flowers = CountSpawnsOfKind(EHanuboySpawnKind::Flower);
        ShowBattleMessage(FString::Printf(TEXT("Hanuboy plants a flower (%d up)"), Flowers), FColor::Green);
}
void AHanuboyCharacter::CastTerritory(const FBattleMoveRow* Move)
{
        TerritoryManaAmount = Move->Amount1;
        AddFragileSpawn(EHanuboySpawnKind::Mushroom, FMath::RoundToInt(Move->Amount2), Move->Turns);
        int32 Mushrooms = CountSpawnsOfKind(EHanuboySpawnKind::Mushroom);
        ShowBattleMessage(FString::Printf(TEXT("Hanuboy grows a mushroom (%d up)"), Mushrooms), FColor::Green);
}
void AHanuboyCharacter::AddFragileSpawn(EHanuboySpawnKind Kind, int32 MaxOfKind, int32 Rounds)
{
        if (MaxOfKind < 1)
        {
                MaxOfKind = 1;
        }
        if (CountSpawnsOfKind(Kind) >= MaxOfKind)
        {
                int32 OldestIndex = FindOldestOfKind(Kind);
                if (OldestIndex >= 0)
                {
                        FragileSpawns.RemoveAt(OldestIndex);
                }
        }
        FHanuboySpawn NewSpawn;
        NewSpawn.Kind = Kind;
        NewSpawn.RoundsLeft = Rounds;
        FragileSpawns.Add(NewSpawn);
}
int32 AHanuboyCharacter::CountSpawnsOfKind(EHanuboySpawnKind Kind) const
{
        int32 Count = 0;
        for (int32 i = 0; i < FragileSpawns.Num(); i++)
        {
                if (FragileSpawns[i].Kind == Kind)
                {
                        Count = Count + 1;
                }
        }
        return Count;
}
int32 AHanuboyCharacter::FindOldestOfKind(EHanuboySpawnKind Kind) const
{
        for (int32 i = 0; i < FragileSpawns.Num(); i++)
        {
                if (FragileSpawns[i].Kind == Kind)
                {
                        return i;
                }
        }
        return -1;
}
int32 AHanuboyCharacter::CountAllSpawns() const
{
        int32 Count = FragileSpawns.Num() + Bombs.Num();
        if (TreeRoundsLeft > 0)
        {
                Count = Count + 1;
        }
        return Count;
}
void AHanuboyCharacter::CastBlossom(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        float DamageDealt = 0.0f;
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss != nullptr)
        {
                if (!Boss->Health->IsDead())
                {
                        float Damage = Boss->Health->MaxHealth * Move->Amount1;
                        DamageDealt = Boss->ApplyHit(Damage, true, this);
                }
        }
        TreeRoundsLeft = Move->Turns;
        TreeTickFraction = Move->Amount2;
        SetAllyDebuffScale(Manager, Move->Amount3);
        ShowBattleMessage(FString::Printf(TEXT("A tree bursts up for %d; debuffs on allies are softened"), FMath::RoundToInt(DamageDealt)), FColor::Green);
}
void AHanuboyCharacter::TickTree(ATurnBattleManager* Manager)
{
        if (TreeRoundsLeft <= 0)
        {
                return;
        }
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss != nullptr)
        {
                if (!Boss->Health->IsDead())
                {
                        float Damage = Boss->Health->MaxHealth * TreeTickFraction;
                        float DamageDealt = Boss->ApplyHit(Damage, true, this);
                        ShowBattleMessage(FString::Printf(TEXT("The tree strikes the boss for %d"), FMath::RoundToInt(DamageDealt)), FColor::Green);
                }
        }
        TreeRoundsLeft = TreeRoundsLeft - 1;
        if (TreeRoundsLeft <= 0)
        {
                EndTree(Manager);
        }
}
void AHanuboyCharacter::EndTree(ATurnBattleManager* Manager)
{
        TreeRoundsLeft = 0;
        SetAllyDebuffScale(Manager, 1.0f);
        ShowBattleMessage(TEXT("The tree withers"), FColor::Green);
}
void AHanuboyCharacter::SetAllyDebuffScale(ATurnBattleManager* Manager, float Scale)
{
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        Ally->Stats->DebuffScale = Scale;
                }
        }
}
void AHanuboyCharacter::CastHarvest(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        int32 SpawnCount = CountAllSpawns();
        FragileSpawns.Empty();
        Bombs.Empty();
        if (TreeRoundsLeft > 0)
        {
                EndTree(Manager);
        }
        if (SpawnCount <= 0)
        {
                ShowBattleMessage(TEXT("Nothing to harvest"), FColor::Orange);
                return;
        }
        float Bonus = SpawnCount * Move->Amount1;
        AddBattleModifier(this, FName("HanuboyHarvest"), ECharacterStat::Damage, EModifierKind::Percent, Bonus, Move->Turns);
        ShowBattleMessage(FString::Printf(TEXT("Harvest: %d spawns give +%d%% damage"), SpawnCount, FMath::RoundToInt(Bonus * 100.0f)), FColor::Green);
}
void AHanuboyCharacter::CastSqueak(ATurnBattleManager* Manager, const FBattleMoveRow* Move, int32 TargetIndex)
{
        APartyMemberCharacter* Ally = Manager->GetPartyMember(TargetIndex);
        if (Ally == nullptr)
        {
                return;
        }
        if (Ally->Health->IsDead())
        {
                return;
        }
        AddBattleModifier(Ally, FName("HanuboySqueak"), ECharacterStat::Defense, EModifierKind::Percent, Move->Amount1, Move->Turns);
        ShowBattleMessage(FString::Printf(TEXT("Squeak: %s's defense rises"), *Ally->DisplayName), FColor::Green);
}
void AHanuboyCharacter::CastLoudSqueak(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        FString Summary = TEXT("Super Loud Squeak heals:");
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        if (!Ally->Health->IsDead())
                        {
                                float Fraction = FMath::FRandRange(Move->Amount1, Move->Amount2);
                                float HealAmount = Ally->Health->MaxHealth * Fraction;
                                Ally->Health->Heal(HealAmount);
                                Summary = Summary + FString::Printf(TEXT(" %s +%d"), *Ally->DisplayName, FMath::RoundToInt(HealAmount));
                        }
                }
        }
        ShowBattleMessage(Summary, FColor::Green);
}
void AHanuboyCharacter::CastShelter(ATurnBattleManager* Manager, const FBattleMoveRow* Move, int32 TargetIndex)
{
        APartyMemberCharacter* Ally = Manager->GetPartyMember(TargetIndex);
        if (Ally == nullptr)
        {
                return;
        }
        if (Ally->Health->IsDead())
        {
                return;
        }
        if (Ally == this)
        {
                Stats->RestoreMana(Move->ManaCost);
                ShowBattleMessage(TEXT("Hanuboy cannot shelter himself (mana refunded)"), FColor::Orange);
                return;
        }
        Ally->SetBattleShelter(this, Move->Amount1);
        ShowBattleMessage(FString::Printf(TEXT("Hanuboy shelters %s this round"), *Ally->DisplayName), FColor::Green);
}
void AHanuboyCharacter::ClearMyShelters(ATurnBattleManager* Manager)
{
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        if (Ally->GetShelterGuardian() == this)
                        {
                                Ally->ClearBattleShelter();
                        }
                }
        }
}
float AHanuboyCharacter::FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker)
{
        float Result = Super::FilterIncomingDamage(RawDamage, bTrueDamage, Attacker);
        if (Result <= 0.0f)
        {
                return Result;
        }
        if (Attacker == nullptr)
        {
                return Result;
        }
        if (!Attacker->bIsEnemy)
        {
                return Result;
        }
        if (FragileSpawns.Num() == 0)
        {
                return Result;
        }
        FString SpawnName = TEXT("flower");
        if (FragileSpawns[0].Kind == EHanuboySpawnKind::Mushroom)
        {
                SpawnName = TEXT("mushroom");
        }
        FragileSpawns.RemoveAt(0);
        ShowBattleMessage(FString::Printf(TEXT("A %s takes the hit for Hanuboy and breaks"), *SpawnName), FColor::Orange);
        return 0.0f;
}
void AHanuboyCharacter::OnBattleRoundEnd(ATurnBattleManager* Manager)
{
        Super::OnBattleRoundEnd(Manager);
        GiveSpawnMana();
        TickFragileSpawns(Manager);
        AgeFragileSpawns();
        TickTree(Manager);
        ClearMyShelters(Manager);
}
void AHanuboyCharacter::GiveSpawnMana()
{
        if (Health->IsDead())
        {
                return;
        }
        int32 SpawnCount = CountAllSpawns();
        if (SpawnCount <= 0)
        {
                return;
        }
        float MaxMana = Stats->GetFinal(ECharacterStat::MaxMana);
        float ManaGained = MaxMana * SpawnManaFraction * SpawnCount;
        Stats->RestoreMana(ManaGained);
}
void AHanuboyCharacter::TickFragileSpawns(ATurnBattleManager* Manager)
{
        int32 Flowers = CountSpawnsOfKind(EHanuboySpawnKind::Flower);
        int32 Mushrooms = CountSpawnsOfKind(EHanuboySpawnKind::Mushroom);
        for (int32 i = 0; i < Flowers; i++)
        {
                HealAllAllies(Manager, SproutHealFraction);
        }
        for (int32 i = 0; i < Mushrooms; i++)
        {
                GiveManaToAllAllies(Manager, TerritoryManaAmount);
        }
        if (Flowers > 0)
        {
                ShowBattleMessage(FString::Printf(TEXT("%d flower(s) heal the party"), Flowers), FColor::Green);
        }
        if (Mushrooms > 0)
        {
                ShowBattleMessage(FString::Printf(TEXT("%d mushroom(s) restore the party's mana"), Mushrooms), FColor::Green);
        }
}
void AHanuboyCharacter::HealAllAllies(ATurnBattleManager* Manager, float MaxHealthFraction)
{
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        if (!Ally->Health->IsDead())
                        {
                                Ally->Health->Heal(Ally->Health->MaxHealth * MaxHealthFraction);
                        }
                }
        }
}
void AHanuboyCharacter::GiveManaToAllAllies(ATurnBattleManager* Manager, float ManaAmount)
{
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        if (!Ally->Health->IsDead())
                        {
                                Ally->Stats->RestoreMana(ManaAmount);
                        }
                }
        }
}
void AHanuboyCharacter::AgeFragileSpawns()
{
        TArray<FHanuboySpawn> StillStanding;
        int32 ExpiredCount = 0;
        for (int32 i = 0; i < FragileSpawns.Num(); i++)
        {
                FHanuboySpawn Current = FragileSpawns[i];
                Current.RoundsLeft = Current.RoundsLeft - 1;
                if (Current.RoundsLeft > 0)
                {
                        StillStanding.Add(Current);
                }
                else
                {
                        ExpiredCount = ExpiredCount + 1;
                }
        }
        FragileSpawns = StillStanding;
        if (ExpiredCount > 0)
        {
                ShowBattleMessage(FString::Printf(TEXT("%d of Hanuboy's spawns wilt away"), ExpiredCount), FColor::Green);
        }
}
void AHanuboyCharacter::OnBattlePlayerPhaseStart(ATurnBattleManager* Manager)
{
        Super::OnBattlePlayerPhaseStart(Manager);
        DetonateBombs(Manager);
        if (!Health->IsDead())
        {
                DropBomb(Manager);
        }
}
void AHanuboyCharacter::DetonateBombs(ATurnBattleManager* Manager)
{
        TArray<FHanuboyBomb> BombsToDetonate = Bombs;
        Bombs.Empty();
        for (int32 i = 0; i < BombsToDetonate.Num(); i++)
        {
                DetonateBomb(Manager, BombsToDetonate[i]);
        }
}
void AHanuboyCharacter::DetonateBomb(ATurnBattleManager* Manager, FHanuboyBomb Bomb)
{
        if (Bomb.bOnBoss)
        {
                ABossCharacter* Boss = Manager->GetBoss();
                if (Boss == nullptr)
                {
                        return;
                }
                if (Boss->Health->IsDead())
                {
                        return;
                }
                float Damage = Boss->Health->CurrentHealth * BombBossDamageFraction;
                float DamageDealt = Boss->ApplyHit(Damage, true, this);
                ShowBattleMessage(FString::Printf(TEXT("The flower bomb bursts on the boss for %d"), FMath::RoundToInt(DamageDealt)), FColor::Green);
                return;
        }
        APartyMemberCharacter* Ally = Manager->GetPartyMember(Bomb.AllyIndex);
        if (Ally == nullptr)
        {
                return;
        }
        if (Ally->Health->IsDead())
        {
                return;
        }
        float HealAmount = Ally->Health->CurrentHealth * BombAllyHealFraction;
        Ally->Health->Heal(HealAmount);
        ShowBattleMessage(FString::Printf(TEXT("The flower bomb blooms on %s (+%d)"), *Ally->DisplayName, FMath::RoundToInt(HealAmount)), FColor::Green);
}
void AHanuboyCharacter::DropBomb(ATurnBattleManager* Manager)
{
        FHanuboyBomb NewBomb;
        FString TargetName;
        float Roll = FMath::FRand();
        if (Roll < BombBossChance)
        {
                ABossCharacter* Boss = Manager->GetBoss();
                if (Boss == nullptr)
                {
                        return;
                }
                if (Boss->Health->IsDead())
                {
                        return;
                }
                NewBomb.bOnBoss = true;
                TargetName = Boss->DisplayName;
        }
        else
        {
                int32 AllyIndex = Manager->PickRandomLivingMember();
                if (AllyIndex < 0)
                {
                        return;
                }
                NewBomb.AllyIndex = AllyIndex;
                TargetName = Manager->GetPartyMember(AllyIndex)->DisplayName;
        }
        Bombs.Add(NewBomb);
        ShowBattleMessage(FString::Printf(TEXT("A flower bomb drops on %s (goes off next round)"), *TargetName), FColor::Green);
}
void AHanuboyCharacter::PerformBattleRetribution(ATurnBattleManager* Manager)
{
        Super::PerformBattleRetribution(Manager);
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        if (!Ally->Health->IsDead())
                        {
                                FHanuboyBomb AllyBomb;
                                AllyBomb.AllyIndex = i;
                                Bombs.Add(AllyBomb);
                        }
                }
        }
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss != nullptr)
        {
                if (!Boss->Health->IsDead())
                {
                        FHanuboyBomb BossBomb;
                        BossBomb.bOnBoss = true;
                        Bombs.Add(BossBomb);
                }
        }
        ShowBattleMessage(TEXT("Hanuboy's retribution: flower bombs drop on everyone"), FColor::Green);
}
