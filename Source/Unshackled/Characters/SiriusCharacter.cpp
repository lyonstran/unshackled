#include "SiriusCharacter.h"
#include "Engine/World.h"

ASiriusCharacter::ASiriusCharacter()
{
}

void ASiriusCharacter::PerformBasicAttack(FVector Direction)
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
		Crescent->Launch(this, Direction, 1.0f, 1.0f);
	}
}

// NOTES
// Spawns the crescent a little in front of Sirius so it does not start inside him. Damage scale is 1.0 (a basic attack).
// The projectile damages through Sirius's DealDamage, so his Damage stat, lifesteal and passives all apply.