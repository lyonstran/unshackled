#pragma once
#include "CoreMinimal.h"
#include "PartyMemberCharacter.h"
#include "Projectiles/AbilityProjectile.h"
#include "Abilities/TornadoActor.h"
#include "Abilities/VeilActor.h"
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
	UPROPERTY(EditAnywhere, Category = "Sirius")
	TSubclassOf<ATornadoActor> TornadoClass;
	UPROPERTY(EditAnywhere, Category = "Sirius")
	int32 EmpoweredAttackCount = 3;
	UPROPERTY(EditAnywhere, Category = "Sirius")
	float EmpoweredSizeScale = 2.0f;
	UPROPERTY(EditAnywhere, Category = "Sirius")
	float EmpoweredDamageScale = 1.5f;
	UPROPERTY(EditAnywhere, Category = "Sirius")
	float TornadoSpawnOffset = 150.0f;
	UPROPERTY(EditAnywhere, Category = "Sirius")
	TSubclassOf<AVeilActor> VeilClass;
protected:
	virtual void PerformBasicAttack(FVector Direction) override;
	virtual void PerformSpecial(FVector AimDirection) override;
	virtual void PerformUltimate(FVector AimDirection) override;
	void FireCrescent(FVector Direction, float DamageScale, float SizeScale);
	void SpawnTornado(FVector Direction);
	void SpawnVeil();
	int32 EmpoweredAttacksLeft = 0;
	UPROPERTY()
	AVeilActor* ActiveVeil;
};