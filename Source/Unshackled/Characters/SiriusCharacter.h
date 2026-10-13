#pragma once
#include "CoreMinimal.h"
#include "PartyMemberCharacter.h"
#include "Projectiles/AbilityProjectile.h"
#include "SiriusCharacter.generated.h"

UCLASS()
class UNSHACKLED_API ASiriusCharacter : public APartyMemberCharacter
{
	GENERATED_BODY()

public:
	ASiriusCharacter();

	UPROPERTY(EditAnywhere, Category = "Sirius")
	TSubclassOf<AAbilityProjectile> CrescentClass;

	UPROPERTY(EditAnywhere, Category = "Sirius")
	float CrescentSpawnOffset = 60.0f;

protected:
	virtual void PerformBasicAttack(FVector Direction) override;
};

// NOTES
// CrescentClass is empty by default and falls back to the plain C++ projectile. Set it to BP_CrescentProjectile once it has art.
// Special (C), ultimate (V), the passive, and Retribution will be added to this class later.