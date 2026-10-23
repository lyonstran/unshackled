#include "KaliaCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
AKaliaCharacter::AKaliaCharacter()
{
        DisplayName = TEXT("Kalia");
        AuraColor = FLinearColor(1.0f, 0.1f, 0.1f);
        AttackDamage = 28.0f;
        AttackCooldown = 0.45f;
        Health->MaxHealth = 80.0f;
        Stats->BaseDefense = 10.0f;
}
void AKaliaCharacter::PerformBasicAttack(FVector Direction)
{
        Direction.Z = 0.0f;
        Direction.Normalize();
        FVector SlashCenter = GetActorLocation() + (Direction * DaggerRange);
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        FCollisionShape SlashShape = FCollisionShape::MakeSphere(DaggerRadius);
        GetWorld()->OverlapMultiByObjectType(OverlapResults, SlashCenter, FQuat::Identity, ObjectParams, SlashShape, QueryParams);
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
                                                DealDamage(OtherCharacter, DaggerDamageScale, false);
                                        }
                                }
                        }
                }
        }
        DrawDebugSphere(GetWorld(), SlashCenter, DaggerRadius, 12, FColor::Red, false, 0.15f);
}
void AKaliaCharacter::OnKilledTarget(AUnshackledCharacterBase* Target)
{
        Super::OnKilledTarget(Target);
        if (Target == nullptr)
        {
                return;
        }
        if (Target->bIsEnemy == bIsEnemy)
        {
                return;
        }
        FreezeEnemiesAround(Target->GetActorLocation());
}
void AKaliaCharacter::FreezeEnemiesAround(FVector Center)
{
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        FCollisionShape ExplosionShape = FCollisionShape::MakeSphere(ExplosionRadius);
        GetWorld()->OverlapMultiByObjectType(OverlapResults, Center, FQuat::Identity, ObjectParams, ExplosionShape, QueryParams);
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
                                                FreezeEnemy(OtherCharacter);
                                        }
                                }
                        }
                }
        }
        DrawDebugSphere(GetWorld(), Center, ExplosionRadius, 16, FColor::Cyan, false, 0.5f);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Kalia freeze explosion!"));
}
void AKaliaCharacter::FreezeEnemy(AUnshackledCharacterBase* Enemy)
{
        if (Enemy->Health->IsDead())
        {
                return;
        }
        FStatModifier FreezeModifier;
        FreezeModifier.SourceId = FName("KaliaFreeze");
        FreezeModifier.Stat = ECharacterStat::Speed;
        FreezeModifier.Kind = EModifierKind::Flat;
        FreezeModifier.Amount = 0.0f;
        FreezeModifier.Duration = FreezeDuration;
        FreezeModifier.Rule = EStackRule::RefreshSameSource;
        FreezeModifier.Flag = EStatusFlag::Freeze;
        Enemy->Stats->AddModifier(FreezeModifier);
}
void AKaliaCharacter::PerformRetribution()
{
        Super::PerformRetribution();
        if (Health->IsDead())
        {
                return;
        }
        AddRetributionModifier(ECharacterStat::Speed, EModifierKind::Percent, RetributionSpeedBonus);
        AddRetributionModifier(ECharacterStat::Defense, EModifierKind::Percent, RetributionDefenseBonus);
        AddRetributionModifier(ECharacterStat::Damage, EModifierKind::Percent, RetributionDamageBonus);
        AddRetributionModifier(ECharacterStat::Lifesteal, EModifierKind::Flat, RetributionLifesteal);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Kalia Retribution"));
}
void AKaliaCharacter::AddRetributionModifier(ECharacterStat Stat, EModifierKind Kind, float Amount)
{
        FStatModifier Modifier;
        Modifier.SourceId = FName("KaliaRetribution");
        Modifier.Stat = Stat;
        Modifier.Kind = Kind;
        Modifier.Amount = Amount;
        Modifier.Duration = RetributionDuration;
        Modifier.Rule = EStackRule::RefreshSameSource;
        Stats->AddModifier(Modifier);
}
