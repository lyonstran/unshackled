#include "WaveSpawner.h"
#include "WaveRow.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Characters/UnshackledCharacterBase.h"
#include "Game/UnshackledGameInstance.h"
#include "UI/UnshackledHUD.h"
#include "GameFramework/PlayerController.h"
AWaveSpawner::AWaveSpawner()
{
      PrimaryActorTick.bCanEverTick = true;
      StartTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("StartTrigger"));
      RootComponent = StartTrigger;
      StartTrigger->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));
      StartTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
      StartTrigger->SetGenerateOverlapEvents(true);
}
void AWaveSpawner::BeginPlay()
{
      Super::BeginPlay();
      if (!bUseGameInstanceFloor)
      {
              return;
      }
      UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
      if (GameInstanceRef == nullptr)
      {
              return;
      }
      FloorNumber = GameInstanceRef->CurrentFloor;
}
bool AWaveSpawner::IsFloorCleared() const
{
      return State == EWaveState::FloorCleared;
}
void AWaveSpawner::Tick(float DeltaTime)
{
      Super::Tick(DeltaTime);
      GEngine->AddOnScreenDebugMessage(7002, 0.0f, FColor::Cyan, FString::Printf(TEXT("Floor %d"), FloorNumber)); // DEBUG_FLOOR_LINE
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
      if (StartTrigger->IsOverlappingActor(PlayerPawn))
      {
              if (bUseGameInstanceFloor)
              {
                      UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
                      if (GameInstanceRef != nullptr)
                      {
                              RecruitedPartySize = GameInstanceRef->GetRecruitedCount();
                      }
              }
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
      SpawnTimer = SpawnTimer - DeltaTime;
      bool HasPending = PendingEnemies.Num() > 0;
      bool HasRoom = AliveEnemies.Num() < MaxConcurrentEnemies;
      if (HasPending && HasRoom && SpawnTimer <= 0.0f)
      {
              SpawnOneEnemy();
              SpawnTimer = SpawnInterval;
      }
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
      if (!Row.bScales)
      {
              return Row.BaseCount;
      }
      float FloorMultiplier = FMath::Pow(GrowthRate, (float)(FloorNumber - 1));
      int32 ExpectedPartySize = GetExpectedPartySize();
      float PartyMultiplier = 1.0f + PartyBonus * (float)(RecruitedPartySize - ExpectedPartySize);
      if (PartyMultiplier < 0.5f)
      {
              PartyMultiplier = 0.5f;
      }
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
      TSubclassOf<AEnemyCharacter> EnemyClass = PendingEnemies[0];
      PendingEnemies.RemoveAt(0);
      FVector SpawnLocation = GetSpawnLocation();
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
      APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
      if (PlayerController != nullptr)
      {
              AUnshackledHUD* Hud = Cast<AUnshackledHUD>(PlayerController->GetHUD());
              if (Hud != nullptr)
              {
                      Hud->ShowBanner(TEXT("Floor cleared! Find the exit"), FLinearColor(0.4f, 1.0f, 0.5f, 1.0f), 5.0f);
              }
      }
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