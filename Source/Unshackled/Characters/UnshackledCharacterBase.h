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
        void BasicAttack();                           // attacks the way we are facing (used by enemies)
        void AttackInDirection(FVector Direction);
        void Dash();
        void ReviveCharacter();

        // Runs a hit through defense and the other stats. Returns the health actually lost.
        // bTrueDamage skips defense (used for "% of health" damage).
        float ApplyHit(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker);

        // Builds the raw number from our Damage stat times the ability's scale, then hits the target.
        void DealDamage(AUnshackledCharacterBase* Target, float AbilityScale, bool bTrueDamage);

        // Hooks. Empty by default; characters override them for passives.
        // Overrides must call Super:: first so lifesteal keeps working.
        virtual void OnDealtDamage(AUnshackledCharacterBase* Target, float DamageDealt);
        virtual void OnKilledTarget(AUnshackledCharacterBase* Target);

        // Called whenever something deals damage to this character (old-style ApplyDamage ends up here).
        virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                class AController* EventInstigator, AActor* DamageCauser) override;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UHealthComponent* Health;

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
        UStatsComponent* Stats;

        // Combat settings (changeable in Blueprints)
        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackDamage = 20.0f;

        // how far in front of me the hit sphere sits
        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackRange = 80.0f;

        // size of the hit sphere
        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackRadius = 60.0f;

        UPROPERTY(EditAnywhere, Category = "Combat")
        float AttackCooldown = 0.5f;

        // true on enemies so sides don't hurt themselves (does anyone like friendly fire hello?)
        UPROPERTY(EditAnywhere, Category = "Combat")
        bool bIsEnemy = false;

        // Dash settings
        UPROPERTY(EditAnywhere, Category = "Dash")
        float DashSpeed = 1400.0f;

        UPROPERTY(EditAnywhere, Category = "Dash")
        float DashImmunityTime = 1.0f;

        // Use 5 while testing.
        UPROPERTY(EditAnywhere, Category = "Dash")
        float DashCooldown = 5.0f;

        UPROPERTY(EditAnywhere, Category = "Dash")
        float DashDuration = 0.15f;

protected:
        virtual void BeginPlay() override;
        virtual void Die();

        // Return true to cancel a death (Mal'Kholm's passive). Called when health hits 0.
        virtual bool WouldDie();
        // Copies stat results (speed, max health) onto the engine components.
        void ApplyStatEffects();
        // Steps used by ApplyHit.
        float CalculateFinalDamage(float RawDamage, bool bTrueDamage);
        void CheckForDeath(AUnshackledCharacterBase* Attacker);
        FVector FacingDirection = FVector(0.0f, 1.0f, 0.0f);
        bool bCanAttack = true;
        bool bCanDash = true;
        bool bInvulnerable = false;
        FTimerHandle AttackTimer;
        FTimerHandle DashImmunityTimer;
        FTimerHandle DashCooldownTimer;
        FTimerHandle DashDurationTimer;
        UFUNCTION()
        void ResetAttack();

        UFUNCTION()
        void EndInvulnerability();

        UFUNCTION()
        void ResetDash();

        UFUNCTION()
        void EndDash();
};