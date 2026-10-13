#pragma once
#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "UnshackledCharacterBase.generated.h"

UCLASS()
class UNSHACKLED_API AUnshackledCharacterBase : public APaperZDCharacter
{
        GENERATED_BODY()

public:
        AUnshackledCharacterBase();

        virtual void Tick(float DeltaTime) override;

        void MoveCharacter(FVector2D Direction);
        void BasicAttack();
        void AttackInDirection(FVector Direction);
        void Dash();
        void ReviveCharacter();

        void UseSpecial(FVector AimDirection);
        void UseUltimate(FVector AimDirection);

        FVector GetFacingDirection() const;

        float ApplyHit(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker);
        void DealDamage(AUnshackledCharacterBase* Target, float AbilityScale, bool bTrueDamage);

        virtual void OnDealtDamage(AUnshackledCharacterBase* Target, float DamageDealt);
        virtual void OnKilledTarget(AUnshackledCharacterBase* Target);

        virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                class AController* EventInstigator, AActor* DamageCauser) override;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UHealthComponent* Health;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UStatsComponent* Stats;

        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackDamage = 20.0f;

        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackRange = 80.0f;

        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackRadius = 60.0f;

        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackCooldown = 0.5f;

        UPROPERTY(EditAnywhere, Category = "Combat")
        bool bIsEnemy = false;

        UPROPERTY(EditAnywhere, Category = "Dash")
        float DashSpeed = 1400.0f;

        UPROPERTY(EditAnywhere, Category = "Dash")
        float DashImmunityTime = 1.0f;

        UPROPERTY(EditAnywhere, Category = "Dash")
        float DashCooldown = 5.0f;

        UPROPERTY(EditAnywhere, Category = "Dash")
        float DashDuration = 0.15f;

        UPROPERTY(EditAnywhere, Category = "Special")
        float SpecialManaCost = 20.0f;

        UPROPERTY(EditAnywhere, Category = "Special")
        float SpecialCooldown = 6.0f;

        UPROPERTY(EditAnywhere, Category = "Special")
        float SpecialWindup = 0.0f;

        UPROPERTY(EditAnywhere, Category = "Ultimate")
        float UltimateManaCost = 60.0f;

        UPROPERTY(EditAnywhere, Category = "Ultimate")
        float UltimateCooldown = 30.0f;

        UPROPERTY(EditAnywhere, Category = "Ultimate")
        float UltimateWindup = 0.0f;

        UPROPERTY(EditAnywhere, Category = "Mana")
        float ManaRegenPerSecond = 3.0f;

protected:
        virtual void BeginPlay() override;
        virtual void Die();
        virtual bool WouldDie();

        virtual void PerformBasicAttack(FVector Direction);
        virtual void PerformSpecial(FVector AimDirection);
        virtual void PerformUltimate(FVector AimDirection);

        void ApplyStatEffects();
        void RegenerateMana(float DeltaTime);

        float CalculateFinalDamage(float RawDamage, bool bTrueDamage);
        void CheckForDeath(AUnshackledCharacterBase* Attacker);

        bool TryPayMana(float ManaCost);
        FVector CleanAimDirection(FVector AimDirection);

        FVector FacingDirection = FVector(0.0f, 1.0f, 0.0f);

        bool bCanAttack = true;
        bool bCanDash = true;
        bool bInvulnerable = false;
        bool bCanUseSpecial = true;
        bool bCanUseUltimate = true;

        FVector PendingSpecialAim = FVector(0.0f, 1.0f, 0.0f);
        FVector PendingUltimateAim = FVector(0.0f, 1.0f, 0.0f);

        FTimerHandle AttackTimer;
        FTimerHandle DashImmunityTimer;
        FTimerHandle DashCooldownTimer;
        FTimerHandle DashDurationTimer;
        FTimerHandle SpecialWindupTimer;
        FTimerHandle SpecialCooldownTimer;
        FTimerHandle UltimateWindupTimer;
        FTimerHandle UltimateCooldownTimer;

        UFUNCTION()
        void ResetAttack();

        UFUNCTION()
        void EndInvulnerability();

        UFUNCTION()
        void ResetDash();

        UFUNCTION()
        void EndDash();

        UFUNCTION()
        void FinishSpecialWindup();

        UFUNCTION()
        void ResetSpecial();

        UFUNCTION()
        void FinishUltimateWindup();

        UFUNCTION()
        void ResetUltimate();
};

// The default PerformBasicAttack is the melee sphere (used by enemies).
// UseSpecial and UseUltimate are called by the controller with the aim direction.
// PerformSpecial and PerformUltimate are overridden per character. They run after all checks pass.
// OnDealtDamage, OnKilledTarget and WouldDie are passive hooks. Overrides call Super:: first.
// Windup values are seconds between key press and the ability firing (0 means instant).
// ManaRegenPerSecond only applies to characters with bHasMana.
// bTrueDamage skips defense (use it for "% of health" damage).