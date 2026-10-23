#include "HanuboyCharacter.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
AHanuboyCharacter::AHanuboyCharacter()
{
        DisplayName = TEXT("Hanuboy");
        AuraColor = FLinearColor(0.1f, 1.0f, 0.2f);
        AttackDamage = 16.0f;
        AttackCooldown = 0.6f;
        Health->MaxHealth = 120.0f;
        Stats->BaseDefense = 25.0f;
}
void AHanuboyCharacter::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        TickBombs(DeltaTime);
}
void AHanuboyCharacter::PerformBasicAttack(FVector Direction)
{
        Direction.Z = 0.0f;
        Direction.Normalize();
        FVector Origin = GetActorLocation();
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        FCollisionShape ReachShape = FCollisionShape::MakeSphere(BallLength);
        GetWorld()->OverlapMultiByObjectType(OverlapResults, Origin, FQuat::Identity, ObjectParams, ReachShape, QueryParams);
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
                                                if (IsInBallLane(OtherCharacter, Direction))
                                                {
                                                        DealDamage(OtherCharacter, BallDamageScale, false);
                                                        KnockBack(OtherCharacter);
                                                }
                                        }
                                }
                        }
                }
        }
        FVector LaneStart = Origin - (Direction * BallLength);
        FVector LaneEnd = Origin + (Direction * BallLength);
        DrawDebugLine(GetWorld(), LaneStart, LaneEnd, FColor::Green, false, 0.2f, 0, 6.0f);
}
bool AHanuboyCharacter::IsInBallLane(AUnshackledCharacterBase* Target, FVector Direction) const
{
        FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
        ToTarget.Z = 0.0f;
        float DistanceAlong = FVector::DotProduct(ToTarget, Direction);
        FVector PointOnLine = Direction * DistanceAlong;
        float SidewaysDistance = (ToTarget - PointOnLine).Size();
        if (SidewaysDistance <= BallHalfWidth)
        {
                return true;
        }
        return false;
}
void AHanuboyCharacter::KnockBack(AUnshackledCharacterBase* Target)
{
        if (Target->Health->IsDead())
        {
                return;
        }
        FVector PushDirection = Target->GetActorLocation() - GetActorLocation();
        PushDirection.Z = 0.0f;
        if (PushDirection.IsNearlyZero())
        {
                PushDirection = GetFacingDirection();
        }
        PushDirection.Normalize();
        FVector NewLocation = Target->GetActorLocation() + (PushDirection * BallKnockbackDistance);
        Target->SetActorLocation(NewLocation);
}
void AHanuboyCharacter::PerformSpecial(FVector AimDirection)
{
        SpawnFlower(AimDirection);
}
void AHanuboyCharacter::SpawnFlower(FVector Direction)
{
        TSubclassOf<AFlowerCharacter> ClassToSpawn = FlowerClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = AFlowerCharacter::StaticClass();
        }
        Direction.Z = 0.0f;
        Direction.Normalize();
        FVector SpawnLocation = GetActorLocation() + (Direction * FlowerSpawnOffset);
        FRotator SpawnRotation = FRotator::ZeroRotator;
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        GetWorld()->SpawnActor<AFlowerCharacter>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
}
void AHanuboyCharacter::TickBombs(float DeltaTime)
{
        if (Health->IsDead())
        {
                return;
        }
        if (!Stats->CanAct())
        {
                return;
        }
        BombTimeLeft = BombTimeLeft - DeltaTime;
        if (BombTimeLeft > 0.0f)
        {
                return;
        }
        BombTimeLeft = 0.0f;
        int32 ActiveBombs = CountActiveBombs();
        if (ActiveBombs >= MaxActiveBombs)
        {
                return;
        }
        AUnshackledCharacterBase* Target = FindNearestEnemyInRange();
        if (Target == nullptr)
        {
                return;
        }
        ThrowBomb(Target);
        BombTimeLeft = BombInterval;
}
AUnshackledCharacterBase* AHanuboyCharacter::FindNearestEnemyInRange()
{
        AUnshackledCharacterBase* NearestEnemy = nullptr;
        float NearestDistance = BombSearchRange;
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (Candidate->bIsEnemy != bIsEnemy)
                {
                        if (!Candidate->Health->IsDead())
                        {
                                float Distance = FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation());
                                if (Distance <= NearestDistance)
                                {
                                        NearestDistance = Distance;
                                        NearestEnemy = Candidate;
                                }
                        }
                }
        }
        return NearestEnemy;
}
int32 AHanuboyCharacter::CountActiveBombs()
{
        int32 BombCount = 0;
        for (TActorIterator<AFlowerBombActor> It(GetWorld()); It; ++It)
        {
                BombCount = BombCount + 1;
        }
        return BombCount;
}
void AHanuboyCharacter::ThrowBomb(AUnshackledCharacterBase* Target)
{
        FVector Direction = Target->GetActorLocation() - GetActorLocation();
        Direction.Z = 0.0f;
        if (Direction.IsNearlyZero())
        {
                Direction = GetFacingDirection();
        }
        Direction.Normalize();
        TSubclassOf<AFlowerBombActor> ClassToSpawn = BombClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = AFlowerBombActor::StaticClass();
        }
        float ThrowDistance = BombThrowDistance;
        float DistanceToTarget = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());
        if (DistanceToTarget < ThrowDistance)
        {
                ThrowDistance = DistanceToTarget;
        }
        FVector SpawnLocation = GetActorLocation() + (Direction * ThrowDistance);
        FRotator SpawnRotation = FRotator::ZeroRotator;
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AFlowerBombActor* Bomb = GetWorld()->SpawnActor<AFlowerBombActor>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
        if (Bomb != nullptr)
        {
                Bomb->Setup(this);
        }
}
