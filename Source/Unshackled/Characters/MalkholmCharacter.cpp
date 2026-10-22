#include "MalkholmCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
AMalkholmCharacter::AMalkholmCharacter()
{
        DisplayName = TEXT("Mal'Kholm");
        AuraColor = FLinearColor(1.0f, 0.9f, 0.0f);
        AttackDamage = 22.0f;
        AttackCooldown = 0.7f;
        SpecialWindup = 1.5f;
        Health->MaxHealth = 160.0f;
        Stats->BaseDefense = 40.0f;
}
void AMalkholmCharacter::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        TickRetribution(DeltaTime);
        TickPassiveCooldown(DeltaTime);
}
float AMalkholmCharacter::GetRangeMultiplier() const
{
        if (RetributionTimeLeft > 0.0f)
        {
                return RetributionRangeMultiplier;
        }
        return 1.0f;
}
void AMalkholmCharacter::PerformBasicAttack(FVector Direction)
{
        SwingCount = SwingCount + 1;
        FVector SwingCenter = GetActorLocation();
        float EffectiveRadius = ChainRadius * GetRangeMultiplier();
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        FCollisionShape Circle = FCollisionShape::MakeSphere(EffectiveRadius);
        GetWorld()->OverlapMultiByObjectType(OverlapResults, SwingCenter, FQuat::Identity, ObjectParams, Circle, QueryParams);
        TArray<AActor*> ActorsAlreadyHit;
        for (int32 i = 0; i < OverlapResults.Num(); i++)
        {
                AActor* HitActor = OverlapResults[i].GetActor();
                if (HitActor != nullptr)
                {
                        if (!ActorsAlreadyHit.Contains(HitActor))
                        {
                                ActorsAlreadyHit.Add(HitActor);
                                AUnshackledCharacterBase* OtherCharacter = Cast<AUnshackledCharacterBase>(HitActor);
                                if (OtherCharacter != nullptr)
                                {
                                        if (OtherCharacter->bIsEnemy != bIsEnemy)
                                        {
                                                DealDamage(OtherCharacter, SwingDamageScale, false);
                                                ApplyEdgeDebuffs(OtherCharacter);
                                        }
                                }
                        }
                }
        }
        if (IsBonusSwing())
        {
                GainDefenseStack();
        }
        DrawDebugSphere(GetWorld(), SwingCenter, EffectiveRadius, 16, FColor::Orange, false, 0.2f);
        ShowSwingText();
        bSwingClockwise = !bSwingClockwise;
}
bool AMalkholmCharacter::IsBonusSwing() const
{
        int32 Remainder = SwingCount % 2;
        if (Remainder == 0)
        {
                return true;
        }
        return false;
}
bool AMalkholmCharacter::IsOnEdge(AUnshackledCharacterBase* Target) const
{
        float DistanceToTarget = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());
        float EdgeDistance = ChainRadius * GetRangeMultiplier() * EdgeStartFraction;
        if (DistanceToTarget >= EdgeDistance)
        {
                return true;
        }
        return false;
}
void AMalkholmCharacter::ApplyEdgeDebuffs(AUnshackledCharacterBase* Target)
{
        if (!IsBonusSwing())
        {
                return;
        }
        if (Target->Health->IsDead())
        {
                return;
        }
        if (!IsOnEdge(Target))
        {
                return;
        }
        FStatModifier DamageLoss;
        DamageLoss.SourceId = FName("MalkholmChainDamageLoss");
        DamageLoss.Stat = ECharacterStat::Damage;
        DamageLoss.Kind = EModifierKind::Percent;
        DamageLoss.Amount = 0.0f - DamageLossPerStack;
        DamageLoss.Duration = DamageLossDuration;
        DamageLoss.Rule = EStackRule::StackToMax;
        DamageLoss.MaxStacks = MaxDamageLossStacks;
        Target->Stats->AddModifier(DamageLoss);
        FStatModifier EdgeSlow;
        EdgeSlow.SourceId = FName("MalkholmChainSlow");
        EdgeSlow.Stat = ECharacterStat::Speed;
        EdgeSlow.Kind = EModifierKind::Percent;
        EdgeSlow.Amount = 0.0f - EdgeSlowFraction;
        EdgeSlow.Duration = EdgeSlowDuration;
        EdgeSlow.Rule = EStackRule::RefreshSameSource;
        Target->Stats->AddModifier(EdgeSlow);
}
void AMalkholmCharacter::GainDefenseStack()
{
        FStatModifier DefenseGain;
        DefenseGain.SourceId = FName("MalkholmChainDefense");
        DefenseGain.Stat = ECharacterStat::Defense;
        DefenseGain.Kind = EModifierKind::Percent;
        DefenseGain.Amount = DefenseGainPerStack;
        DefenseGain.Duration = DefenseGainDuration;
        DefenseGain.Rule = EStackRule::StackToMax;
        DefenseGain.MaxStacks = MaxDefenseStacks;
        Stats->AddModifier(DefenseGain);
}
void AMalkholmCharacter::ShowSwingText()
{
        FString DirectionName = TEXT("Counterclockwise");
        if (bSwingClockwise)
        {
                DirectionName = TEXT("Clockwise");
        }
        FString Message = FString::Printf(TEXT("Chain swing %d: %s"), SwingCount, *DirectionName);
        GEngine->AddOnScreenDebugMessage(-1, 0.6f, FColor::Orange, Message);
}
void AMalkholmCharacter::PerformSpecial(FVector AimDirection)
{
        float RangeMultiplier = GetRangeMultiplier();
        float EffectiveRange = SlamRange * RangeMultiplier;
        float EffectiveRadius = SlamRadius * RangeMultiplier;
        FVector SlamCenter = GetActorLocation() + (AimDirection * EffectiveRange);
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        FCollisionShape SlamShape = FCollisionShape::MakeSphere(EffectiveRadius);
        GetWorld()->OverlapMultiByObjectType(OverlapResults, SlamCenter, FQuat::Identity, ObjectParams, SlamShape, QueryParams);
        TArray<AActor*> ActorsAlreadyHit;
        for (int32 i = 0; i < OverlapResults.Num(); i++)
        {
                AActor* HitActor = OverlapResults[i].GetActor();
                if (HitActor != nullptr)
                {
                        if (!ActorsAlreadyHit.Contains(HitActor))
                        {
                                ActorsAlreadyHit.Add(HitActor);
                                AUnshackledCharacterBase* OtherCharacter = Cast<AUnshackledCharacterBase>(HitActor);
                                if (OtherCharacter != nullptr)
                                {
                                        if (OtherCharacter->bIsEnemy != bIsEnemy)
                                        {
                                                DealDamage(OtherCharacter, SlamDamageScale, false);
                                                ApplySlamEffects(OtherCharacter);
                                        }
                                }
                        }
                }
        }
        DrawDebugSphere(GetWorld(), SlamCenter, EffectiveRadius, 16, FColor::Red, false, 0.5f);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Chain slam!"));
}
bool AMalkholmCharacter::IsAtSlamTip(AUnshackledCharacterBase* Target) const
{
        float DistanceToTarget = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());
        float TipDistance = SlamTipDistance * GetRangeMultiplier();
        if (DistanceToTarget >= TipDistance)
        {
                return true;
        }
        return false;
}
void AMalkholmCharacter::ApplySlamEffects(AUnshackledCharacterBase* Target)
{
        if (Target->Health->IsDead())
        {
                return;
        }
        FStatModifier SlamSlow;
        SlamSlow.SourceId = FName("MalkholmSlamSlow");
        SlamSlow.Stat = ECharacterStat::Speed;
        SlamSlow.Kind = EModifierKind::Percent;
        SlamSlow.Amount = 0.0f - SlamSlowFraction;
        SlamSlow.Duration = SlamSlowDuration;
        SlamSlow.Rule = EStackRule::RefreshSameSource;
        Target->Stats->AddModifier(SlamSlow);
        if (!IsAtSlamTip(Target))
        {
                return;
        }
        FStatModifier SlamRoot;
        SlamRoot.SourceId = FName("MalkholmSlamRoot");
        SlamRoot.Stat = ECharacterStat::Speed;
        SlamRoot.Kind = EModifierKind::Flat;
        SlamRoot.Amount = 0.0f;
        SlamRoot.Duration = SlamRootDuration;
        SlamRoot.Rule = EStackRule::RefreshSameSource;
        SlamRoot.Flag = EStatusFlag::Root;
        Target->Stats->AddModifier(SlamRoot);
}
void AMalkholmCharacter::PerformRetribution()
{
        Super::PerformRetribution();
        if (Health->IsDead())
        {
                return;
        }
        float HealthCost = Health->CurrentHealth * RetributionHealthCost;
        Health->TakeHit(HealthCost);
        RetributionTimeLeft = RetributionDuration;
        FStatModifier DamageBonus;
        DamageBonus.SourceId = FName("MalkholmRetribution");
        DamageBonus.Stat = ECharacterStat::Damage;
        DamageBonus.Kind = EModifierKind::Percent;
        DamageBonus.Amount = RetributionDamageBonus;
        DamageBonus.Duration = RetributionDuration;
        DamageBonus.Rule = EStackRule::RefreshSameSource;
        Stats->AddModifier(DamageBonus);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Mal'Kholm Retribution"));
}
void AMalkholmCharacter::TickRetribution(float DeltaTime)
{
        if (RetributionTimeLeft <= 0.0f)
        {
                return;
        }
        if (Health->IsDead())
        {
                RetributionTimeLeft = 0.0f;
                return;
        }
        RetributionTimeLeft = RetributionTimeLeft - DeltaTime;
        if (RetributionTimeLeft <= 0.0f)
        {
                RetributionTimeLeft = 0.0f;
                EndRetribution();
        }
}
void AMalkholmCharacter::EndRetribution()
{
        float MissingHealth = Health->MaxHealth - Health->CurrentHealth;
        float MissingHeal = MissingHealth * RetributionMissingHealHeal;
        float MaxHeal = Health->MaxHealth * RetributionMaxHealHeal;
        Health->Heal(MissingHeal + MaxHeal);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Mal'Kholm Retribution ended"));
}
bool AMalkholmCharacter::WouldDie()
{
        bool bBaseCancelled = Super::WouldDie();
        if (bBaseCancelled)
        {
                return true;
        }
        if (PassiveCooldownLeft > 0.0f)
        {
                return false;
        }
        Health->CurrentHealth = Health->MaxHealth;
        PassiveCooldownLeft = PassiveCooldown;
        GiveCheatDeathBuffs();
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Mal'Kholm cheated death!"));
        return true;
}
void AMalkholmCharacter::GiveCheatDeathBuffs()
{
        FStatModifier DefenseLoss;
        DefenseLoss.SourceId = FName("MalkholmCheatDeath");
        DefenseLoss.Stat = ECharacterStat::Defense;
        DefenseLoss.Kind = EModifierKind::Flat;
        DefenseLoss.Amount = 0.0f - PassiveDefenseLoss;
        DefenseLoss.Duration = PassiveBuffDuration;
        DefenseLoss.Rule = EStackRule::RefreshSameSource;
        Stats->AddModifier(DefenseLoss);
        FStatModifier LifestealGain;
        LifestealGain.SourceId = FName("MalkholmCheatDeath");
        LifestealGain.Stat = ECharacterStat::Lifesteal;
        LifestealGain.Kind = EModifierKind::Flat;
        LifestealGain.Amount = PassiveLifesteal;
        LifestealGain.Duration = PassiveBuffDuration;
        LifestealGain.Rule = EStackRule::RefreshSameSource;
        Stats->AddModifier(LifestealGain);
}
void AMalkholmCharacter::TickPassiveCooldown(float DeltaTime)
{
        if (PassiveCooldownLeft <= 0.0f)
        {
                return;
        }
        PassiveCooldownLeft = PassiveCooldownLeft - DeltaTime;
        if (PassiveCooldownLeft < 0.0f)
        {
                PassiveCooldownLeft = 0.0f;
        }
}
