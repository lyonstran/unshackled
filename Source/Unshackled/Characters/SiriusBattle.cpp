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
                CastSubmission(Manager, Move);
        }
        else
        {
                Super::PerformBattleMove(Manager, MoveId, Move, TargetIndex);
        }
        if (MoveTouchesEnemy(Move))
        {
                AddMarkToBoss(Manager);
        }
        if (Move->Power > 0.0f)
        {
                TryBattleExecute(Manager);
        }
}
bool ASiriusCharacter::MoveTouchesEnemy(const FBattleMoveRow* Move) const
{
        if (Move->TargetKind == EBattleTargetKind::Enemy)
        {
                return true;
        }
        if (Move->TargetKind == EBattleTargetKind::AllEnemies)
        {
                return true;
        }
        if (Move->TargetKind == EBattleTargetKind::Everyone)
        {
                return true;
        }
        return false;
}
void ASiriusCharacter::AddMarkToBoss(ATurnBattleManager* Manager)
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
        FName MarkId = FName("SiriusMarks");
        int32 StacksBefore = Boss->Stats->GetStacks(MarkId, ECharacterStat::Damage, EModifierKind::Percent);
        FStatModifier Mark;
        Mark.SourceId = MarkId;
        Mark.Stat = ECharacterStat::Damage;
        Mark.Kind = EModifierKind::Percent;
        Mark.Amount = 0.0f - MarkDamageReductionPerStack;
        Mark.Duration = 0.0f;
        Mark.TurnsLeft = 0;
        Mark.Rule = EStackRule::StackToMax;
        Mark.MaxStacks = MarkMaxStacks;
        Boss->Stats->AddModifier(Mark);
        int32 StacksAfter = Boss->Stats->GetStacks(MarkId, ECharacterStat::Damage, EModifierKind::Percent);
        if (StacksAfter >= MarkMaxStacks)
        {
                if (StacksBefore < MarkMaxStacks)
                {
                        Manager->ShowMessage(TEXT("Sirius's marks are full"), FColor::Cyan);
                }
        }
}
void ASiriusCharacter::TryBattleExecute(ATurnBattleManager* Manager)
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
        int32 Stacks = Boss->Stats->GetStacks(FName("SiriusMarks"), ECharacterStat::Damage, EModifierKind::Percent);
        if (Stacks < MarkMaxStacks)
        {
                return;
        }
        float MaxHealthValue = Boss->Health->MaxHealth;
        if (MaxHealthValue <= 0.0f)
        {
                return;
        }
        float HealthFraction = Boss->Health->CurrentHealth / MaxHealthValue;
        if (HealthFraction < MarkExecuteThreshold)
        {
                Boss->ApplyHit(ExecuteDamage, true, this);
                Manager->ShowMessage(TEXT("Sirius executes the boss!"), FColor::Cyan);
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
        float DamageDealt = HitBattleBoss(Manager, Move->Power, false);
        BattleTornadoDamage = DamageDealt * Move->Amount1;
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
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss == nullptr)
        {
                return;
        }
        if (Boss->Health->IsDead())
        {
                return;
        }
        float DamageDealt = Boss->ApplyHit(BattleTornadoDamage, true, this);
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
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss != nullptr)
        {
                AddBattleModifier(Boss, SourceId, ECharacterStat::Damage, EModifierKind::Percent, Move->Amount1, Move->Turns);
        }
}
void ASiriusCharacter::CastSubmission(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss == nullptr)
        {
                return;
        }
        FName SourceId = FName("SiriusSubmission");
        AddBattleModifier(Boss, SourceId, ECharacterStat::Damage, EModifierKind::Percent, 0.0f - Move->Amount1, Move->Turns);
        AddBattleModifier(Boss, SourceId, ECharacterStat::Defense, EModifierKind::Percent, 0.0f - Move->Amount1, Move->Turns);
}