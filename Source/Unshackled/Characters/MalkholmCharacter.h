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
        virtual void Tick(float DeltaTime) override;
        virtual void PerformRetribution() override;
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
        UPROPERTY(EditAnywhere, Category = "Malkholm Retribution")
        float RetributionDuration = 10.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Retribution")
        float RetributionHealthCost = 0.35f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Retribution")
        float RetributionDamageBonus = 0.25f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Retribution")
        float RetributionRangeMultiplier = 2.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Retribution")
        float RetributionMissingHealHeal = 0.25f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Retribution")
        float RetributionMaxHealHeal = 0.2f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Passive")
        float PassiveCooldown = 180.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Passive")
        float PassiveBuffDuration = 15.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Passive")
        float PassiveLifesteal = 0.1f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Passive")
        float PassiveDefenseLoss = 1000.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Grounded")
        float WhipInterval = 4.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Grounded")
        float WhipRadius = 220.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Grounded")
        float WhipDamageScale = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Grounded")
        float WhipKnockbackDistance = 150.0f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Grounded")
        float YankHealthFraction = 0.1f;
        UPROPERTY(EditAnywhere, Category = "Malkholm Grounded")
        float YankOffset = 100.0f;
protected:
        virtual void PerformBasicAttack(FVector Direction) override;
        virtual void PerformSpecial(FVector AimDirection) override;
        virtual bool WouldDie() override;
        virtual void GroundedTick(float DeltaTime) override;
        bool IsBonusSwing() const;
        bool IsOnEdge(AUnshackledCharacterBase* Target) const;
        bool IsAtSlamTip(AUnshackledCharacterBase* Target) const;
        void ApplyEdgeDebuffs(AUnshackledCharacterBase* Target);
        void ApplySlamEffects(AUnshackledCharacterBase* Target);
        void GainDefenseStack();
        void ShowSwingText();
        float GetRangeMultiplier() const;
        void TickRetribution(float DeltaTime);
        void EndRetribution();
        void TickPassiveCooldown(float DeltaTime);
        void GiveCheatDeathBuffs();
        bool TryYankLeader();
        int32 WhipEnemiesInRange();
        void KnockBack(AUnshackledCharacterBase* Target);
        int32 SwingCount = 0;
        bool bSwingClockwise = true;
        float RetributionTimeLeft = 0.0f;
        float PassiveCooldownLeft = 0.0f;
        float WhipTimeLeft = 0.0f;
};
