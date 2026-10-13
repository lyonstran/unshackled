#include "AbilityProjectile.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"

AAbilityProjectile::AAbilityProjectile()
{
        PrimaryActorTick.bCanEverTick = true;

        USceneComponent* RootPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
        SetRootComponent(RootPoint);

        Visual = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Visual"));
        Visual->SetupAttachment(RootPoint);
        Visual->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAbilityProjectile::Launch(AUnshackledCharacterBase* InOwner, FVector Direction, float InDamageScale, float InSizeScale)
{
        OwnerCharacter = InOwner;
        DamageScale = InDamageScale;
        SizeScale = InSizeScale;

        Direction.Z = 0.0f;
        Direction.Normalize();
        TravelDirection = Direction;

        SetActorRotation(TravelDirection.Rotation());
        SetActorScale3D(FVector(SizeScale, SizeScale, SizeScale));
}

void AAbilityProjectile::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);

        if (OwnerCharacter == nullptr)
        {
                Destroy();
                return;
        }

        MoveForward(DeltaTime);

        bool bHitSomething = HitOverlappingCharacters();
        if (bHitSomething && !bPierces)
        {
                Destroy();
                return;
        }

        DrawDebugShapeIfNoSprite();

        if (DistanceTraveled >= MaxRange)
        {
                Destroy();
        }
}

void AAbilityProjectile::MoveForward(float DeltaTime)
{
        float DistanceThisFrame = Speed * DeltaTime;
        FVector NewLocation = GetActorLocation() + (TravelDirection * DistanceThisFrame);
        SetActorLocation(NewLocation);

        DistanceTraveled = DistanceTraveled + DistanceThisFrame;
}

bool AAbilityProjectile::HitOverlappingCharacters()
{
        bool bHitSomething = false;

        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        QueryParams.AddIgnoredActor(OwnerCharacter);

        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

        float CurrentRadius = Radius * SizeScale;
        FCollisionShape Sphere = FCollisionShape::MakeSphere(CurrentRadius);

        GetWorld()->OverlapMultiByObjectType(OverlapResults, GetActorLocation(), FQuat::Identity, ObjectParams, Sphere, QueryParams);

        for (int32 i = 0; i < OverlapResults.Num(); i++)
        {
                AActor* HitActor = OverlapResults[i].GetActor();
                if (HitActor == nullptr)
                {
                        continue;
                }

                if (ActorsAlreadyHit.Contains(HitActor))
                {
                        continue;
                }
                ActorsAlreadyHit.Add(HitActor);

                AUnshackledCharacterBase* Target = Cast<AUnshackledCharacterBase>(HitActor);
                if (Target == nullptr)
                {
                        continue;
                }
                if (Target->bIsEnemy == OwnerCharacter->bIsEnemy)
                {
                        continue;
                }

                OwnerCharacter->DealDamage(Target, DamageScale, false);
                bHitSomething = true;
        }

        return bHitSomething;
}

void AAbilityProjectile::DrawDebugShapeIfNoSprite()
{
        if (!bDrawDebugShape)
        {
                return;
        }
        if (Visual->GetFlipbook() != nullptr)
        {
                return;
        }

        float CurrentRadius = Radius * SizeScale;
        DrawDebugSphere(GetWorld(), GetActorLocation(), CurrentRadius, 16, FColor::Cyan, false, 0.0f);
}


// Hit checks poll an overlap sphere each frame (no overlap delegates). Each actor is hit once per projectile.
// If the owner is gone (destroyed), the projectile destroys itself.