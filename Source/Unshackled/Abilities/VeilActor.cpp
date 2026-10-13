#include "VeilActor.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"

AVeilActor::AVeilActor()
{
        PrimaryActorTick.bCanEverTick = true;
        USceneComponent* RootPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
        SetRootComponent(RootPoint);
        Visual = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Visual"));
        Visual->SetupAttachment(RootPoint);
        Visual->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AVeilActor::Setup(AUnshackledCharacterBase* InOwner)
{
        OwnerCharacter = InOwner;
        FollowOwner();
}

void AVeilActor::Tick(float DeltaTime) {
        Super::Tick(DeltaTime);

        if (OwnerCharacter == nullptr)
        {
                Destroy();
                return;
        }

        AgeSeconds = AgeSeconds + DeltaTime;
        if (AgeSeconds >= Lifetime)
        {
                Destroy();
                return;
        }

        FollowOwner();

        TimeSinceAura = TimeSinceAura + DeltaTime;
        if (TimeSinceAura >= AuraInterval)
        {
                TimeSinceAura = 0.0f;
                ApplyDebuffsToEnemies();
        }

        TimeSinceDamage = TimeSinceDamage + DeltaTime;
        if (TimeSinceDamage >= DamageInterval)
        {
                TimeSinceDamage = 0.0f;
                DamageEnemies();
        }

        DrawDebugShapeIfNoSprite();
}

void AVeilActor::FollowOwner()
{
        SetActorLocation(OwnerCharacter->GetActorLocation());
}

TArray<AUnshackledCharacterBase*> AVeilActor::FindEnemiesInside()
{
        TArray<AUnshackledCharacterBase*> EnemiesInside;

        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.AddIgnoredActor(OwnerCharacter);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
        GetWorld()->OverlapMultiByObjectType(OverlapResults, GetActorLocation(), FQuat::Identity, ObjectParams, Sphere, QueryParams);
        for (int32 i = 0; i < OverlapResults.Num(); i++)
        {
                AUnshackledCharacterBase* Target = Cast<AUnshackledCharacterBase>(OverlapResults[i].GetActor());
                if (Target != nullptr)
                {
                        if (Target->bIsEnemy != OwnerCharacter->bIsEnemy)
                        {
                                if (!EnemiesInside.Contains(Target))
                                {
                                        EnemiesInside.Add(Target);
                                }
                        }
                }
        }

        return EnemiesInside;
}

void AVeilActor::ApplyDebuffsToEnemies()
{
        TArray<AUnshackledCharacterBase*> EnemiesInside = FindEnemiesInside();
        for (int32 i = 0; i < EnemiesInside.Num(); i++)
        {
                ApplyDebuffsToTarget(EnemiesInside[i]);
        }
}

void AVeilActor::ApplyDebuffsToTarget(AUnshackledCharacterBase* Target)
{
        FStatModifier SlowModifier;
        SlowModifier.SourceId = FName("SiriusVeil");
        SlowModifier.Stat = ECharacterStat::Speed;
        SlowModifier.Kind = EModifierKind::Percent;
        SlowModifier.Amount = 0.0f - SlowFraction;
        SlowModifier.Duration = AuraDuration;
        SlowModifier.Rule = EStackRule::RefreshSameSource;
        Target->Stats->AddModifier(SlowModifier);

        FStatModifier DefenseModifier;
        DefenseModifier.SourceId = FName("SiriusVeil");
        DefenseModifier.Stat = ECharacterStat::Defense;
        DefenseModifier.Kind = EModifierKind::Percent;
        DefenseModifier.Amount = 0.0f - DefenseReductionFraction;
        DefenseModifier.Duration = AuraDuration;
        DefenseModifier.Rule = EStackRule::RefreshSameSource;
        Target->Stats->AddModifier(DefenseModifier);
}

void AVeilActor::DamageEnemies()
{
        TArray<AUnshackledCharacterBase*> EnemiesInside = FindEnemiesInside();
        for (int32 i = 0; i < EnemiesInside.Num(); i++)
        {
                OwnerCharacter->DealDamage(EnemiesInside[i], DamageScale, false);
        }
}

void AVeilActor::DrawDebugShapeIfNoSprite()
{
        if (!bDrawDebugShape)
        {
                return;
        }
        if (Visual->GetFlipbook() != nullptr)
        {
                return;
        }
        DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 24, FColor::Purple, false, 0.0f);
}