#pragma once
#include "CoreMinimal.h"
#include "PartyMemberCharacter.h"
#include "Projectiles/AbilityProjectile.h"
#include "Abilities/TornadoActor.h"
#include "Abilities/VeilActor.h"
#include "SiriusCharacter.generated.h"
UCLASS()
class UNSHACKLED_API ASiriusCharacter : public APartyMemberCharacter
{
        GENERATED_BODY()
public:
        ASiriusCharacter();
        virtual void OnDealtDamage(AUnshackledCharacterBase* Target, float DamageDealt) override;
        virtual void Tick(float DeltaTime) override;
        virtual void OnSwappedOut(APartyMemberCharacter* NewLeader) override;
        virtual void PerformRetribution() override;
        virtual void PerformBattleMove(ATurnBattleManager* Manager, FName MoveId, const FBattleMoveRow* Move, int32 TargetIndex) override;
        virtual void OnBattleRoundEnd(ATurnBattleManager* Manager) override;
        virtual void PerformBattleRetribution(ATurnBattleManager* Manager) override;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        TSubclassOf<AAbilityProjectile> CrescentClass;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        float CrescentSpawnOffset = 60.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        TSubclassOf<ATornadoActor> TornadoClass;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        int32 EmpoweredAttackCount = 3;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        float EmpoweredSizeScale = 2.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        float EmpoweredDamageScale = 1.5f;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        float TornadoSpawnOffset = 150.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        TSubclassOf<AVeilActor> VeilClass;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        float ExecuteThreshold = 0.05f;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        float ExecuteDamage = 100000.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        float SwapVeilReduction = 0.25f;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        float SwapVeilDuration = 3.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius")
        int32 SwapVeilHits = 1;
        UPROPERTY(EditAnywhere, Category = "Sirius Grounded")
        TSubclassOf<AAbilityProjectile> AirBladeClass;
        UPROPERTY(EditAnywhere, Category = "Sirius Grounded")
        float AirBladeInterval = 2.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius Grounded")
        float AirBladeSearchRange = 700.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius Grounded")
        float AirBladeDamageScale = 0.6f;
        UPROPERTY(EditAnywhere, Category = "Sirius Grounded")
        float AirBladeSpeed = 900.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius Grounded")
        float AirBladeRange = 800.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius Grounded")
        float AirBladeRadius = 50.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius Grounded")
        float AirBladeSpawnOffset = 60.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius Grounded")
        float AirBladeAllySlow = 0.1f;
        UPROPERTY(EditAnywhere, Category = "Sirius Grounded")
        float AirBladeAllySlowDuration = 2.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius Retribution")
        float RetributionDuration = 10.0f;
        UPROPERTY(EditAnywhere, Category = "Sirius Retribution")
        float RetributionDefenseReduction = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Sirius Retribution")
        float RetributionSlow = 0.25f;
        UPROPERTY(EditAnywhere, Category = "Sirius Retribution")
        float RetributionLifesteal = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Sirius Retribution")
        float RetributionAuraInterval = 0.25f;
        UPROPERTY(EditAnywhere, Category = "Sirius Retribution")
        float RetributionAuraDuration = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Sirius Battle")
        int32 MarkMaxStacks = 25;
        UPROPERTY(EditAnywhere, Category = "Sirius Battle")
        float MarkDamageReductionPerStack = 0.01f;
        UPROPERTY(EditAnywhere, Category = "Sirius Battle")
        float MarkExecuteThreshold = 0.05f;
        UPROPERTY(EditAnywhere, Category = "Sirius Battle")
        float BattleRetributionHealFraction = 0.25f;
protected:
        virtual void PerformBasicAttack(FVector Direction) override;
        float GetBattleBossHealth(ATurnBattleManager* Manager) const;
        void TryBattleRetributionHeal(ATurnBattleManager* Manager, float BossHealthBefore);
        virtual void PerformSpecial(FVector AimDirection) override;
        virtual void PerformUltimate(FVector AimDirection) override;
        virtual void GroundedTick(float DeltaTime) override;
        virtual float FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker) override;
        bool MoveTouchesEnemy(const FBattleMoveRow* Move) const;
        void AddMarkToBoss(ATurnBattleManager* Manager);
        void TryBattleExecute(ATurnBattleManager* Manager);
        void CastBattleVeil(ATurnBattleManager* Manager, const FBattleMoveRow* Move);
        void CastWindsUnbound(const FBattleMoveRow* Move);
        void CastGaleUnleashed(ATurnBattleManager* Manager, const FBattleMoveRow* Move);
        void CastWindWall(const FBattleMoveRow* Move);
        void CastWindsOfWar(ATurnBattleManager* Manager, const FBattleMoveRow* Move);
        void CastSubmission(ATurnBattleManager* Manager, const FBattleMoveRow* Move);
        void TickBattleTornado(ATurnBattleManager* Manager);
        void FireCrescent(FVector Direction, float DamageScale, float SizeScale);
        void SpawnTornado(FVector Direction);
        void SpawnVeil();
        void DestroyOldVeils();
        AVeilActor* SpawnVeilOn(AUnshackledCharacterBase* VeilOwner);
        void SpawnVeilsOnGroundedAllies();
        void TryExecute(AUnshackledCharacterBase* Target);
        void GiveSwapVeil(APartyMemberCharacter* Ally);
        AUnshackledCharacterBase* FindNearestEnemy();
        void FireAirBlade(AUnshackledCharacterBase* Target);
        void TickRetribution(float DeltaTime);
        void ApplyRetributionToEnemies();
        int32 EmpoweredAttacksLeft = 0;
        float AirBladeTimeLeft = 0.0f;
        float RetributionTimeLeft = 0.0f;
        float RetributionAuraTimer = 0.0f;
        float BattleTornadoDamage = 0.0f;
        int32 BattleTornadoRoundsLeft = 0;
        bool bWindWallUp = false;
        int32 WindWallUses = 0;
        bool bBattleRetributionHealReady = false;
        float WindWallManaFraction = 0.0f;
        UPROPERTY()
        AVeilActor* ActiveVeil;
        UPROPERTY()
        TArray<AVeilActor*> AllyVeils;
};
