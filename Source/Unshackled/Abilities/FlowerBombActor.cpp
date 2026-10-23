#include "FlowerBombActor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
AFlowerBombActor::AFlowerBombActor()
{
        PrimaryActorTick.bCanEverTick = true;
        USceneComponent* RootPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
        SetRootComponent(RootPoint);
        Visual = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Visual"));
        Visual->SetupAttachment(RootPoint);
        Visual->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void AFlowerBombActor::Setup(AUnshackledCharacterBase* InOwner)
{
        if (InOwner != nullptr)
        {
                bOwnerIsEnemy = InOwner->bIsEnemy;
        }
}
void AFlowerBombActor::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        AgeSeconds = AgeSeconds + DeltaTime;
        if (AgeSeconds >= Lifetime)
        {
                Destroy();
                return;
        }
        if (IsEnemyOnBomb())
        {
                Explode();
                return;
        }
        DrawDebugShapeIfNoSprite();
}
bool AFlowerBombActor::IsEnemyOnBomb()
{
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (Candidate->bIsEnemy != bOwnerIsEnemy)
                {
                        if (!Candidate->Health->IsDead())
                        {
                                float Distance = FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation());
                                if (Distance <= TriggerRadius)
                                {
                                        return true;
                                }
                        }
                }
        }
        return false;
}
void AFlowerBombActor::Explode()
{
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (Candidate->bIsEnemy != bOwnerIsEnemy)
                {
                        if (!Candidate->Health->IsDead())
                        {
                                float Distance = FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation());
                                if (Distance <= BlastRadius)
                                {
                                        ApplyDebuffs(Candidate);
                                }
                        }
                }
        }
        DrawDebugSphere(GetWorld(), GetActorLocation(), BlastRadius, 20, FColor::Orange, false, 0.5f);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("Flower bomb!"));
        Destroy();
}
void AFlowerBombActor::ApplyDebuffs(AUnshackledCharacterBase* Target)
{
        FStatModifier DefenseLoss;
        DefenseLoss.SourceId = FName("HanuboyBomb");
        DefenseLoss.Stat = ECharacterStat::Defense;
        DefenseLoss.Kind = EModifierKind::Percent;
        DefenseLoss.Amount = 0.0f - DefenseReduction;
        DefenseLoss.Duration = DebuffDuration;
        DefenseLoss.Rule = EStackRule::RefreshSameSource;
        Target->Stats->AddModifier(DefenseLoss);
        FStatModifier DamageLoss;
        DamageLoss.SourceId = FName("HanuboyBomb");
        DamageLoss.Stat = ECharacterStat::Damage;
        DamageLoss.Kind = EModifierKind::Percent;
        DamageLoss.Amount = 0.0f - DamageReduction;
        DamageLoss.Duration = DebuffDuration;
        DamageLoss.Rule = EStackRule::RefreshSameSource;
        Target->Stats->AddModifier(DamageLoss);
        FStatModifier Slow;
        Slow.SourceId = FName("HanuboyBomb");
        Slow.Stat = ECharacterStat::Speed;
        Slow.Kind = EModifierKind::Percent;
        Slow.Amount = 0.0f - SlowFraction;
        Slow.Duration = DebuffDuration;
        Slow.Rule = EStackRule::RefreshSameSource;
        Target->Stats->AddModifier(Slow);
}
void AFlowerBombActor::DrawDebugShapeIfNoSprite()
{
        if (!bDrawDebugShape)
        {
                return;
        }
        if (Visual->GetFlipbook() != nullptr)
        {
                return;
        }
        DrawDebugSphere(GetWorld(), GetActorLocation(), TriggerRadius, 12, FColor::Orange, false, 0.0f);
}
