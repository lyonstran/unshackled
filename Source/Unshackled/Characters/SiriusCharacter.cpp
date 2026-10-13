#include "SiriusCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

ASiriusCharacter::ASiriusCharacter()
{
}

void ASiriusCharacter::PerformBasicAttack(FVector Direction)
{
        if (EmpoweredAttacksLeft > 0)
        {
                FireCrescent(Direction, EmpoweredDamageScale, EmpoweredSizeScale);

                EmpoweredAttacksLeft = EmpoweredAttacksLeft - 1;

                FString Message = FString::Printf(TEXT("Empowered attacks left: %d"), EmpoweredAttacksLeft);
                GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, Message);

                if (EmpoweredAttacksLeft == 0)
                {
                        SpawnTornado(Direction);
                }
        }
        else
        {
                FireCrescent(Direction, 1.0f, 1.0f);
        }
}

void ASiriusCharacter::PerformSpecial(FVector AimDirection)
{
        EmpoweredAttacksLeft = EmpoweredAttackCount;

        FString Message = FString::Printf(TEXT("Empowered attacks: %d"), EmpoweredAttacksLeft);
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, Message);
}

void ASiriusCharacter::PerformUltimate(FVector AimDirection)
{
        SpawnVeil();
}

void ASiriusCharacter::FireCrescent(FVector Direction, float DamageScale, float SizeScale)
{
        TSubclassOf<AAbilityProjectile> ClassToSpawn = CrescentClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = AAbilityProjectile::StaticClass();
        }

        FVector SpawnLocation = GetActorLocation() + (Direction * CrescentSpawnOffset);
        FRotator SpawnRotation = Direction.Rotation();

        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AAbilityProjectile* Crescent = GetWorld()->SpawnActor<AAbilityProjectile>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
        if (Crescent != nullptr)
        {
                Crescent->Launch(this, Direction, DamageScale, SizeScale);
        }
}

void ASiriusCharacter::SpawnTornado(FVector Direction)
{
        TSubclassOf<ATornadoActor> ClassToSpawn = TornadoClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = ATornadoActor::StaticClass();
        }

        FVector SpawnLocation = GetActorLocation() + (Direction * TornadoSpawnOffset);
        FRotator SpawnRotation = FRotator::ZeroRotator;

        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        ATornadoActor* Tornado = GetWorld()->SpawnActor<ATornadoActor>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
        if (Tornado != nullptr)
        {
                Tornado->Setup(this);
        }
}

void ASiriusCharacter::SpawnVeil()
{
        if (IsValid(ActiveVeil))
        {
                ActiveVeil->Destroy();
        }

        TSubclassOf<AVeilActor> ClassToSpawn = VeilClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = AVeilActor::StaticClass();
        }

        FVector SpawnLocation = GetActorLocation();
        FRotator SpawnRotation = FRotator::ZeroRotator;

        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        ActiveVeil = GetWorld()->SpawnActor<AVeilActor>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
        if (ActiveVeil != nullptr)
        {
                ActiveVeil->Setup(this);
        }
}

// X while empowered fires a bigger, harder crescent, counts down, and on the last one also spawns the tornado.
// X when not empowered fires a normal crescent (damage scale 1.0, size scale 1.0).
// C only sets the counter. The mana cost, cooldown and windup are handled by the base class before PerformSpecial runs.
// V only spawns the veil. The veil does all of its own work (follow, damage, slow, defense reduction).