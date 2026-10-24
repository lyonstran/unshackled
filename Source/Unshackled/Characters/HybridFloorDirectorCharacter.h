#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "HybridFloorDirectorCharacter.generated.h"
enum class EHybridDirectorState : uint8
{
	Chasing,
	RamWinding,
	Charging,
	Recovering,
	BeamWinding,
	Beaming
};
UCLASS()
class UNSHACKLED_API AHybridFloorDirectorCharacter : public AEnemyCharacter
{
	GENERATED_BODY()
public:
	AHybridFloorDirectorCharacter();
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director")
	float CycleCooldown = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director")
	float StopDistance = 150.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Ram")
	float RamTriggerDistance = 600.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Ram")
	float RamWindupTime = 3.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Ram")
	float RamSpeed = 1800.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Ram")
	float RamDistance = 900.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Ram")
	float RamHitRadius = 80.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Ram")
	float RamMaxHealthFraction = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Ram")
	float RamStunDuration = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Ram")
	float RamRecoverTime = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Beam")
	float BeamWindupTime = 3.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Beam")
	float BeamDuration = 3.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Beam")
	float BeamLength = 1500.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Beam")
	float BeamHalfWidth = 60.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Beam")
	float BeamTurnSpeed = 80.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Beam")
	float BeamTickInterval = 0.1f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Beam")
	float BeamMaxHealthFraction = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Beam")
	float BeamSlowFraction = 0.15f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Beam")
	float BeamSlowDuration = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Summon")
	TSubclassOf<AEnemyCharacter> GuardClass;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Summon")
	TSubclassOf<AEnemyCharacter> CasterClass;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Summon")
	float SummonInterval = 12.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Summon")
	float FirstSummonDelay = 3.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Summon")
	int32 SummonCount = 2;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Summon")
	int32 MaxSummoned = 4;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Summon")
	float SummonRingRadius = 150.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Circle")
	float CircleRadius = 400.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Circle")
	float CircleDefenseLoss = 0.15f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Circle")
	float CircleInterval = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Circle")
	float CircleDuration = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Rally")
	float RallyRadius = 500.0f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Rally")
	float RallyDefenseBonus = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Rally")
	float RallyDamageBonus = 0.1f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Rally")
	float RallyInterval = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Hybrid Director Rally")
	float RallyDuration = 0.5f;
protected:
	void TickChasing(float DeltaTime);
	void TickRamWinding(float DeltaTime);
	void TickCharging(float DeltaTime);
	void TickRecovering(float DeltaTime);
	void TickBeamWinding(float DeltaTime);
	void TickBeaming(float DeltaTime);
	void MoveToward(FVector Direction);
	void StartCharge(FVector Direction);
	void EndCharge();
	void StartBeamWindup();
	void StartBeam();
	void EndCycle();
	bool EnsureBeamTarget();
	void TurnBeamToward(AUnshackledCharacterBase* Target, float DeltaTime);
	bool CanBeHit(AUnshackledCharacterBase* Candidate);
	AUnshackledCharacterBase* FindRamVictim();
	void HitWithRam(AUnshackledCharacterBase* Victim);
	bool IsInBeam(AUnshackledCharacterBase* Candidate);
	void DamageVictimsInBeam(float ElapsedTime);
	void HitWithBeam(AUnshackledCharacterBase* Victim, float ElapsedTime);
	void TickSummon(float DeltaTime);
	int32 CountActiveSummons();
	void SummonPair();
	void TickMagicCircle(float DeltaTime);
	void ApplyCircleDebuff(AUnshackledCharacterBase* Ally);
	void TickRally(float DeltaTime);
	void ApplyRally(AUnshackledCharacterBase* Ally);
	EHybridDirectorState State = EHybridDirectorState::Chasing;
	float WindupTimeLeft = 0.0f;
	float RecoverTimeLeft = 0.0f;
	float ChargeDistanceLeft = 0.0f;
	float CycleCooldownLeft = 0.0f;
	float BeamTimeLeft = 0.0f;
	float BeamTickTimer = 0.0f;
	float SummonTimeLeft = 0.0f;
	float CircleTimer = 0.0f;
	float RallyTimer = 0.0f;
	bool bSummonGuardsNext = true;
	FVector ChargeDirection = FVector(1.0f, 0.0f, 0.0f);
	FVector BeamDirection = FVector(1.0f, 0.0f, 0.0f);
	UPROPERTY()
	AUnshackledCharacterBase* BeamTarget;
	UPROPERTY()
	TArray<AEnemyCharacter*> SummonedEnemies;
};
