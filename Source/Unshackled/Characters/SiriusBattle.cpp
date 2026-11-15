#include "SiriusCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Battle/BattleMoveRow.h"
#include "Characters/BossCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
void ASiriusCharacter::PerformBattleMove(ATurnBattleManager* Manager, FName MoveId, const FBattleMoveRow* Move, int32 TargetIndex)
{
        if (Manager == nullptr)
        {
                return;
        }
        if (Move == nullptr)
        {
                return;
        }
        float EnemyHealthBefore = Manager->GetTotalEnemyHealth();
        if (MoveId == FName("Sirius_Veil"))
        {
                CastBattleVeil(Manager, Move);
        }
        else if (MoveId == FName("Sirius_WindsUnbound"))
        {
                CastWindsUnbound(Move);
        }
        else if (MoveId == FName("Sirius_GaleUnleashed"))
        {
                CastGaleUnleashed(Manager, Move);
        }
        else if (MoveId == FName("Sirius_WindWall"))
        {
                CastWindWall(Move);
        }
        else if (MoveId == FName("Sirius_WindsOfWar"))
        {
                CastWindsOfWar(Manager, Move);
        }
        else if (MoveId == FName("Sirius_Submission"))
        {
                CastSubmission(Manager, Move, TargetIndex);
        }
        else
        {
                Super::PerformBattleMove(Manager, MoveId, Move, TargetIndex);
        }
        TArray<AUnshackledCharacterBase*> TouchedEnemies = Manager->GetMoveEnemyTargets(Move, TargetIndex);
        for (int32 i = 0; i < TouchedEnemies.Num(); i++)
        {
                AddMarkTo(Manager, TouchedEnemies[i]);
                if (Move->Power > 0.0f)
                {
                        TryBattleExecute(Manager, TouchedEnemies[i]);
                }
        }
        TryBattleRetributionHeal(Manager, EnemyHealthBefore);
}
void ASiriusCharacter::TryBattleRetributionHeal(ATurnBattleManager* Manager, float EnemyHealthBefore)
{
        if (!bBattleRetributionHealReady)
        {
                return;
        }
        float EnemyHealthAfter = Manager->GetTotalEnemyHealth();
        if (EnemyHealthAfter >= EnemyHealthBefore)
        {
                return;
        }
        bBattleRetributionHealReady = false;
        float HealAmount = Health->MaxHealth * BattleRetributionHealFraction;
        Health->Heal(HealAmount);
        ShowBattleMessage(FString::Printf(TEXT("Sirius's retribution heals him for %d"), FMath::RoundToInt(HealAmount)), FColor::Cyan);
}
void ASiriusCharacter::PerformBattleRetribution(ATurnBattleManager* Manager)
{
        Super::PerformBattleRetribution(Manager);
        bBattleRetributionHealReady = true;
        ShowBattleMessage(TEXT("Sirius: his next damaging move will heal him"), FColor::Cyan);
}
void ASiriusCharacter::AddMarkTo(ATurnBattleManager* Manager, AUnshackledCharacterBase* Enemy)
{
        if (Enemy == nullptr)
        {
                return;
        }
        if (Enemy->Health->IsDead())
        {
                return;
        }
        FName MarkId = FName("SiriusMarks");
        int32 StacksBefore = Enemy->Stats->GetStacks(MarkId, ECharacterStat::Damage, EModifierKind::Percent);
        FStatModifier Mark;
        Mark.SourceId = MarkId;
        Mark.Stat = ECharacterStat::Damage;
        Mark.Kind = EModifierKind::Percent;
        Mark.Amount = 0.0f - MarkDamageReductionPerStack;
        Mark.Duration = 0.0f;
        Mark.TurnsLeft = 0;
        Mark.Rule = EStackRule::StackToMax;
        Mark.MaxStacks = MarkMaxStacks;
        Enemy->Stats->AddModifier(Mark);
        int32 StacksAfter = Enemy->Stats->GetStacks(MarkId, ECharacterStat::Damage, EModifierKind::Percent);
        if (StacksAfter >= MarkMaxStacks)
        {
                if (StacksBefore < MarkMaxStacks)
                {
                        Manager->ShowMessage(TEXT("Sirius's marks are full"), FColor::Cyan);
                }
        }
}
void ASiriusCharacter::TryBattleExecute(ATurnBattleManager* Manager, AUnshackledCharacterBase* Enemy)
{
        if (Enemy == nullptr)
        {
                return;
        }
        if (Enemy->Health->IsDead())
        {
                return;
        }
        int32 Stacks = Enemy->Stats->GetStacks(FName("SiriusMarks"), ECharacterStat::Damage, EModifierKind::Percent);
        if (Stacks < MarkMaxStacks)
        {
                return;
        }
        float MaxHealthValue = Enemy->Health->MaxHealth;
        if (MaxHealthValue <= 0.0f)
        {
                return;
        }
        float HealthFraction = Enemy->Health->CurrentHealth / MaxHealthValue;
        if (HealthFraction < MarkExecuteThreshold)
        {
                Enemy->ApplyHit(ExecuteDamage, true, this);
                Manager->ShowMessage(TEXT("Sirius executes his target!"), FColor::Cyan);
        }
}
void ASiriusCharacter::CastBattleVeil(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        AddBattleModifier(Ally, FName("SiriusBattleVeil"), ECharacterStat::IncomingDamage, EModifierKind::Percent, 0.0f - Move->Amount1, Move->Turns);
                        AddBattleModifier(Ally, FName("SiriusBattleVeilReflect"), ECharacterStat::Reflect, EModifierKind::Flat, Move->Amount2, Move->Turns);
                }
        }
}
void ASiriusCharacter::CastWindsUnbound(const FBattleMoveRow* Move)
{
        AddBattleModifier(this, FName("SiriusWindsUnbound"), ECharacterStat::Damage, EModifierKind::Percent, Move->Amount1, Move->Turns);
}
void ASiriusCharacter::CastGaleUnleashed(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        int32 HitCount = Manager->CountLivingEnemies();
        float DamageDealt = HitAllBattleEnemies(Manager, Move->Power, false);
        BattleTornadoDamage = 0.0f;
        if (HitCount > 0)
        {
                BattleTornadoDamage = (DamageDealt / (float)HitCount) * Move->Amount1;
        }
        BattleTornadoRoundsLeft = Move->Turns;
}
void ASiriusCharacter::OnBattleRoundEnd(ATurnBattleManager* Manager)
{
        Super::OnBattleRoundEnd(Manager);
        TickBattleTornado(Manager);
}
void ASiriusCharacter::TickBattleTornado(ATurnBattleManager* Manager)
{
        if (BattleTornadoRoundsLeft <= 0)
        {
                return;
        }
        BattleTornadoRoundsLeft = BattleTornadoRoundsLeft - 1;
        TArray<AUnshackledCharacterBase*> Targets = Manager->GetLivingEnemies();
        float DamageDealt = 0.0f;
        for (int32 i = 0; i < Targets.Num(); i++)
        {
                DamageDealt = DamageDealt + Targets[i]->ApplyHit(BattleTornadoDamage, true, this);
        }
        Manager->ShowMessage(FString::Printf(TEXT("The tornado lingers for %d"), FMath::RoundToInt(DamageDealt)), FColor::Cyan);
}
void ASiriusCharacter::CastWindWall(const FBattleMoveRow* Move)
{
        float SuccessChance = 1.0f - (WindWallUses * Move->Amount2);
        WindWallUses = WindWallUses + 1;
        float Roll = FMath::FRand();
        if (Roll >= SuccessChance)
        {
                ShowBattleMessage(TEXT("Wind Wall failed"), FColor::Orange);
                return;
        }
        bWindWallUp = true;
        WindWallManaFraction = Move->Amount1;
        ShowBattleMessage(TEXT("Wind Wall is up"), FColor::Cyan);
}
float ASiriusCharacter::FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker)
{
        float Result = Super::FilterIncomingDamage(RawDamage, bTrueDamage, Attacker);
        if (!bWindWallUp)
        {
                return Result;
        }
        if (Result <= 0.0f)
        {
                return Result;
        }
        float DamageItWouldDo = CalculateFinalDamage(Result, bTrueDamage);
        float ManaGained = DamageItWouldDo * WindWallManaFraction;
        Stats->RestoreMana(ManaGained);
        bWindWallUp = false;
        ShowBattleMessage(FString::Printf(TEXT("Wind Wall stops the hit (+%d mana)"), FMath::RoundToInt(ManaGained)), FColor::Cyan);
        return 0.0f;
}
void ASiriusCharacter::CastWindsOfWar(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        FName SourceId = FName("SiriusWindsOfWar");
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        AddBattleModifier(Ally, SourceId, ECharacterStat::Damage, EModifierKind::Percent, Move->Amount1, Move->Turns);
                }
        }
        TArray<AUnshackledCharacterBase*> Targets = Manager->GetLivingEnemies();
        for (int32 i = 0; i < Targets.Num(); i++)
        {
                AddBattleModifier(Targets[i], SourceId, ECharacterStat::Damage, EModifierKind::Percent, Move->Amount1, Move->Turns);
        }
}
void ASiriusCharacter::CastSubmission(ATurnBattleManager* Manager, const FBattleMoveRow* Move, int32 TargetIndex)
{
        AUnshackledCharacterBase* Target = Manager->GetTargetEnemy(TargetIndex);
        if (Target == nullptr)
        {
                return;
        }
        FName SourceId = FName("SiriusSubmission");
        AddBattleModifier(Target, SourceId, ECharacterStat::Damage, EModifierKind::Percent, 0.0f - Move->Amount1, Move->Turns);
        AddBattleModifier(Target, SourceId, ECharacterStat::Defense, EModifierKind::Percent, 0.0f - Move->Amount1, Move->Turns);
}