#include "CasterGuardCharacter.h"
#include "Engine/World.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
ACasterGuardCharacter::ACasterGuardCharacter()
{
        StatsRowName = FName("CasterGuard");
        AttackDamage = 12.0f;
        Health->MaxHealth = 40.0f;
        Stats->BaseDefense = 10.0f;
}
void ACasterGuardCharacter::Tick(float DeltaTime)
{
        AUnshackledCharacterBase::Tick(DeltaTime);
        if (Health->IsDead())
        {
                return;
        }
        AUnshackledCharacterBase* Target = FindClosestTarget();
        if (Target == nullptr)
        {
                return;
        }
        FVector VectorToTarget = Target->GetActorLocation() - GetActorLocation();
        VectorToTarget.Z = 0.0f;
        float DistanceToTarget = VectorToTarget.Size();
        FVector DirectionToTarget = VectorToTarget;
        DirectionToTarget.Normalize();
        if (bCanAttack)
        {
                if (DistanceToTarget <= FireRange)
                {
                        FacingDirection = DirectionToTarget;
                        BasicAttack();
                }
                else
                {
                        MoveToward(DirectionToTarget);
                }
        }
        else
        {
                if (DistanceToTarget < RetreatDistance)
                {
                        MoveAwayFrom(DirectionToTarget);
                }
        }
}
void ACasterGuardCharacter::MoveToward(FVector Direction)
{
        FVector2D MoveInput = FVector2D(Direction.Y, Direction.X);
        MoveCharacter(MoveInput);
}
void ACasterGuardCharacter::MoveAwayFrom(FVector Direction)
{
        FVector2D MoveInput = FVector2D(-Direction.Y, -Direction.X);
        MoveCharacter(MoveInput);
}
void ACasterGuardCharacter::PerformBasicAttack(FVector Direction)
{
        TSubclassOf<AAbilityProjectile> ClassToSpawn = MissileClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = AAbilityProjectile::StaticClass();
        }
        Direction.Z = 0.0f;
        Direction.Normalize();
        FVector SpawnLocation = GetActorLocation() + (Direction * MissileSpawnOffset);
        FRotator SpawnRotation = Direction.Rotation();
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AAbilityProjectile* Missile = GetWorld()->SpawnActor<AAbilityProjectile>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
        if (Missile != nullptr)
        {
                Missile->Speed = MissileSpeed;
                Missile->MaxRange = MissileRange;
                Missile->Radius = MissileRadius;
                Missile->bPierces = false;
                Missile->Launch(this, Direction, MissileDamageScale, 1.0f);
        }
}
