#include "KaliaCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Battle/BattleMoveRow.h"
#include "Characters/BossCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Engine/DataTable.h"
#include "Stats/StatTypes.h"
void AKaliaCharacter::PerformBattleMove(ATurnBattleManager* Manager, FName MoveId, const FBattleMoveRow* Move, int32 TargetIndex)
{
        if (Manager == nullptr)
        {
                return;
        }
        if (Move == nullptr)
        {
                return;
        }
        bool bWasConcentrating = bConcentrationReady;
        if (MoveId == FName("Kalia_Chaos"))
        {
                CastChaos(Manager, Move);
        }
        else if (MoveId == FName("Kalia_Sever"))
        {
                CastSever(Move);
        }
        else if (MoveId == FName("Kalia_Battlesong"))
        {
                CastBattlesong(Manager, Move, TargetIndex);
        }
        else if (MoveId == FName("Kalia_Riposte"))
        {
                CastRiposte(Move);
        }
        else if (MoveId == FName("Kalia_Barrage"))
        {
                CastBarrage(Manager, Move);
        }
        else if (MoveId == FName("Kalia_Concentration"))
        {
                CastConcentration(Move);
        }
        else if (MoveId == FName("Kalia_Slayer"))
        {
                CastSlayer(Move);
        }
        else
        {
                Super::PerformBattleMove(Manager, MoveId, Move, TargetIndex);
        }
        if (bWasConcentrating)
        {
                if (MoveTargetsEnemy(Move))
                {
                        ApplyConcentration(Manager);
                }
        }
}
bool AKaliaCharacter::CanUseBattleMove(FName MoveId) const
{
        if (MoveId == FName("Kalia_Slayer"))
        {
                if (bSlayerApplied)
                {
                        return false;
                }
        }
        if (MoveId == FName("Kalia_Sever"))
        {
                if (bSeverPending)
                {
                        return false;
                }
        }
        return Super::CanUseBattleMove(MoveId);
}
FString AKaliaCharacter::GetBattleCardNote() const
{
        FString Note = Super::GetBattleCardNote();
        Note = Note + FString::Printf(TEXT("Proxy %d/%d"), GetProxyStacks(), ProxyMaxStacks);
        if (!ProxySuggestion.IsNone())
        {
                Note = Note + FString::Printf(TEXT(": %s"), *ProxySuggestionLabel);
        }
        if (bRiposteReady)
        {
                Note = Note + TEXT(" Parry");
        }
        if (bSeverPending)
        {
                Note = Note + TEXT(" Sickle");
        }
        return Note;
}
int32 AKaliaCharacter::PickRandomLivingAlly(ATurnBattleManager* Manager) const
{
        TArray<int32> LivingIndices;
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        if (!Ally->Health->IsDead())
                        {
                                LivingIndices.Add(i);
                        }
                }
        }
        if (LivingIndices.Num() == 0)
        {
                return -1;
        }
        int32 Pick = FMath::RandRange(0, LivingIndices.Num() - 1);
        return LivingIndices[Pick];
}
void AKaliaCharacter::CastChaos(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        int32 AllyIndex = PickRandomLivingAlly(Manager);
        APartyMemberCharacter* Ally = Manager->GetPartyMember(AllyIndex);
        if (Ally == nullptr)
        {
                return;
        }
        ECharacterStat ChosenStat = ECharacterStat::Damage;
        FString StatName = TEXT("attack");
        if (FMath::RandBool())
        {
                ChosenStat = ECharacterStat::Defense;
                StatName = TEXT("defense");
        }
        float Amount = FMath::FRandRange(Move->Amount1, Move->Amount2);
        int32 Rounds = FMath::RandRange(FMath::RoundToInt(Move->Amount3), FMath::RoundToInt(Move->Amount4));
        AddBattleModifier(Ally, FName("KaliaChaos"), ChosenStat, EModifierKind::Percent, Amount, Rounds);
        ShowBattleMessage(FString::Printf(TEXT("Chaos: %s gets +%.1f%% %s for %d rounds"), *Ally->DisplayName, Amount * 100.0f, *StatName, Rounds), FColor::Magenta);
}
void AKaliaCharacter::CastSever(const FBattleMoveRow* Move)
{
        SeverCasts = SeverCasts + 1;
        SeverFailChance = Move->Amount4 * SeverCasts;
        if (SeverFailChance > SeverMaxFailChance)
        {
                SeverFailChance = SeverMaxFailChance;
        }
        SeverMaxHealthFraction = Move->Amount1 * SeverCasts;
        SeverMissingHealthFraction = Move->Amount2 * SeverCasts;
        SeverDebuff = Move->Amount3;
        SeverRounds = Move->Turns;
        bSeverPending = true;
        ShowBattleMessage(FString::Printf(TEXT("Kalia throws her sickle (%d%% to miss)"), FMath::RoundToInt(SeverFailChance * 100.0f)), FColor::Magenta);
}
void AKaliaCharacter::LandSever(ATurnBattleManager* Manager)
{
        if (!bSeverPending)
        {
                return;
        }
        bSeverPending = false;
        if (Health->IsDead())
        {
                ShowBattleMessage(TEXT("The sickle falls with no one to catch it"), FColor::Orange);
                return;
        }
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
        if (Roll < SeverFailChance)
        {
                ShowBattleMessage(TEXT("The sickle misses"), FColor::Orange);
                return;
        }
        FName SourceId = FName("KaliaSever");
        AddBattleModifier(Boss, SourceId, ECharacterStat::Damage, EModifierKind::Percent, 0.0f - SeverDebuff, SeverRounds);
        AddBattleModifier(Boss, SourceId, ECharacterStat::Defense, EModifierKind::Percent, 0.0f - SeverDebuff, SeverRounds);
        float MissingHealth = Boss->Health->MaxHealth - Boss->Health->CurrentHealth;
        float Damage = (Boss->Health->MaxHealth * SeverMaxHealthFraction) + (MissingHealth * SeverMissingHealthFraction);
        float DamageDealt = Boss->ApplyHit(Damage, true, this);
        ShowBattleMessage(FString::Printf(TEXT("Kalia dashes to her sickle and severs for %d"), FMath::RoundToInt(DamageDealt)), FColor::Magenta);
}
void AKaliaCharacter::CastBattlesong(ATurnBattleManager* Manager, const FBattleMoveRow* Move, int32 TargetIndex)
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
        Ally->SetNextAttackBonus(Move->Amount1);
        ShowBattleMessage(FString::Printf(TEXT("Battlesong: %s's next attack hits harder"), *Ally->DisplayName), FColor::Magenta);
}
void AKaliaCharacter::CastRiposte(const FBattleMoveRow* Move)
{
        bRiposteReady = true;
        RiposteReduction = Move->Amount1;
        RiposteReturn = Move->Amount2;
        Stats->bRepelDebuffs = true;
        ShowBattleMessage(TEXT("Kalia readies a parry"), FColor::Magenta);
}
float AKaliaCharacter::FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker)
{
        float Result = Super::FilterIncomingDamage(RawDamage, bTrueDamage, Attacker);
        if (!bRiposteReady)
        {
                return Result;
        }
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
        bRiposteReady = false;
        float DamageItWouldDo = CalculateFinalDamage(Result, bTrueDamage);
        float ReturnedDamage = DamageItWouldDo * RiposteReturn;
        float DamageReturned = Attacker->ApplyHit(ReturnedDamage, true, this);
        ShowBattleMessage(FString::Printf(TEXT("Riposte! Kalia sends back %d"), FMath::RoundToInt(DamageReturned)), FColor::Magenta);
        return Result * (1.0f - RiposteReduction);
}
void AKaliaCharacter::CastBarrage(ATurnBattleManager* Manager, const FBattleMoveRow* Move)
{
        int32 HitCount = FMath::RoundToInt(Move->Amount1);
        float TotalDamage = 0.0f;
        for (int32 i = 0; i < HitCount; i++)
        {
                TotalDamage = TotalDamage + HitBattleBoss(Manager, Move->Power, false);
        }
        ShowBattleMessage(FString::Printf(TEXT("Barrage: %d strikes for %d"), HitCount, FMath::RoundToInt(TotalDamage)), FColor::Magenta);
}
void AKaliaCharacter::CastConcentration(const FBattleMoveRow* Move)
{
        bConcentrationReady = true;
        ConcentrationAmount = Move->Amount1;
        ConcentrationRounds = Move->Turns;
        ShowBattleMessage(TEXT("Kalia concentrates on her next strike"), FColor::Magenta);
}
void AKaliaCharacter::ApplyConcentration(ATurnBattleManager* Manager)
{
        bConcentrationReady = false;
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss == nullptr)
        {
                return;
        }
        if (Boss->Health->IsDead())
        {
                return;
        }
        AddBattleModifier(Boss, FName("KaliaConcentration"), ECharacterStat::Defense, EModifierKind::Percent, 0.0f - ConcentrationAmount, ConcentrationRounds);
        ShowBattleMessage(TEXT("Concentration: the boss's defense drops"), FColor::Magenta);
}
void AKaliaCharacter::CastSlayer(const FBattleMoveRow* Move)
{
        bSlayerApplied = true;
        SlayerFraction = Move->Amount1;
        ShowBattleMessage(TEXT("Slayer: the boss carries a wound that will not close"), FColor::Magenta);
}
void AKaliaCharacter::OnBattleBossTurn(ATurnBattleManager* Manager)
{
        Super::OnBattleBossTurn(Manager);
        if (!bSlayerApplied)
        {
                return;
        }
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss == nullptr)
        {
                return;
        }
        if (Boss->Health->IsDead())
        {
                return;
        }
        float MissingHealth = Boss->Health->MaxHealth - Boss->Health->CurrentHealth;
        float Damage = MissingHealth * SlayerFraction;
        if (Damage <= 0.0f)
        {
                return;
        }
        float DamageDealt = Boss->ApplyHit(Damage, true, this);
        ShowBattleMessage(FString::Printf(TEXT("The Slayer wound bleeds for %d"), FMath::RoundToInt(DamageDealt)), FColor::Magenta);
}
void AKaliaCharacter::OnBattlePlayerPhaseStart(ATurnBattleManager* Manager)
{
        Super::OnBattlePlayerPhaseStart(Manager);
        LandSever(Manager);
        PickProxySuggestion(Manager);
}
void AKaliaCharacter::OnBattlePlayerPhaseEnd(ATurnBattleManager* Manager)
{
        Super::OnBattlePlayerPhaseEnd(Manager);
        ProxySuggestion = NAME_None;
        ProxySuggestionLabel = FString();
}
void AKaliaCharacter::OnBattleRoundEnd(ATurnBattleManager* Manager)
{
        Super::OnBattleRoundEnd(Manager);
        bRiposteReady = false;
        Stats->bRepelDebuffs = false;
        ApplyProxyDecay(Manager);
}
void AKaliaCharacter::PickProxySuggestion(ATurnBattleManager* Manager)
{
        ProxySuggestion = NAME_None;
        ProxySuggestionLabel = FString();
        if (bProxyTakeoverUsed)
        {
                return;
        }
        if (Health->IsDead())
        {
                return;
        }
        if (Manager->GetRound() % 2 != 0)
        {
                return;
        }
        FName Choice = FName("Block");
        float Roll = FMath::FRand();
        if (Roll >= ProxyBlockWeight)
        {
                if (Roll < ProxyBlockWeight + ProxyItemWeight)
                {
                        int32 MyIndex = Manager->FindPartyIndex(this);
                        if (Manager->IsEntryUsable(MyIndex, FName("Item")))
                        {
                                Choice = FName("Item");
                        }
                }
                else
                {
                        FName MoveChoice = PickRandomOwnMove(Manager);
                        if (!MoveChoice.IsNone())
                        {
                                Choice = MoveChoice;
                        }
                }
        }
        ProxySuggestion = Choice;
        ProxySuggestionLabel = Choice.ToString();
        const FBattleMoveRow* SuggestedMove = Manager->GetMove(Choice);
        if (SuggestedMove != nullptr)
        {
                ProxySuggestionLabel = SuggestedMove->DisplayName.ToString();
        }
        ShowBattleMessage(FString::Printf(TEXT("Proxy whispers to Kalia: %s"), *ProxySuggestionLabel), FColor::Magenta);
}
FName AKaliaCharacter::PickRandomOwnMove(ATurnBattleManager* Manager) const
{
        if (Manager->MoveTable == nullptr)
        {
                return NAME_None;
        }
        TArray<FName> AllNames = Manager->MoveTable->GetRowNames();
        TArray<FName> MyMoves;
        for (int32 i = 0; i < AllNames.Num(); i++)
        {
                const FBattleMoveRow* Move = Manager->GetMove(AllNames[i]);
                if (Move != nullptr)
                {
                        if (Move->MemberId == StatsRowName)
                        {
                                MyMoves.Add(AllNames[i]);
                        }
                }
        }
        if (MyMoves.Num() == 0)
        {
                return NAME_None;
        }
        int32 Pick = FMath::RandRange(0, MyMoves.Num() - 1);
        return MyMoves[Pick];
}
void AKaliaCharacter::OnBattleActionChosen(ATurnBattleManager* Manager, FName EntryName)
{
        Super::OnBattleActionChosen(Manager, EntryName);
        if (ProxySuggestion.IsNone())
        {
                return;
        }
        FName Suggested = ProxySuggestion;
        ProxySuggestion = NAME_None;
        ProxySuggestionLabel = FString();
        if (EntryName == Suggested)
        {
                GainProxyStack(Manager);
        }
        else
        {
                ShowBattleMessage(TEXT("Kalia ignores Proxy"), FColor::Magenta);
        }
}
int32 AKaliaCharacter::GetProxyStacks() const
{
        return Stats->GetStacks(FName("KaliaProxy"), ECharacterStat::Damage, EModifierKind::Percent);
}
void AKaliaCharacter::GainProxyStack(ATurnBattleManager* Manager)
{
        FStatModifier ProxyStack;
        ProxyStack.SourceId = FName("KaliaProxy");
        ProxyStack.Stat = ECharacterStat::Damage;
        ProxyStack.Kind = EModifierKind::Percent;
        ProxyStack.Amount = ProxyStackDamage;
        ProxyStack.Duration = 0.0f;
        ProxyStack.TurnsLeft = 0;
        ProxyStack.Rule = EStackRule::StackToMax;
        ProxyStack.MaxStacks = ProxyMaxStacks;
        Stats->AddModifier(ProxyStack);
        int32 Stacks = GetProxyStacks();
        ShowBattleMessage(FString::Printf(TEXT("Kalia heeds Proxy (%d/%d)"), Stacks, ProxyMaxStacks), FColor::Magenta);
        if (Stacks < ProxyMaxStacks)
        {
                return;
        }
        if (bProxyTakeoverUsed)
        {
                return;
        }
        bProxyTakeoverUsed = true;
        Manager->NegateBossTurns(ProxyFreezeTurns);
        ShowBattleMessage(TEXT("Proxy takes over Kalia: the boss is frozen in time"), FColor::Magenta);
}
void AKaliaCharacter::ApplyProxyDecay(ATurnBattleManager* Manager)
{
        if (!bProxyTakeoverUsed)
        {
                return;
        }
        if (Manager->GetNegatedBossTurnsLeft() > 0)
        {
                return;
        }
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss == nullptr)
        {
                return;
        }
        if (Boss->Health->IsDead())
        {
                return;
        }
        FStatModifier Decay;
        Decay.SourceId = FName("KaliaProxyDecay");
        Decay.Kind = EModifierKind::Percent;
        Decay.Amount = 0.0f - ProxyDecayPerRound;
        Decay.Duration = 0.0f;
        Decay.TurnsLeft = 0;
        Decay.Rule = EStackRule::StackToMax;
        Decay.MaxStacks = ProxyDecayMaxStacks;
        Decay.Stat = ECharacterStat::Damage;
        Boss->Stats->AddModifier(Decay);
        Decay.Stat = ECharacterStat::Defense;
        Boss->Stats->AddModifier(Decay);
        int32 Stacks = Boss->Stats->GetStacks(FName("KaliaProxyDecay"), ECharacterStat::Damage, EModifierKind::Percent);
        ShowBattleMessage(FString::Printf(TEXT("Proxy's curse weakens the boss (-%.1f%%)"), Stacks * ProxyDecayPerRound * 100.0f), FColor::Magenta);
}
void AKaliaCharacter::PerformBattleRetribution(ATurnBattleManager* Manager)
{
        Super::PerformBattleRetribution(Manager);
        int32 CleansedCount = 0;
        for (int32 i = 0; i < Manager->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Ally = Manager->GetPartyMember(i);
                if (Ally != nullptr)
                {
                        if (Ally != this)
                        {
                                if (!Ally->Health->IsDead())
                                {
                                        Ally->Stats->RemoveHarmfulModifiers();
                                        CleansedCount = CleansedCount + 1;
                                }
                        }
                }
        }
        ShowBattleMessage(FString::Printf(TEXT("Kalia's retribution cleanses %d allies"), CleansedCount), FColor::Magenta);
}
