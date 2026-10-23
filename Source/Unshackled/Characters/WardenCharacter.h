#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "WardenCharacter.generated.h"
UCLASS()
class UNSHACKLED_API AWardenCharacter : public AEnemyCharacter
{
        GENERATED_BODY()
public:
        AWardenCharacter();
        virtual void Tick(float DeltaTime) override;
        UPROPERTY(EditAnywhere, Category = "Warden")
        float SlamRange = 100.0f;
        UPROPERTY(EditAnywhere, Category = "Warden")
        float SlamRadius = 90.0f;
        UPROPERTY(EditAnywhere, Category = "Warden")
        float SlamDamageScale = 1.0f;
        UPROPERTY(EditAnywhere, Category = "Warden")
        float SlamRootDuration = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Warden")
        float SlamDefenseLoss = 0.25f;
        UPROPERTY(EditAnywhere, Category = "Warden")
        float SlamDefenseLossDuration = 3.0f;
        UPROPERTY(EditAnywhere, Category = "Warden")
        float ShieldHealthFraction = 0.3f;
        UPROPERTY(EditAnywhere, Category = "Warden")
        float ShieldDefenseBonus = 0.25f;
protected:
        virtual void PerformBasicAttack(FVector Direction) override;
        void ApplySlamEffects(AUnshackledCharacterBase* Target);
        void TickShield();
        bool bShieldRaised = false;
};
