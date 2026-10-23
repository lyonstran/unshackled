#include "WardenCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
AWardenCharacter::AWardenCharacter()
{
        StatsRowName = FName("Warden");
        AttackTriggerDistance = 110.0f;
        AttackDamage = 14.0f;
        AttackCooldown = 1.5f;
        Health->MaxHealth = 120.0f;
        Stats->BaseDefense = 35.0f;
}
void AWardenCharacter::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        TickShield();
}
void AWardenCharacter::PerformBasicAttack(FVector Direction)
{
        Direction.Z = 0.0f;
        Direction.Normalize();
        FVector SlamCenter = GetActorLocation() + (Direction * SlamRange);
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        FCollisionShape SlamShape = FCollisionShape::MakeSphere(SlamRadius);
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
        DrawDebugSphere(GetWorld(), SlamCenter, SlamRadius, 12, FColor::Yellow, false, 0.2f);
}
void AWardenCharacter::ApplySlamEffects(AUnshackledCharacterBase* Target)
{
        if (Target->Health->IsDead())
        {
                return;
        }
        FStatModifier SlamRoot;
        SlamRoot.SourceId = FName("WardenSlamRoot");
        SlamRoot.Stat = ECharacterStat::Speed;
        SlamRoot.Kind = EModifierKind::Flat;
        SlamRoot.Amount = 0.0f;
        SlamRoot.Duration = SlamRootDuration;
        SlamRoot.Rule = EStackRule::RefreshSameSource;
        SlamRoot.Flag = EStatusFlag::Root;
        Target->Stats->AddModifier(SlamRoot);
        FStatModifier DefenseLoss;
        DefenseLoss.SourceId = FName("WardenSlam");
        DefenseLoss.Stat = ECharacterStat::Defense;
        DefenseLoss.Kind = EModifierKind::Percent;
        DefenseLoss.Amount = 0.0f - SlamDefenseLoss;
        DefenseLoss.Duration = SlamDefenseLossDuration;
        DefenseLoss.Rule = EStackRule::RefreshSameSource;
        Target->Stats->AddModifier(DefenseLoss);
}
void AWardenCharacter::TickShield()
{
        if (bShieldRaised)
        {
                return;
        }
        if (Health->IsDead())
        {
                return;
        }
        float MaxHealthValue = Health->MaxHealth;
        if (MaxHealthValue <= 0.0f)
        {
                return;
        }
        float HealthFraction = Health->CurrentHealth / MaxHealthValue;
        if (HealthFraction > ShieldHealthFraction)
        {
                return;
        }
        bShieldRaised = true;
        FStatModifier ShieldBonus;
        ShieldBonus.SourceId = FName("WardenShield");
        ShieldBonus.Stat = ECharacterStat::Defense;
        ShieldBonus.Kind = EModifierKind::Percent;
        ShieldBonus.Amount = ShieldDefenseBonus;
        ShieldBonus.Duration = 0.0f;
        ShieldBonus.Rule = EStackRule::RefreshSameSource;
        Stats->AddModifier(ShieldBonus);
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, TEXT("Warden raised its shield!"));
}
