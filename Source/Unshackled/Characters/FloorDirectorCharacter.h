#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "FloorDirectorCharacter.generated.h"
enum class EDirectorState : uint8
{
        Chasing,
        Winding,
        Charging,
        Recovering
};
UCLASS()
class UNSHACKLED_API AFloorDirectorCharacter : public AEnemyCharacter
{
        GENERATED_BODY()
public:
        AFloorDirectorCharacter();
        virtual void Tick(float DeltaTime) override;
        UPROPERTY(EditAnywhere, Category = "Floor Director")
        float RamTriggerDistance = 600.0f;
        UPROPERTY(EditAnywhere, Category = "Floor Director")
        float StopDistance = 150.0f;
        UPROPERTY(EditAnywhere, Category = "Floor Director")
        float RamWindupTime = 3.0f;
        UPROPERTY(EditAnywhere, Category = "Floor Director")
        float RamSpeed = 1800.0f;
        UPROPERTY(EditAnywhere, Category = "Floor Director")
        float RamDistance = 900.0f;
        UPROPERTY(EditAnywhere, Category = "Floor Director")
        float RamHitRadius = 80.0f;
        UPROPERTY(EditAnywhere, Category = "Floor Director")
        float RamMaxHealthFraction = 0.25f;
        UPROPERTY(EditAnywhere, Category = "Floor Director")
        float RamStunDuration = 1.0f;
        UPROPERTY(EditAnywhere, Category = "Floor Director")
        float RamRecoverTime = 1.0f;
        UPROPERTY(EditAnywhere, Category = "Floor Director")
        float RamCooldown = 6.0f;
        UPROPERTY(EditAnywhere, Category = "Floor Director Rally")
        float RallyRadius = 500.0f;
        UPROPERTY(EditAnywhere, Category = "Floor Director Rally")
        float RallyDefenseBonus = 0.25f;
        UPROPERTY(EditAnywhere, Category = "Floor Director Rally")
        float RallyDamageBonus = 0.1f;
        UPROPERTY(EditAnywhere, Category = "Floor Director Rally")
        float RallyInterval = 0.25f;
        UPROPERTY(EditAnywhere, Category = "Floor Director Rally")
        float RallyDuration = 0.5f;
protected:
        void TickChasing(float DeltaTime);
        void TickWinding(float DeltaTime);
        void TickCharging(float DeltaTime);
        void TickRecovering(float DeltaTime);
        void StartCharge(FVector Direction);
        void EndCharge();
        void MoveToward(FVector Direction);
        bool CanBeRammed(AUnshackledCharacterBase* Candidate);
        AUnshackledCharacterBase* FindRamVictim();
        void HitVictim(AUnshackledCharacterBase* Victim);
        void TickRally(float DeltaTime);
        void ApplyRally(AUnshackledCharacterBase* Ally);
        EDirectorState State = EDirectorState::Chasing;
        float WindupTimeLeft = 0.0f;
        float RecoverTimeLeft = 0.0f;
        float RamCooldownLeft = 0.0f;
        float ChargeDistanceLeft = 0.0f;
        float RallyTimer = 0.0f;
        FVector ChargeDirection = FVector(1.0f, 0.0f, 0.0f);
};
