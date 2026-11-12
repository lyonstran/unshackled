#include "MalkholmCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Battle/BattleMoveRow.h"
#include "Characters/BossCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
void AMalkholmCharacter::PerformBattleMove(ATurnBattleManager* Manager, FName MoveId, const FBattleMoveRow* Move, int32 TargetIndex)
{
        if (Manager == nullptr)
        {
                return;
        }
        if (Move == nullptr)
        {
                return;
        }
        UpdateBattleRage();
        if (MoveId == FName("Malkholm_Attack"))
        {
                CastDoubleStrike(Manager, Move);
        }
        else if (MoveId == FName("Malkholm_Revolution"))
        {
                CastRevolution(Move);
        }
        else if (MoveId == FName("Malkholm_Sacrifice"))
        {
                CastSacrifice(Manager, Move);
        }
        else if (MoveId == FName("Malkholm_Freedom"))
        {
                CastFreedom(Manager, Move, TargetIndex);
        }
        else if (MoveId == FName("Malkholm_Confrontation"))
        {
                CastConfrontation(Manager, Move);
        }
        else if (MoveId == FName("Malkholm_Power"))
        {
                CastPower(Manager, Move);
        }
        else if (MoveId == FName("Malkholm_Unity"))
        {
                CastUnity(Manager, Move);
        }
        else
        {
                Super::PerformBattleMove(Manager, MoveId, Move, TargetIndex);
        }
}
bool AMalkholmCharacter::CanUseBattleMove(FName MoveId) const
{
        if (MoveId == FName("Malkholm_Revolution"))
        {
                if (RevolutionLockRoundsLeft > 0)
                {
                        return false;
                }
        }
        if (MoveId == FName("Malkholm_Unity"))
        {
                ATurnBattleManager* Manager = FindBattleManager();
                if (Manager == nullptr)
                {
                        return false;
                }
                if (!Manager->HasOtherMemberToAct(this))
                {
                        return false;
                }
        }
        return Super::CanUseBattleMove(MoveId);
}
void AMalkholmCharacter::UpdateBattleRage()
{
        float MaxHealthValue = Health->MaxHealth;
        if (MaxHealthValue <= 0.0f)
        {
                return;
        }
        float MissingFraction = (MaxHealthValue - Health->CurrentHealth) / MaxHealthValue;
        if (MissingFraction < 0.0f)
        {
                MissingFraction = 0.0f;
        }
        FStatModifier Rage;
        Rage.SourceId = FName("MalkholmBattleRage");
        Rage.Stat = ECharacterStat::Damage;
        Rage.Kind = EModifierKind::Percent;
        Rage.Amount = MissingFraction * RageDamageAtNoHealth;
        Rage.Duration = 0.0f;
        Rage.TurnsLeft = 0;
        Rage.Rule = EStackRule::RefreshSameSource;
        Stats->AddModifier(Rage);
}
void AMalkholmCharacter::CastDoubleStrike(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        HitBattleBoss(Manager, Move->Power, false);
        HitBattleBoss(Manager, Move->Power, false);
        FStatModifier Hardened;
        Hardened.SourceId = FName("MalkholmDoubleStrike");
        Hardened.Stat = ECharacterStat::Defense;
        Hardened.Kind = EModifierKind::Percent;
        Hardened.Amount = Move->Amount1;
        Hardened.Duration = 0.0f;
        Hardened.TurnsLeft = 0;
        Hardened.Rule = EStackRule::StackToMax;
        Hardened.MaxStacks = DoubleStrikeMaxStacks;
        Stats->AddModifier(Hardened);
}
void AMalkholmCharacter::CastRevolution(const FBattleMoveRow* Move)
{
        bRevolutionThisRound = true;
        RevolutionFraction = Move->Amount1;
        RevolutionLockRoundsLeft = Move->Turns + 1;
        ShowBattleMessage(TEXT("Revolution: this round will echo"), FColor::Yellow);
}
void AMalkholmCharacter::CastSacrifice(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        float HealthCost = Health->CurrentHealth * Move->Amount1;
        Health->TakeHit(HealthCost);
        Manager->GrantExtraActionNextRound(this);
        ShowBattleMessage(FString::Printf(TEXT("Mal'Kholm gives up %d health for an extra action next round"), FMath::RoundToInt(HealthCost)), FColor::Yellow);
}
void AMalkholmCharacter::CastFreedom(ATurnBattleManager* Manager, const FBattleMoveRow* Move, int32 TargetIndex)
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
        FStatModifier Freedom;
        Freedom.SourceId = FName("MalkholmFreedom");
        Freedom.Stat = ECharacterStat::Defense;
        Freedom.Kind = EModifierKind::Percent;
        Freedom.Amount = Move->Amount1;
        Freedom.Duration = 0.0f;
        Freedom.TurnsLeft = Move->Turns;
        Freedom.Rule = EStackRule::StackToMax;
        Freedom.MaxStacks = FMath::RoundToInt(Move->Amount2);
        Ally->Stats->AddModifier(Freedom);
        int32 Stacks = Ally->Stats->GetStacks(Freedom.SourceId, ECharacterStat::Defense, EModifierKind::Percent);
        ShowBattleMessage(FString::Printf(TEXT("Freedom on %s (%d stacks)"), *Ally->DisplayName, Stacks), FColor::Yellow);
}
void AMalkholmCharacter::CastConfrontation(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
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
        float BossDefense = Boss->Stats->GetFinal(ECharacterStat::Defense);
        float StolenDefense = BossDefense * Move->Amount1;
        AddBattleModifier(Boss, FName("MalkholmConfrontation"), ECharacterStat::Defense, EModifierKind::Flat, 0.0f - StolenDefense, Move->Turns);
        AddBattleModifier(this, FName("MalkholmConfrontation"), ECharacterStat::Defense, EModifierKind::Flat, StolenDefense, Move->Turns);
        float MissingHealth = Health->MaxHealth - Health->CurrentHealth;
        float Damage = (MissingHealth * Move->Amount3) + (Boss->Health->MaxHealth * Move->Amount4);
        float DamageDealt = Boss->ApplyHit(Damage, true, this);
        float HealAmount = Boss->Health->CurrentHealth * Move->Amount2;
        Health->Heal(HealAmount);
        ShowBattleMessage(FString::Printf(TEXT("Confrontation: steals %d defense, deals %d, heals %d"), FMath::RoundToInt(StolenDefense), FMath::RoundToInt(DamageDealt), FMath::RoundToInt(HealAmount)), FColor::Yellow);
}
void AMalkholmCharacter::CastPower(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        float MyDefense = Stats->GetFinal(ECharacterStat::Defense);
        float MissingHealth = Health->MaxHealth - Health->CurrentHealth;
        float ShieldAmount = (MyDefense * Move->Amount1) + (MissingHealth * Move->Amount2);
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        Ally->Health->AddShield(ShieldAmount);
                }
        }
        ShowBattleMessage(FString::Printf(TEXT("Power: every ally gains a %d shield"), FMath::RoundToInt(ShieldAmount)), FColor::Yellow);
}
bool AMalkholmCharacter::TryBattleLastStand()
{
        if (bLastStandUsed)
        {
                return false;
        }
        bLastStandUsed = true;
        Health->CurrentHealth = 1.0f;
        LastStandRoundsLeft = LastStandRounds;
        Health->HealMultiplier = LastStandHealMultiplier;
        Health->ShieldMultiplier = LastStandHealMultiplier;
        AddBattleModifier(this, FName("MalkholmLastStand"), ECharacterStat::Defense, EModifierKind::Percent, LastStandDefenseBonus, LastStandRounds);
        ShowBattleMessage(TEXT("Mal'Kholm refuses to fall!"), FColor::Yellow);
        return true;
}
void AMalkholmCharacter::OnBattleRoundEnd(ATurnBattleManager* Manager)
{
        Super::OnBattleRoundEnd(Manager);
        if (RevolutionLockRoundsLeft > 0)
        {
                RevolutionLockRoundsLeft = RevolutionLockRoundsLeft - 1;
        }
        if (LastStandRoundsLeft > 0)
        {
                LastStandRoundsLeft = LastStandRoundsLeft - 1;
                if (LastStandRoundsLeft <= 0)
                {
                        Health->HealMultiplier = 1.0f;
                        Health->ShieldMultiplier = 1.0f;
                }
        }
}
void AMalkholmCharacter::OnBattlePlayerPhaseStart(ATurnBattleManager* Manager)
{
        Super::OnBattlePlayerPhaseStart(Manager);
        if (bRevolutionEchoReady)
        {
                bRevolutionEchoReady = false;
                ApplyRevolutionEcho(Manager);
        }
        TakeRoundSnapshot(Manager);
}
void AMalkholmCharacter::OnBattlePlayerPhaseEnd(ATurnBattleManager* Manager)
{
        Super::OnBattlePlayerPhaseEnd(Manager);
        if (!bRevolutionThisRound)
        {
                return;
        }
        bRevolutionThisRound = false;
        RecordRevolutionRound(Manager);
        bRevolutionEchoReady = true;
}
void AMalkholmCharacter::TakeRoundSnapshot(ATurnBattleManager* Manager)
{
        SnapshotBossHealth = 0.0f;
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss != nullptr)
        {
                SnapshotBossHealth = Boss->Health->CurrentHealth;
        }
        SnapshotHealth.Empty();
        SnapshotShield.Empty();
        SnapshotDamage.Empty();
        SnapshotDefense.Empty();
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        SnapshotHealth.Add(Ally->Health->CurrentHealth);
                        SnapshotShield.Add(Ally->Health->Shield);
                        SnapshotDamage.Add(Ally->Stats->GetFinal(ECharacterStat::Damage));
                        SnapshotDefense.Add(Ally->Stats->GetFinal(ECharacterStat::Defense));
                }
                else
                {
                        SnapshotHealth.Add(0.0f);
                        SnapshotShield.Add(0.0f);
                        SnapshotDamage.Add(0.0f);
                        SnapshotDefense.Add(0.0f);
                }
        }
}
float AMalkholmCharacter::GainOnly(float Before, float After) const
{
        float Gain = After - Before;
        if (Gain < 0.0f)
        {
                return 0.0f;
        }
        return Gain;
}
void AMalkholmCharacter::RecordRevolutionRound(ATurnBattleManager* Manager)
{
        EchoBossDamage = 0.0f;
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss != nullptr)
        {
                EchoBossDamage = GainOnly(Boss->Health->CurrentHealth, SnapshotBossHealth) * RevolutionFraction;
        }
        EchoHealing.Empty();
        EchoShield.Empty();
        EchoDamageBuff.Empty();
        EchoDefenseBuff.Empty();
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                float Healing = 0.0f;
                float ShieldGain = 0.0f;
                float DamageGain = 0.0f;
                float DefenseGain = 0.0f;
                if (Ally != nullptr)
                {
                        if (i < SnapshotHealth.Num())
                        {
                                Healing = GainOnly(SnapshotHealth[i], Ally->Health->CurrentHealth);
                                ShieldGain = GainOnly(SnapshotShield[i], Ally->Health->Shield);
                                DamageGain = GainOnly(SnapshotDamage[i], Ally->Stats->GetFinal(ECharacterStat::Damage));
                                DefenseGain = GainOnly(SnapshotDefense[i], Ally->Stats->GetFinal(ECharacterStat::Defense));
                        }
                }
                EchoHealing.Add(Healing * RevolutionFraction);
                EchoShield.Add(ShieldGain * RevolutionFraction);
                EchoDamageBuff.Add(DamageGain * RevolutionFraction);
                EchoDefenseBuff.Add(DefenseGain * RevolutionFraction);
        }
}
void AMalkholmCharacter::ApplyRevolutionEcho(ATurnBattleManager* Manager)
{
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss != nullptr)
        {
                if (EchoBossDamage > 0.0f)
                {
                        Boss->ApplyHit(EchoBossDamage, true, this);
                }
        }
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        if (i < EchoHealing.Num())
                        {
                                ApplyEchoToAlly(Ally, EchoHealing[i], EchoShield[i], EchoDamageBuff[i], EchoDefenseBuff[i]);
                        }
                }
        }
        ShowBattleMessage(FString::Printf(TEXT("Revolution echoes last round (%d damage)"), FMath::RoundToInt(EchoBossDamage)), FColor::Yellow);
}
void AMalkholmCharacter::ApplyEchoToAlly(APartyMemberCharacter* Ally, float Healing, float ShieldAmount, float DamageBuff, float DefenseBuff)
{
        if (Ally->Health->IsDead())
        {
                return;
        }
        if (Healing > 0.0f)
        {
                Ally->Health->Heal(Healing);
        }
        if (ShieldAmount > 0.0f)
        {
                Ally->Health->AddShield(ShieldAmount);
        }
        if (DamageBuff > 0.0f)
        {
                AddBattleModifier(Ally, FName("MalkholmRevolutionEcho"), ECharacterStat::Damage, EModifierKind::Flat, DamageBuff, RevolutionEchoBuffRounds);
        }
        if (DefenseBuff > 0.0f)
        {
                AddBattleModifier(Ally, FName("MalkholmRevolutionEcho"), ECharacterStat::Defense, EModifierKind::Flat, DefenseBuff, RevolutionEchoBuffRounds);
        }
}
void AMalkholmCharacter::CastUnity(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        Manager->RestartRetributionOn(this, Move->Amount1, Move->Amount2, Move->Turns);
}
void AMalkholmCharacter::PerformBattleRetribution(ATurnBattleManager* Manager)
{
        Super::PerformBattleRetribution(Manager);
        Manager->GrantFreeAction();
}
