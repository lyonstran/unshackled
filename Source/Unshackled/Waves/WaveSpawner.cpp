#include "WaveSpawner.h"
#include "WaveRow.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Characters/UnshackledCharacterBase.h"

AWaveSpawner::AWaveSpawner()
{
      PrimaryActorTick.bCanEverTick = true;
      StartTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("StartTrigger"));
      RootComponent = StartTrigger;
      StartTrigger->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));
      StartTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
      StartTrigger->SetGenerateOverlapEvents(true);
}

void AWaveSpawner::Tick(float DeltaTime)
{
      Super::Tick(DeltaTime);
      // Each frame, do only the job for the current state.
      if (State == EWaveState::WaitingForPlayer)
      {
              CheckForPlayer();
      }
      else if (State == EWaveState::SpawningWave)
      {
              UpdateSpawning(DeltaTime);
      }
      else if (State == EWaveState::PauseBetweenWaves)
      {
              UpdatePause(DeltaTime);
      }
}

void AWaveSpawner::CheckForPlayer()
{
      APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
      if (PlayerPawn == nullptr)
      {
              return;
      }

      // Asking the box directly avoids needing an overlap event function.
      if (StartTrigger->IsOverlappingActor(PlayerPawn))
      {
              StartNextWave();
      }
}

void AWaveSpawner::StartNextWave()
{
      CurrentWave = CurrentWave + 1;
      BuildWave(CurrentWave);
      SpawnTimer = 0.0f;
      State = EWaveState::SpawningWave;

      FString Message = FString::Printf(TEXT("Wave %d: %d enemies"), CurrentWave, PendingEnemies.Num());
      GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, Message);
}

void AWaveSpawner::UpdateSpawning(float DeltaTime)
{
      RemoveDeadEnemies();
      // Spawn one enemy at a time, but only if there is room and the delay has passed.
      SpawnTimer = SpawnTimer - DeltaTime;
      bool HasPending = PendingEnemies.Num() > 0;
      bool HasRoom = AliveEnemies.Num() < MaxConcurrentEnemies;
      if (HasPending && HasRoom && SpawnTimer <= 0.0f)
      {
              SpawnOneEnemy();
              SpawnTimer = SpawnInterval;
      }
      // The wave is over when nobody is waiting to spawn and nobody is alive.
      bool NothingLeftToSpawn = PendingEnemies.Num() == 0;
      bool NobodyAlive = AliveEnemies.Num() == 0;
      if (NothingLeftToSpawn && NobodyAlive)
      {
              if (WaveExists(CurrentWave + 1))
              {
                      PauseTimer = WavePauseSeconds;
                      State = EWaveState::PauseBetweenWaves;
              }
              else
              {
                      FinishFloor();
              }
      }
}

void AWaveSpawner::UpdatePause(float DeltaTime)
{
      PauseTimer = PauseTimer - DeltaTime;
      if (PauseTimer <= 0.0f)
      {
              StartNextWave();
      }
}

void AWaveSpawner::BuildWave(int32 WaveNumber)
{
      // Start with an empty line, then add every enemy this wave needs.
      PendingEnemies.Empty();

      if (WaveTable == nullptr)
      {
              return;
      }

      TArray<FName> RowNames = WaveTable->GetRowNames();
      for (int32 i = 0; i < RowNames.Num(); i++)
      {
              FWaveRow* Row = WaveTable->FindRow<FWaveRow>(RowNames[i], TEXT("BuildWave"));
              if (Row == nullptr)
              {
                      continue;
              }
              if (Row->Floor != FloorNumber)
              {
                      continue;
              }
              if (Row->Wave != WaveNumber)
              {
                      continue;
              }
              if (Row->EnemyClass == nullptr)
              {
                      continue;
              }

              int32 Count = CalculateCount(*Row);
              for (int32 n = 0; n < Count; n++)
              {
                      PendingEnemies.Add(Row->EnemyClass);
              }
      }
}

bool AWaveSpawner::WaveExists(int32 WaveNumber)
{
      if (WaveTable == nullptr)
      {
              return false;
      }

      TArray<FName> RowNames = WaveTable->GetRowNames();
      for (int32 i = 0; i < RowNames.Num(); i++)
      {
              FWaveRow* Row = WaveTable->FindRow<FWaveRow>(RowNames[i], TEXT("WaveExists"));
              if (Row == nullptr)
              {
                      continue;
              }
              if (Row->Floor == FloorNumber && Row->Wave == WaveNumber)
              {
                      return true;
              }
      }
      return false;
}

int32 AWaveSpawner::CalculateCount(const FWaveRow& Row)
{
      // Minibosses and bosses are always exactly what the table says.
      if (!Row.bScales)
      {
              return Row.BaseCount;
      }

      // Floor 1 gives 1.0, floor 2 gives GrowthRate, floor 3 gives GrowthRate squared, and so on.
      float FloorMultiplier = FMath::Pow(GrowthRate, (float)(FloorNumber - 1));

      // 1.0 when the party is the size the floor expects. Bigger party = more enemies.
      int32 ExpectedPartySize = GetExpectedPartySize();
      float PartyMultiplier = 1.0f + PartyBonus * (float)(RecruitedPartySize - ExpectedPartySize);
      if (PartyMultiplier < 0.5f)
      {
              PartyMultiplier = 0.5f;
      }

      // Round up, but subtract a tiny amount first so 4.0000001 doesn't become 5.
      float ScaledCount = (float)Row.BaseCount * FloorMultiplier * PartyMultiplier;
      int32 FinalCount = FMath::CeilToInt(ScaledCount - 0.001f);
      if (FinalCount < 1)
      {
              FinalCount = 1;
      }
      return FinalCount;
}

int32 AWaveSpawner::GetExpectedPartySize()
{
      // If the table or row is missing, assume the party is exactly as expected (no party scaling).
      if (FloorTable == nullptr)
      {
              return RecruitedPartySize;
      }

      FName RowName = FName(*FString::FromInt(FloorNumber));
      FFloorRow* Row = FloorTable->FindRow<FFloorRow>(RowName, TEXT("GetExpectedPartySize"));
      if (Row == nullptr)
      {
              return RecruitedPartySize;
      }
      return Row->ExpectedPartySize;
}

void AWaveSpawner::SpawnOneEnemy()
{
      // Take the first enemy in line.
      TSubclassOf<AEnemyCharacter> EnemyClass = PendingEnemies[0];
      PendingEnemies.RemoveAt(0);
      FVector SpawnLocation = GetSpawnLocation();
      // Spawn even if something is in the way; the engine nudges it if it can.
      FActorSpawnParameters SpawnParams;
      SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

      AEnemyCharacter* NewEnemy = GetWorld()->SpawnActor<AEnemyCharacter>(EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
      if (NewEnemy != nullptr)
      {
              AliveEnemies.Add(NewEnemy);
      }
}

FVector AWaveSpawner::GetSpawnLocation()
{
      // No spawn points set: spawn somewhere near the spawner so testing still works.
      if (SpawnPoints.Num() == 0)
      {
              float OffsetX = FMath::RandRange(-300.0f, 300.0f);
              float OffsetY = FMath::RandRange(-300.0f, 300.0f);
              return GetActorLocation() + FVector(OffsetX, OffsetY, 0.0f);
      }

      int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
      AActor* ChosenPoint = SpawnPoints[RandomIndex];
      if (ChosenPoint == nullptr)
      {
              return GetActorLocation();
      }
      return ChosenPoint->GetActorLocation();
}

void AWaveSpawner::RemoveDeadEnemies()
{
      for (int32 i = AliveEnemies.Num() - 1; i >= 0; i--)
      {
              AEnemyCharacter* Enemy = AliveEnemies[i];
              if (!IsValid(Enemy))
              {
                      AliveEnemies.RemoveAt(i);
              }
              else if (Enemy->Health->IsDead())
              {
                      AliveEnemies.RemoveAt(i);
              }
      }
}

void AWaveSpawner::FinishFloor()
{
      State = EWaveState::FloorCleared;
      ReviveDeadAllies();

      GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Green, TEXT("Floor cleared!"));
      UE_LOG(LogTemp, Warning, TEXT("Floor %d cleared"), FloorNumber);
}

void AWaveSpawner::ReviveDeadAllies()
{
      TArray<AActor*> AllCharacters;
      UGameplayStatics::GetAllActorsOfClass(this, AUnshackledCharacterBase::StaticClass(), AllCharacters);

      for (int32 i = 0; i < AllCharacters.Num(); i++)
      {
              AUnshackledCharacterBase* Character = Cast<AUnshackledCharacterBase>(AllCharacters[i]);
              if (Character == nullptr)
              {
                      continue;
              }
              if (Character->bIsEnemy)
              {
                      continue;
              }
              Character->ReviveCharacter();
      }
}