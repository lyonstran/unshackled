#pragma once
#include "CoreMinimal.h"
#include "PartyMemberCharacter.h"
#include "MalkholmCharacter.generated.h"
UCLASS()
class UNSHACKLED_API AMalkholmCharacter : public APartyMemberCharacter
{
        GENERATED_BODY()
public:
        AMalkholmCharacter();
        UPROPERTY(EditAnywhere, Category = "Malkholm")
        float ChainRadius = 160.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm")
        float EdgeStartFraction = 0.6f;
        UPROPERTY(EditAnywhere, Category = "Malkholm")
        float SwingDamageScale = 1.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm")
        float DefenseGainPerStack = 0.02f;
        UPROPERTY(EditAnywhere, Category = "Malkholm")
        int32 MaxDefenseStacks = 5;
        UPROPERTY(EditAnywhere, Category = "Malkholm")
        float DefenseGainDuration = 7.5f;
        UPROPERTY(EditAnywhere, Category = "Malkholm")
        float DamageLossPerStack = 0.02f;
        UPROPERTY(EditAnywhere, Category = "Malkholm")
        int32 MaxDamageLossStacks = 5;
        UPROPERTY(EditAnywhere, Category = "Malkholm")
        float DamageLossDuration = 5.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm")
        float EdgeSlowFraction = 0.15f;
        UPROPERTY(EditAnywhere, Category = "Malkholm")
        float EdgeSlowDuration = 1.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Slam")
        float SlamRange = 200.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Slam")
        float SlamRadius = 150.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Slam")
        float SlamTipDistance = 250.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Slam")
        float SlamDamageScale = 1.5f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Slam")
        float SlamSlowFraction = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Slam")
        float SlamSlowDuration = 1.5f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Slam")
        float SlamRootDuration = 2.0f;
protected:
        virtual void PerformBasicAttack(FVector Direction) override;
        virtual void PerformSpecial(FVector AimDirection) override;
        bool IsBonusSwing() const;
        bool IsOnEdge(AUnshackledCharacterBase* Target) const;
        bool IsAtSlamTip(AUnshackledCharacterBase* Target) const;
        void ApplyEdgeDebuffs(AUnshackledCharacterBase* Target);
        void ApplySlamEffects(AUnshackledCharacterBase* Target);
        void GainDefenseStack();
        void ShowSwingText();
        int32 SwingCount = 0;
        bool bSwingClockwise = true;
};
