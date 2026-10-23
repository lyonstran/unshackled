#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "Projectiles/AbilityProjectile.h"
#include "CasterGuardCharacter.generated.h"
UCLASS()
class UNSHACKLED_API ACasterGuardCharacter : public AEnemyCharacter
{
        GENERATED_BODY()
public:
        ACasterGuardCharacter();
        virtual void Tick(float DeltaTime) override;
        UPROPERTY(EditAnywhere, Category = "Caster Guard")
        TSubclassOf<AAbilityProjectile> MissileClass;
        UPROPERTY(EditAnywhere, Category = "Caster Guard")
        float FireRange = 500.0f;
        UPROPERTY(EditAnywhere, Category = "Caster Guard")
        float RetreatDistance = 300.0f;
        UPROPERTY(EditAnywhere, Category = "Caster Guard")
        float MissileDamageScale = 1.0f;
        UPROPERTY(EditAnywhere, Category = "Caster Guard")
        float MissileSpeed = 700.0f;
        UPROPERTY(EditAnywhere, Category = "Caster Guard")
        float MissileRange = 700.0f;
        UPROPERTY(EditAnywhere, Category = "Caster Guard")
        float MissileRadius = 40.0f;
        UPROPERTY(EditAnywhere, Category = "Caster Guard")
        float MissileSpawnOffset = 60.0f;
protected:
        virtual void PerformBasicAttack(FVector Direction) override;
        void MoveAwayFrom(FVector Direction);
        void MoveToward(FVector Direction);
};
