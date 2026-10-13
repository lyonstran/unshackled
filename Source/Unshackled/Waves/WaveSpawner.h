#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Characters/EnemyCharacter.h"
#include "WaveRow.h"
#include "WaveSpawner.generated.h"
class UBoxComponent;
enum class EWaveState : uint8
{
      WaitingForPlayer,
      SpawningWave,
      PauseBetweenWaves,
      FloorCleared
};
UCLASS()
class UNSHACKLED_API AWaveSpawner : public AActor
{
      GENERATED_BODY()
public:
      AWaveSpawner();
      virtual void Tick(float DeltaTime) override;
      UPROPERTY(VisibleAnywhere, Category = "Components")
      UBoxComponent* StartTrigger;
      UPROPERTY(EditAnywhere, Category = "Waves")
      UDataTable* WaveTable;
      UPROPERTY(EditAnywhere, Category = "Waves")
      UDataTable* FloorTable;
      UPROPERTY(EditAnywhere, Category = "Waves")
      int32 FloorNumber = 1;
      UPROPERTY(EditAnywhere, Category = "Waves")
      int32 RecruitedPartySize = 2;
      UPROPERTY(EditAnywhere, Category = "Scaling")
      float GrowthRate = 1.2f;
      UPROPERTY(EditAnywhere, Category = "Scaling")
      float PartyBonus = 0.1f;
      UPROPERTY(EditAnywhere, Category = "Scaling")
      int32 MaxConcurrentEnemies = 8;
      UPROPERTY(EditAnywhere, Category = "Timing")
      float WavePauseSeconds = 2.5f;
      UPROPERTY(EditAnywhere, Category = "Timing")
      float SpawnInterval = 0.4f;
      UPROPERTY(EditAnywhere, Category = "Waves")
      TArray<AActor*> SpawnPoints;
protected:
      EWaveState State = EWaveState::WaitingForPlayer;
      int32 CurrentWave = 0;
      float PauseTimer = 0.0f;
      float SpawnTimer = 0.0f;
      UPROPERTY()
      TArray<TSubclassOf<AEnemyCharacter>> PendingEnemies;
      UPROPERTY()
      TArray<AEnemyCharacter*> AliveEnemies;
      void CheckForPlayer();
      void StartNextWave();
      void UpdateSpawning(float DeltaTime);
      void UpdatePause(float DeltaTime);
      void BuildWave(int32 WaveNumber);
      bool WaveExists(int32 WaveNumber);
      int32 CalculateCount(const FWaveRow& Row);
      int32 GetExpectedPartySize();
      void SpawnOneEnemy();
      FVector GetSpawnLocation();
      void RemoveDeadEnemies();
      void FinishFloor();
      void ReviveDeadAllies();
};