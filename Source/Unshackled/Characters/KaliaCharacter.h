#pragma once
#include "CoreMinimal.h"
#include "PartyMemberCharacter.h"
#include "Stats/StatTypes.h"
#include "KaliaCharacter.generated.h"
UCLASS()
class UNSHACKLED_API AKaliaCharacter : public APartyMemberCharacter
{
        GENERATED_BODY()
public:
        AKaliaCharacter();
        virtual void OnKilledTarget(AUnshackledCharacterBase* Target) override;
        virtual void PerformRetribution() override;
        UPROPERTY(EditAnywhere, Category = "Kalia")
        float DaggerRange = 110.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia")
        float DaggerRadius = 90.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia")
        float DaggerDamageScale = 1.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Passive")
        float ExplosionRadius = 250.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Passive")
        float FreezeDuration = 1.2f;
        UPROPERTY(EditAnywhere, Category = "Kalia Retribution")
        float RetributionDuration = 12.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Retribution")
        float RetributionSpeedBonus = 0.12f;
        UPROPERTY(EditAnywhere, Category = "Kalia Retribution")
        float RetributionDefenseBonus = 0.12f;
        UPROPERTY(EditAnywhere, Category = "Kalia Retribution")
        float RetributionDamageBonus = 0.12f;
        UPROPERTY(EditAnywhere, Category = "Kalia Retribution")
        float RetributionLifesteal = 0.012f;
protected:
        virtual void PerformBasicAttack(FVector Direction) override;
        void FreezeEnemiesAround(FVector Center);
        void FreezeEnemy(AUnshackledCharacterBase* Enemy);
        void AddRetributionModifier(ECharacterStat Stat, EModifierKind Kind, float Amount);
};
