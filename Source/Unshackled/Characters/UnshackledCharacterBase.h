#pragma once
#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Components/HealthComponent.h"
#include "UnshackledCharacterBase.generated.h"

UCLASS()
class UNSHACKLED_API AUnshackledCharacterBase : public APaperZDCharacter
{
      GENERATED_BODY()

public:
      AUnshackledCharacterBase();

      void MoveCharacter(FVector2D Direction);
      void BasicAttack();
      void Dash();
      void ReviveCharacter();

      //  called whenever something deals damage to this character.
      virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
              class AController* EventInstigator, AActor* DamageCauser) override;

      UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
      UHealthComponent* Health;

      // Combat settings (changeable in Blueprints)
      UPROPERTY(EditAnywhere, Category = "Combat")
      float AttackDamage = 20.0f;

      UPROPERTY(EditAnywhere, Category = "Combat")
      float AttackRange = 80.0f;      
      // how far in front of me the hit sphere sits

      UPROPERTY(EditAnywhere, Category = "Combat")
      float AttackRadius = 60.0f;     
      // size of the hit sphere

      UPROPERTY(EditAnywhere, Category = "Combat")
      float AttackCooldown = 0.5f;

      UPROPERTY(EditAnywhere, Category = "Combat")
      bool bIsEnemy = false;          // true on enemies so sides don't hurt themselves (does anyone like friendly fire hello?)

      // Dash settings
      UPROPERTY(EditAnywhere, Category = "Dash")
      float DashSpeed = 1400.0f;

      UPROPERTY(EditAnywhere, Category = "Dash")
      float DashImmunityTime = 1.0f;

      UPROPERTY(EditAnywhere, Category = "Dash")
      float DashCooldown = 5.0f;      // Use 5 while testing.
      
      UPROPERTY(EditAnywhere, Category = "Dash")
      float DashDuration = 0.15f;

protected:
      virtual void Die();

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