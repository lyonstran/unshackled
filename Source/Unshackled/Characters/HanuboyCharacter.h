#pragma once
#include "CoreMinimal.h"
#include "PartyMemberCharacter.h"
#include "FlowerCharacter.h"
#include "Abilities/FlowerBombActor.h"
#include "HanuboyCharacter.generated.h"
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
protected:
        virtual void PerformBasicAttack(FVector Direction) override;
        virtual void PerformSpecial(FVector AimDirection) override;
        bool IsInBallLane(AUnshackledCharacterBase* Target, FVector Direction) const;
        void KnockBack(AUnshackledCharacterBase* Target);
        void SpawnFlower(FVector Direction);
        void TickBombs(float DeltaTime);
        AUnshackledCharacterBase* FindNearestEnemyInRange();
        int32 CountActiveBombs();
        void ThrowBomb(AUnshackledCharacterBase* Target);
        float BombTimeLeft = 3.0f;
};
