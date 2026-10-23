#pragma once
#include "CoreMinimal.h"
#include "PartyMemberCharacter.h"
#include "Stats/StatTypes.h"
#include "KaliaCharacter.generated.h"
enum class EKaliaGroundedState : uint8
{
        Idle,
        Out,
        Back
};
UCLASS()
class UNSHACKLED_API AKaliaCharacter : public APartyMemberCharacter
{
        GENERATED_BODY()
public:
        AKaliaCharacter();
        virtual void Tick(float DeltaTime) override;
        virtual void OnDealtDamage(AUnshackledCharacterBase* Target, float DamageDealt) override;
        virtual void OnKilledTarget(AUnshackledCharacterBase* Target) override;
        virtual void PerformRetribution() override;
        UPROPERTY(EditAnywhere, Category = "Kalia")
        float DaggerRange = 110.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia")
        float DaggerRadius = 90.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia")
        float DaggerDamageScale = 1.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Knife")
        float KnifeRange = 700.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Knife")
        float KnifeWidth = 60.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Knife")
        float KnifeDamageScale = 2.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Knife")
        float KnifeDefenseReduction = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Kalia Knife")
        float KnifeDefenseDuration = 5.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Knife")
        float BlinkBehindDistance = 90.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Knife")
        float RefreshHealthFraction = 0.1f;
        UPROPERTY(EditAnywhere, Category = "Kalia Passive")
        float ExplosionRadius = 250.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Passive")
        float FreezeDuration = 1.2f;
        UPROPERTY(EditAnywhere, Category = "Kalia Grounded")
        float GroundedRadius = 500.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Grounded")
        float GroundedInterval = 1.5f;
        UPROPERTY(EditAnywhere, Category = "Kalia Grounded")
        float GroundedDashSpeed = 2200.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Grounded")
        float GroundedStopDistance = 70.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Grounded")
        float GroundedSlashScale = 0.8f;
        UPROPERTY(EditAnywhere, Category = "Kalia Ultimate")
        float UltimateDuration = 12.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Ultimate")
        float UltimateTimeDilation = 0.01f;
        UPROPERTY(EditAnywhere, Category = "Kalia Ultimate")
        float UltimateDrainFraction = 0.12f;
        UPROPERTY(EditAnywhere, Category = "Kalia Ultimate")
        float UltimateHealthFloorFraction = 0.012f;
        UPROPERTY(EditAnywhere, Category = "Kalia Ultimate")
        float UltimateHealFraction = 0.12f;
        UPROPERTY(EditAnywhere, Category = "Kalia Ultimate")
        float UltimateMissingDamageFraction = 0.12f;
        UPROPERTY(EditAnywhere, Category = "Kalia Ultimate")
        float UltimateBlinkOffset = 90.0f;
        UPROPERTY(EditAnywhere, Category = "Kalia Ultimate")
        float UltimateSlashScale = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Kalia Ultimate")
        float UltimateCastDamageFraction = 0.012f;
        UPROPERTY(EditAnywhere, Category = "Kalia Ultimate")
        float EchoFraction = 0.12f;
        UPROPERTY(EditAnywhere, Category = "Kalia Ultimate")
        int32 EchoRepeats = 2;
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
        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
        virtual void PerformBasicAttack(FVector Direction) override;
        virtual void PerformUltimate(FVector AimDirection) override;
        virtual void PerformSpecial(FVector AimDirection) override;
        AUnshackledCharacterBase* FindKnifeTarget(FVector Direction);
        void ApplyKnifeDebuff(AUnshackledCharacterBase* Target);
        void RefreshSpecialIfLow(AUnshackledCharacterBase* Target);
        void BlinkBehind(AUnshackledCharacterBase* Target, FVector Direction);
        void FreezeEnemiesAround(FVector Center);
        void FreezeEnemy(AUnshackledCharacterBase* Enemy);
        void AddRetributionModifier(ECharacterStat Stat, EModifierKind Kind, float Amount);
        virtual void GroundedTick(float DeltaTime) override;
        void ResetGroundedState();
        void TickGroundedIdle(float DeltaTime);
        void TickGroundedOut(float DeltaTime);
        void TickGroundedBack(float DeltaTime);
        AUnshackledCharacterBase* FindGroundedTarget();
        bool MoveTowardPoint(FVector Destination, float StopDistance, float DeltaTime);
        void StartUltimate();
        void DamageEnemiesOnCast();
        void EndUltimate();
        void TickUltimate(float DeltaTime);
        void KeepAbilitiesReady();
        void DrainUltimateHealth(float DeltaTime);
        void RestoreTimeDilation();
        void BlinkToRandomTarget();
        void ApplyBlinkEffects(AUnshackledCharacterBase* Target);
        void ApplyEcho();
        int32 FindEchoIndex(AUnshackledCharacterBase* Target);
        void AddEchoDamage(AUnshackledCharacterBase* Target, float Amount);
        void AddEchoHealing(AUnshackledCharacterBase* Target, float Amount);
        bool bUltimateActive = false;
        bool bDilationSynced = false;
        float UltimateTimeLeft = 0.0f;
        float SavedSpecialManaCost = 0.0f;
        float SavedSpecialWindup = 0.0f;
        float SavedUltimateManaCost = 0.0f;
        bool bSavedCanDash = true;
        UPROPERTY()
        TArray<AUnshackledCharacterBase*> EchoTargets;
        TArray<float> EchoDamage;
        TArray<float> EchoHealing;
        EKaliaGroundedState GroundedState = EKaliaGroundedState::Idle;
        bool bHasGroundedHome = false;
        float GroundedWaitLeft = 0.0f;
        FVector GroundedHome = FVector::ZeroVector;
        UPROPERTY()
        AUnshackledCharacterBase* GroundedTarget = nullptr;
};
