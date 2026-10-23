#pragma once
#include "CoreMinimal.h"
#include "PartyMemberCharacter.h"
#include "FlowerCharacter.h"
#include "Abilities/FlowerBombActor.h"
#include "Abilities/SunTreeActor.h"
#include "HanuboyCharacter.generated.h"
enum class EHanuboyGroundedState : uint8
{
        Idle,
        Out,
        Back
};
UCLASS()
class UNSHACKLED_API AHanuboyCharacter : public APartyMemberCharacter
{
        GENERATED_BODY()
public:
        AHanuboyCharacter();
        virtual void Tick(float DeltaTime) override;
        UPROPERTY(EditAnywhere, Category = "Hanuboy")
        float BallLength = 220.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy")
        float BallHalfWidth = 90.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy")
        float BallDamageScale = 1.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy")
        float BallKnockbackDistance = 120.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Flower")
        TSubclassOf<AFlowerCharacter> FlowerClass;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Flower")
        float FlowerSpawnOffset = 120.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Passive")
        TSubclassOf<AFlowerBombActor> BombClass;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Passive")
        float BombInterval = 8.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Passive")
        float BombThrowDistance = 250.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Passive")
        float BombSearchRange = 800.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Passive")
        int32 MaxActiveBombs = 3;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Ultimate")
        TSubclassOf<ASunTreeActor> TreeClass;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Ultimate")
        float TreeSpawnOffset = 150.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedBaseRadius = 350.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedRadiusPerAlly = 100.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedAllyRange = 500.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedInterval = 2.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedDashSpeed = 1800.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedStopDistance = 70.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedBashScale = 0.7f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedRootDuration = 2.0f;
protected:
        virtual void PerformBasicAttack(FVector Direction) override;
        virtual void PerformSpecial(FVector AimDirection) override;
        virtual void PerformUltimate(FVector AimDirection) override;
        void SpawnTree(FVector Direction);
        bool IsInBallLane(AUnshackledCharacterBase* Target, FVector Direction) const;
        void KnockBack(AUnshackledCharacterBase* Target);
        void SpawnFlower(FVector Direction);
        void TickBombs(float DeltaTime);
        AUnshackledCharacterBase* FindNearestEnemyInRange();
        int32 CountActiveBombs();
        void ThrowBomb(AUnshackledCharacterBase* Target);
        float BombTimeLeft = 3.0f;
        virtual void GroundedTick(float DeltaTime) override;
        void ResetGroundedState();
        void TickGroundedIdle(float DeltaTime);
        void TickGroundedOut(float DeltaTime);
        void TickGroundedBack(float DeltaTime);
        float GetGroundedRadius();
        int32 CountNearbyAllies();
        AUnshackledCharacterBase* FindGroundedTarget(float SearchRadius);
        bool MoveTowardPoint(FVector Destination, float StopDistance, float DeltaTime);
        void BashTarget(AUnshackledCharacterBase* Target);
        EHanuboyGroundedState GroundedState = EHanuboyGroundedState::Idle;
        bool bHasGroundedHome = false;
        float GroundedWaitLeft = 0.0f;
        FVector GroundedHome = FVector::ZeroVector;
        UPROPERTY()
        AUnshackledCharacterBase* GroundedTarget = nullptr;
};
