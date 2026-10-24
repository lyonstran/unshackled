#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "MagicFloorDirectorCharacter.generated.h"
enum class EMagicDirectorState : uint8
{
	Idle,
	Winding,
	Firing
};
UCLASS()
class UNSHACKLED_API AMagicFloorDirectorCharacter : public AEnemyCharacter
{
	GENERATED_BODY()
public:
	AMagicFloorDirectorCharacter();
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, Category = "Magic Director Laser")
	float LaserWindupTime = 2.0f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Laser")
	float LaserDuration = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Laser")
	float LaserCooldown = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Laser")
	float LaserLength = 1500.0f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Laser")
	float LaserHalfWidth = 60.0f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Laser")
	float LaserMissingHealthFraction = 0.15f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Laser")
	float LaserMinMaxHealthFraction = 0.01f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Laser")
	float LaserDefenseLoss = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Laser")
	float LaserDefenseLossDuration = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Summon")
	TSubclassOf<AEnemyCharacter> GuardClass;
	UPROPERTY(EditAnywhere, Category = "Magic Director Summon")
	TSubclassOf<AEnemyCharacter> CasterClass;
	UPROPERTY(EditAnywhere, Category = "Magic Director Summon")
	float SummonInterval = 12.0f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Summon")
	float FirstSummonDelay = 3.0f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Summon")
	int32 SummonCount = 2;
	UPROPERTY(EditAnywhere, Category = "Magic Director Summon")
	int32 MaxSummoned = 4;
	UPROPERTY(EditAnywhere, Category = "Magic Director Summon")
	float SummonRingRadius = 150.0f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Circle")
	float CircleRadius = 400.0f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Circle")
	float CircleDefenseLoss = 0.15f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Circle")
	float CircleInterval = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Circle")
	float CircleDuration = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Rally")
	float RallyRadius = 500.0f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Rally")
	float RallyDefenseBonus = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Rally")
	float RallyDamageBonus = 0.1f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Rally")
	float RallyInterval = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Magic Director Rally")
	float RallyDuration = 0.5f;
protected:
	void TickIdle(float DeltaTime);
	void TickWinding(float DeltaTime);
	void TickFiring(float DeltaTime);
	void StartWindup(AUnshackledCharacterBase* Target);
	void StartFiring();
	void EndFiring();
	void AimAtLockedTarget();
	bool CanBeLasered(AUnshackledCharacterBase* Candidate);
	bool IsInBeam(AUnshackledCharacterBase* Candidate);
	bool WasAlreadyHit(AUnshackledCharacterBase* Candidate);
	void ApplyLaserToVictims();
	void HitWithLaser(AUnshackledCharacterBase* Victim);
	void TickSummon(float DeltaTime);
	int32 CountActiveSummons();
	void SummonPair();
	void TickMagicCircle(float DeltaTime);
	void ApplyCircleDebuff(AUnshackledCharacterBase* Ally);
	void TickRally(float DeltaTime);
	void ApplyRally(AUnshackledCharacterBase* Ally);
	EMagicDirectorState State = EMagicDirectorState::Idle;
	float WindupTimeLeft = 0.0f;
	float FiringTimeLeft = 0.0f;
	float LaserCooldownLeft = 0.0f;
	float SummonTimeLeft = 0.0f;
	float CircleTimer = 0.0f;
	float RallyTimer = 0.0f;
	bool bSummonGuardsNext = true;
	FVector LaserDirection = FVector(1.0f, 0.0f, 0.0f);
	UPROPERTY()
	AUnshackledCharacterBase* LockedTarget;
	UPROPERTY()
	TArray<AUnshackledCharacterBase*> LaserHitTargets;
	UPROPERTY()
	TArray<AEnemyCharacter*> SummonedEnemies;
};
