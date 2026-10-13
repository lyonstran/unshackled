#include "StatsComponent.h"
UStatsComponent::UStatsComponent()
{
        PrimaryComponentTick.bCanEverTick = true;
}
void UStatsComponent::BeginPlay()
{
        Super::BeginPlay();
        if (bHasMana)
        {
                CurrentMana = GetFinal(ECharacterStat::MaxMana);
        }
}
void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction)
{
        Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
        CountDownModifiers(DeltaTime);
}
void UStatsComponent::CountDownModifiers(float DeltaTime)
{
        TArray<FStatModifier> StillActive;
        for (int32 i = 0; i < Modifiers.Num(); i++)
        {
                FStatModifier Current = Modifiers[i];
                if (Current.Duration <= 0.0f)
                {
                        StillActive.Add(Current);
                }
                else
                {
                        Current.TimeLeft = Current.TimeLeft - DeltaTime;
                        if (Current.TimeLeft > 0.0f)
                        {
                                StillActive.Add(Current);
                        }
                }
        }
        Modifiers = StillActive;
}
int32 UStatsComponent::FindModifierIndex(FName SourceId, ECharacterStat Stat, EModifierKind Kind) const
{
        for (int32 i = 0; i < Modifiers.Num(); i++)
        {
                if (Modifiers[i].SourceId == SourceId)
                {
                        if (Modifiers[i].Stat == Stat)
                        {
                                if (Modifiers[i].Kind == Kind)
                                {
                                        return i;
                                }
                        }
                }
        }
        return -1;
}
void UStatsComponent::AddModifier(FStatModifier NewModifier)
{
        int32 ExistingIndex = FindModifierIndex(NewModifier.SourceId, NewModifier.Stat, NewModifier.Kind);
        if (ExistingIndex == -1)
        {
                NewModifier.Stacks = 1;
                NewModifier.TimeLeft = NewModifier.Duration;
                Modifiers.Add(NewModifier);
                return;
        }
        if (NewModifier.Rule == EStackRule::StackToMax)
        {
                int32 CurrentStacks = Modifiers[ExistingIndex].Stacks;
                if (CurrentStacks < NewModifier.MaxStacks)
                {
                        Modifiers[ExistingIndex].Stacks = CurrentStacks + 1;
                }
                Modifiers[ExistingIndex].TimeLeft = NewModifier.Duration;
                return;
        }
        NewModifier.Stacks = 1;
        NewModifier.TimeLeft = NewModifier.Duration;
        Modifiers[ExistingIndex] = NewModifier;
}
void UStatsComponent::RemoveModifiersFromSource(FName SourceId)
{
        TArray<FStatModifier> StillActive;
        for (int32 i = 0; i < Modifiers.Num(); i++)
        {
                if (Modifiers[i].SourceId != SourceId)
                {
                        StillActive.Add(Modifiers[i]);
                }
        }
        Modifiers = StillActive;
}
void UStatsComponent::ConsumeHit()
{
        TArray<FStatModifier> StillActive;
        for (int32 i = 0; i < Modifiers.Num(); i++)
        {
                FStatModifier Current = Modifiers[i];
                bool bKeepIt = true;
                if (Current.Rule == EStackRule::ConsumeOnHit)
                {
                        Current.HitsRemaining = Current.HitsRemaining - 1;
                        if (Current.HitsRemaining <= 0)
                        {
                                bKeepIt = false;
                        }
                }
                if (bKeepIt)
                {
                        StillActive.Add(Current);
                }
        }
        Modifiers = StillActive;
}
float UStatsComponent::GetFinal(ECharacterStat Stat) const
{
        float BaseValue = GetBaseValue(Stat);
        float FlatTotal = GetFlatTotal(Stat);
        float PercentTotal = GetPercentTotal(Stat);
        if (Stat == ECharacterStat::Speed)
        {
                float StrongestSlow = GetStrongestSlow();
                PercentTotal = PercentTotal - StrongestSlow;
        }
        float Result = (BaseValue + FlatTotal) * (1.0f + PercentTotal);
        return ApplyFloors(Stat, Result);
}
float UStatsComponent::GetBaseValue(ECharacterStat Stat) const
{
        if (Stat == ECharacterStat::MaxHealth)
        {
                return BaseMaxHealth;
        }
        if (Stat == ECharacterStat::MaxMana)
        {
                return BaseMaxMana;
        }
        if (Stat == ECharacterStat::Damage)
        {
                return BaseDamage;
        }
        if (Stat == ECharacterStat::Defense)
        {
                return BaseDefense;
        }
        if (Stat == ECharacterStat::Speed)
        {
                return BaseSpeed;
        }
        if (Stat == ECharacterStat::IncomingDamage)
        {
                return 1.0f;
        }
        return 0.0f;
}
float UStatsComponent::GetFlatTotal(ECharacterStat Stat) const
{
        float FlatTotal = 0.0f;
        for (int32 i = 0; i < Modifiers.Num(); i++)
        {
                if (Modifiers[i].Stat == Stat)
                {
                        if (Modifiers[i].Kind == EModifierKind::Flat)
                        {
                                float AmountWithStacks = Modifiers[i].Amount * Modifiers[i].Stacks;
                                FlatTotal = FlatTotal + AmountWithStacks;
                        }
                }
        }
        return FlatTotal;
}
float UStatsComponent::GetPercentTotal(ECharacterStat Stat) const
{
        float PercentTotal = 0.0f;
        for (int32 i = 0; i < Modifiers.Num(); i++)
        {
                if (Modifiers[i].Stat == Stat)
                {
                        if (Modifiers[i].Kind == EModifierKind::Percent)
                        {
                                float AmountWithStacks = Modifiers[i].Amount * Modifiers[i].Stacks;
                                bool bIsSlow = false;
                                if (Stat == ECharacterStat::Speed)
                                {
                                        if (AmountWithStacks < 0.0f)
                                        {
                                                bIsSlow = true;
                                        }
                                }
                                if (!bIsSlow)
                                {
                                        PercentTotal = PercentTotal + AmountWithStacks;
                                }
                        }
                }
        }
        return PercentTotal;
}
float UStatsComponent::GetStrongestSlow() const
{
        float StrongestSlow = 0.0f;
        for (int32 i = 0; i < Modifiers.Num(); i++)
        {
                if (Modifiers[i].Stat == ECharacterStat::Speed)
                {
                        if (Modifiers[i].Kind == EModifierKind::Percent)
                        {
                                float AmountWithStacks = Modifiers[i].Amount * Modifiers[i].Stacks;
                                if (AmountWithStacks < 0.0f)
                                {
                                        float SlowSize = -AmountWithStacks;
                                        if (SlowSize > StrongestSlow)
                                        {
                                                StrongestSlow = SlowSize;
                                        }
                                }
                        }
                }
        }
        if (StrongestSlow > MaxSlowFraction)
        {
                StrongestSlow = MaxSlowFraction;
        }
        return StrongestSlow;
}
float UStatsComponent::ApplyFloors(ECharacterStat Stat, float Value) const
{
        float Result = Value;
        if (Result < 0.0f)
        {
                Result = 0.0f;
        }
        if (Stat == ECharacterStat::MaxHealth)
        {
                if (Result < 1.0f)
                {
                        Result = 1.0f;
                }
        }
        return Result;
}
bool UStatsComponent::HasFlag(EStatusFlag Flag) const
{
        for (int32 i = 0; i < Modifiers.Num(); i++)
        {
                if (Modifiers[i].Flag == Flag)
                {
                        return true;
                }
        }
        return false;
}
bool UStatsComponent::CanMove() const
{
        if (HasFlag(EStatusFlag::Root))
        {
                return false;
        }
        if (HasFlag(EStatusFlag::Stun))
        {
                return false;
        }
        if (HasFlag(EStatusFlag::Freeze))
        {
                return false;
        }
        return true;
}
bool UStatsComponent::CanAct() const
{
        if (HasFlag(EStatusFlag::Stun))
        {
                return false;
        }
        if (HasFlag(EStatusFlag::Freeze))
        {
                return false;
        }
        return true;
}
bool UStatsComponent::SpendMana(float Amount)
{
        if (!bHasMana)
        {
                return false;
        }
        if (CurrentMana < Amount)
        {
                return false;
        }
        CurrentMana = CurrentMana - Amount;
        return true;
}
void UStatsComponent::RestoreMana(float Amount)
{
        if (!bHasMana)
        {
                return;
        }
        CurrentMana = CurrentMana + Amount;
        float MaxMana = GetFinal(ECharacterStat::MaxMana);
        if (CurrentMana > MaxMana)
        {
                CurrentMana = MaxMana;
        }
}