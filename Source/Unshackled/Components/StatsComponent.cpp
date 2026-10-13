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

// Lowers every timed modifier's time left, and drops the ones that ran out.
void UStatsComponent::CountDownModifiers(float DeltaTime)
{
        TArray<FStatModifier> StillActive;
        for (int32 i = 0; i < Modifiers.Num(); i++)
        {
                FStatModifier Current = Modifiers[i];
                // Duration 0 means permanent, so it is always kept.
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

// Returns the position of the same effect in our list, or -1 if we do not have it yet.
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

// Adds a modifier, applying the stacking rule if the same effect is already on us.
void UStatsComponent::AddModifier(FStatModifier NewModifier)
{
        int32 ExistingIndex = FindModifierIndex(NewModifier.SourceId, NewModifier.Stat, NewModifier.Kind);

        // Not on us yet, so add it as a brand new modifier.
        if (ExistingIndex == -1)
        {
                NewModifier.Stacks = 1;
                NewModifier.TimeLeft = NewModifier.Duration;
                Modifiers.Add(NewModifier);
                return;
        }

        // Already on us and it stacks: add one stack (up to the max) and restart the timer.
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

        // Any other rule: replace the old one with the new one (one stack, fresh timer).
        NewModifier.Stacks = 1;
        NewModifier.TimeLeft = NewModifier.Duration;
        Modifiers[ExistingIndex] = NewModifier;
}

// Ends an effect early (for example, recasting an ultimate to cancel it).
void UStatsComponent::RemoveModifiersFromSource(FName SourceId)
{
        TArray<FStatModifier> StillActive;

        for (int32 i = 0; i < Modifiers.Num(); i++)
        {
                // Keep everything that is NOT from this source.
                if (Modifiers[i].SourceId != SourceId)
                {
                        StillActive.Add(Modifiers[i]);
                }
        }

        Modifiers = StillActive;
}

// Call once each time this character is hit. Uses up one hit on ConsumeOnHit modifiers.
void UStatsComponent::ConsumeHit()
{
        TArray<FStatModifier> StillActive;

        for (int32 i = 0; i < Modifiers.Num(); i++)
        {
                FStatModifier Current = Modifiers[i];
                bool bKeepIt = true;

                // Only ConsumeOnHit modifiers get used up by hits.
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

// Final = (Base + all Flat) * (1 + all Percent), and slows are handled separately.
float UStatsComponent::GetFinal(ECharacterStat Stat) const
{
        float BaseValue = GetBaseValue(Stat);
        float FlatTotal = GetFlatTotal(Stat);
        float PercentTotal = GetPercentTotal(Stat);

        // Slows do not add together. Only the strongest one counts, and only for Speed.
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

// Adds up every Flat modifier for this stat. Amount is per stack, so we multiply by Stacks.
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

// Finds the single biggest slow on Speed, as a positive fraction (0.5 = 50% slower).
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

        // No slow can be stronger than the cap.
        if (StrongestSlow > MaxSlowFraction)
        {
                StrongestSlow = MaxSlowFraction;
        }

        return StrongestSlow;
}

// Keeps a stat from going below its lowest allowed value.
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

// Rooted, stunned, or frozen characters cannot walk.
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

// Rooted characters can still attack. Stunned or frozen ones cannot.
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

// Returns false (and spends nothing) if there is not enough mana.
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