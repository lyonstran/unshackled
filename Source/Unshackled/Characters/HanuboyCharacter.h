#pragma once
#include "CoreMinimal.h"
#include "PartyMemberCharacter.h"
#include "FlowerCharacter.h"
#include "Abilities/FlowerBombActor.h"
#include "Abilities/SunTreeActor.h"
#include "Abilities/PetalActor.h"
#include "HanuboyCharacter.generated.h"
enum class EHanuboySpawnKind : uint8
{
        Flower,
        Mushroom
};
struct FHanuboySpawn
{
        EHanuboySpawnKind Kind = EHanuboySpawnKind::Flower;
        int32 RoundsLeft = 0;
};
struct FHanuboyBomb
{
        bool bOnBoss = false;
        int32 AllyIndex = -1;
};
enum class EHanuboyGroundedState : uint8
{
        Idle,
        Out,
        Back
};
UCLASS()
class UNSHACKLED_API AHanuboyCharacter : public APartyMemberCharacter
{
        GENERATED_BODY()
public:
        AHanuboyCharacter();
        virtual void Tick(float DeltaTime) override;
        virtual void PerformRetribution() override;
        virtual void PerformBattleMove(ATurnBattleManager* Manager, FName MoveId, const FBattleMoveRow* Move, int32 TargetIndex) override;
        virtual void OnBattleRoundEnd(ATurnBattleManager* Manager) override;
        virtual void OnBattlePlayerPhaseStart(ATurnBattleManager* Manager) override;
        virtual bool CanUseBattleMove(FName MoveId) const override;
        virtual FString GetBattleCardNote() const override;
        UPROPERTY(EditAnywhere, Category = "Hanuboy")
        float BallLength = 220.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy")
        float BallHalfWidth = 90.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy")
        float BallDamageScale = 1.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy")
        float BallKnockbackDistance = 120.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Flower")
        TSubclassOf<AFlowerCharacter> FlowerClass;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Flower")
        float FlowerSpawnOffset = 120.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Passive")
        TSubclassOf<AFlowerBombActor> BombClass;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Passive")
        float BombInterval = 8.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Passive")
        float BombThrowDistance = 250.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Passive")
        float BombSearchRange = 800.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Passive")
        int32 MaxActiveBombs = 3;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Ultimate")
        TSubclassOf<ASunTreeActor> TreeClass;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Ultimate")
        float TreeSpawnOffset = 150.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Retribution")
        TSubclassOf<APetalActor> PetalClass;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Retribution")
        int32 PetalCount = 12;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Retribution")
        float PetalRainDuration = 2.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Retribution")
        float PetalScatterRadius = 1200.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedBaseRadius = 350.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedRadiusPerAlly = 100.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedAllyRange = 500.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedInterval = 2.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedDashSpeed = 1800.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedStopDistance = 70.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedBashScale = 0.7f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Grounded")
        float GroundedRootDuration = 2.0f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Battle")
        float BombBossChance = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Battle")
        float BombBossDamageFraction = 0.05f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Battle")
        float BombAllyHealFraction = 0.1f;
        UPROPERTY(EditAnywhere, Category = "Hanuboy Battle")
        float SpawnManaFraction = 0.05f;
protected:
        virtual float FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker) override;
        void CastChainBall(ATurnBattleManager* Manager, const FBattleMoveRow* Move);
        void CastSprout(const FBattleMoveRow* Move);
        void CastTerritory(const FBattleMoveRow* Move);
        void CastBlossom(ATurnBattleManager* Manager, const FBattleMoveRow* Move);
        void CastHarvest(ATurnBattleManager* Manager, const FBattleMoveRow* Move);
        void CastSqueak(ATurnBattleManager* Manager, const FBattleMoveRow* Move, int32 TargetIndex);
        void CastLoudSqueak(ATurnBattleManager* Manager, const FBattleMoveRow* Move);
        void CastShelter(ATurnBattleManager* Manager, const FBattleMoveRow* Move, int32 TargetIndex);
        void AddFragileSpawn(EHanuboySpawnKind Kind, int32 MaxOfKind, int32 Rounds);
        int32 CountSpawnsOfKind(EHanuboySpawnKind Kind) const;
        int32 FindOldestOfKind(EHanuboySpawnKind Kind) const;
        int32 CountAllSpawns() const;
        void TickFragileSpawns(ATurnBattleManager* Manager);
        void HealAllAllies(ATurnBattleManager* Manager, float MaxHealthFraction);
        void GiveManaToAllAllies(ATurnBattleManager* Manager, float ManaAmount);
        void AgeFragileSpawns();
        void TickTree(ATurnBattleManager* Manager);
        void EndTree(ATurnBattleManager* Manager);
        void SetAllyDebuffScale(ATurnBattleManager* Manager, float Scale);
        void GiveSpawnMana();
        void DetonateBombs(ATurnBattleManager* Manager);
        void DetonateBomb(ATurnBattleManager* Manager, FHanuboyBomb Bomb);
        void DropBomb(ATurnBattleManager* Manager);
        int32 PickRandomLivingAlly(ATurnBattleManager* Manager) const;
        void ClearMyShelters(ATurnBattleManager* Manager);
        TArray<FHanuboySpawn> FragileSpawns;
        TArray<FHanuboyBomb> Bombs;
        int32 TreeRoundsLeft = 0;
        float SproutHealFraction = 0.0f;
        float TerritoryManaAmount = 0.0f;
        float TreeTickFraction = 0.0f;
        virtual void PerformBasicAttack(FVector Direction) override;
        virtual void PerformSpecial(FVector AimDirection) override;
        virtual void PerformUltimate(FVector AimDirection) override;
        void SpawnTree(FVector Direction);
        bool IsInBallLane(AUnshackledCharacterBase* Target, FVector Direction) const;
        void KnockBack(AUnshackledCharacterBase* Target);
        void SpawnFlower(FVector Direction);
        void TickBombs(float DeltaTime);
        AUnshackledCharacterBase* FindNearestEnemyInRange();
        int32 CountActiveBombs();
        void ThrowBomb(AUnshackledCharacterBase* Target);
        float BombTimeLeft = 3.0f;
        void TickPetalRain(float DeltaTime);
        void SpawnPetal();
        int32 PetalsLeftToSpawn = 0;
        float PetalSpawnTimer = 0.0f;
        virtual void GroundedTick(float DeltaTime) override;
        void ResetGroundedState();
        void TickGroundedIdle(float DeltaTime);
        void TickGroundedOut(float DeltaTime);
        void TickGroundedBack(float DeltaTime);
        float GetGroundedRadius();
        int32 CountNearbyAllies();
        AUnshackledCharacterBase* FindGroundedTarget(float SearchRadius);
        bool MoveTowardPoint(FVector Destination, float StopDistance, float DeltaTime);
        void BashTarget(AUnshackledCharacterBase* Target);
        EHanuboyGroundedState GroundedState = EHanuboyGroundedState::Idle;
        bool bHasGroundedHome = false;
        float GroundedWaitLeft = 0.0f;
        FVector GroundedHome = FVector::ZeroVector;
        UPROPERTY()
        AUnshackledCharacterBase* GroundedTarget = nullptr;
};
