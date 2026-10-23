#pragma once
#include "CoreMinimal.h"
#include "PartyMemberCharacter.h"
#include "FlowerCharacter.h"
#include "HanuboyCharacter.generated.h"
UCLASS()
class UNSHACKLED_API AHanuboyCharacter : public APartyMemberCharacter
{
        GENERATED_BODY()
public:
        AHanuboyCharacter();
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
protected:
        virtual void PerformBasicAttack(FVector Direction) override;
        virtual void PerformSpecial(FVector AimDirection) override;
        bool IsInBallLane(AUnshackledCharacterBase* Target, FVector Direction) const;
        void KnockBack(AUnshackledCharacterBase* Target);
        void SpawnFlower(FVector Direction);
};
