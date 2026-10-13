#include "TornadoActor.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Components/HealthComponent.h"
ATornadoActor::ATornadoActor()
{
        PrimaryActorTick.bCanEverTick = true;
        USceneComponent* RootPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
        SetRootComponent(RootPoint);
        Visual = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Visual"));
        Visual->SetupAttachment(RootPoint);
        Visual->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void ATornadoActor::Setup(AUnshackledCharacterBase* InOwner)
{
        OwnerCharacter = InOwner;
}
void ATornadoActor::Tick(float DeltaTime)
{
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
        ChaseNearestEnemy(DeltaTime);
        TimeSinceDamage = TimeSinceDamage + DeltaTime;
        if (TimeSinceDamage >= DamageInterval)
        {
                TimeSinceDamage = 0.0f;
                DamageEnemiesInside();
        }
        DrawDebugShapeIfNoSprite();
}
AUnshackledCharacterBase* ATornadoActor::FindNearestEnemy()
{
        AUnshackledCharacterBase* NearestEnemy = nullptr;
        float NearestDistance = 0.0f;
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (Candidate != nullptr)
                {
                        if (Candidate->bIsEnemy != OwnerCharacter->bIsEnemy)
                        {
                                if (!Candidate->Health->IsDead())
                                {
                                        float Distance = FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation());
                                        if (NearestEnemy == nullptr)
                                        {
                                                NearestEnemy = Candidate;
                                                NearestDistance = Distance;
                                        }
                                        else
                                        {
                                                if (Distance < NearestDistance)
                                                {
                                                        NearestEnemy = Candidate;
                                                        NearestDistance = Distance;
                                                }
                                        }
                                }
                        }
                }
        }
        return NearestEnemy;
}
void ATornadoActor::ChaseNearestEnemy(float DeltaTime)
{
        AUnshackledCharacterBase* Target = FindNearestEnemy();
        if (Target == nullptr)
        {
                return;
        }
        float DistanceToTarget = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());
        if (DistanceToTarget <= StopDistance)
        {
                return;
        }
        FVector DirectionToTarget = Target->GetActorLocation() - GetActorLocation();
        DirectionToTarget.Z = 0.0f;
        DirectionToTarget.Normalize();
        FVector NewLocation = GetActorLocation() + (DirectionToTarget * MoveSpeed * DeltaTime);
        SetActorLocation(NewLocation);
}
void ATornadoActor::DamageEnemiesInside()
{
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.AddIgnoredActor(OwnerCharacter);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
        GetWorld()->OverlapMultiByObjectType(OverlapResults, GetActorLocation(), FQuat::Identity, ObjectParams, Sphere, QueryParams);
        TArray<AActor*> ActorsHitThisTick;
        for (int32 i = 0; i < OverlapResults.Num(); i++)
        {
                AActor* HitActor = OverlapResults[i].GetActor();
                if (HitActor != nullptr)
                {
                        if (!ActorsHitThisTick.Contains(HitActor))
                        {
                                ActorsHitThisTick.Add(HitActor);
                                AUnshackledCharacterBase* Target = Cast<AUnshackledCharacterBase>(HitActor);
                                if (Target != nullptr)
                                {
                                        if (Target->bIsEnemy != OwnerCharacter->bIsEnemy)
                                        {
                                                OwnerCharacter->DealDamage(Target, DamageScale, false);
                                        }
                                }
                        }
                }
        }
}
void ATornadoActor::DrawDebugShapeIfNoSprite()
{
        if (!bDrawDebugShape)
        {
                return;
        }
        if (Visual->GetFlipbook() != nullptr)
        {
                return;
        }
        DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 16, FColor::Green, false, 0.0f);
}
