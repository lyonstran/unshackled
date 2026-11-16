#include "UnshackledGameInstance.h"
#include "Characters/PartyMemberCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "EngineUtils.h"
#include "Game/UnshackledSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
void UUnshackledGameInstance::Init()
{
	Super::Init();
	ResetToFirstFloor();
}
bool UUnshackledGameInstance::IsLastFloor() const
{
	return CurrentFloor >= MaxFloor;
}
bool UUnshackledGameInstance::AdvanceFloor()
{
	if (IsLastFloor())
	{
		return false;
	}
	CurrentFloor = CurrentFloor + 1;
	return true;
}
void UUnshackledGameInstance::ResetToFirstFloor()
{
	CurrentFloor = 1;
	CheckpointLevelName = OverworldLevelName;
	FloorStartStates.Empty();
	RecruitedIds.Empty();
	RecruitedIds.Add(FName("Sirius"));
	FloorStartRecruitedIds = RecruitedIds;
}
bool UUnshackledGameInstance::IsRecruited(FName CharacterId) const
{
	return RecruitedIds.Contains(CharacterId);
}
void UUnshackledGameInstance::Recruit(FName CharacterId)
{
	if (RecruitedIds.Contains(CharacterId))
	{
		return;
	}
	RecruitedIds.Add(CharacterId);
}
int32 UUnshackledGameInstance::GetRecruitedCount() const
{
	return RecruitedIds.Num();
}
void UUnshackledGameInstance::RestoreFloorStartRecruits()
{
	RecruitedIds = FloorStartRecruitedIds;
}
int32 UUnshackledGameInstance::FindStateIndex(FName CharacterId) const
{
	for (int32 i = 0; i < FloorStartStates.Num(); i++)
	{
		if (FloorStartStates[i].CharacterId == CharacterId)
		{
			return i;
		}
	}
	return -1;
}
void UUnshackledGameInstance::SaveFloorStartStates()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	for (TActorIterator<APartyMemberCharacter> It(World); It; ++It)
	{
		APartyMemberCharacter* Member = *It;
		if (Member->StatsRowName.IsNone())
		{
			continue;
		}
		int32 Index = FindStateIndex(Member->StatsRowName);
		if (Index < 0)
		{
			FCharacterState NewState;
			NewState.CharacterId = Member->StatsRowName;
			Index = FloorStartStates.Add(NewState);
		}
		FloorStartStates[Index].HealthFraction = GetHealthFraction(Member);
		FloorStartStates[Index].ManaFraction = GetManaFraction(Member);
	}
}
bool UUnshackledGameInstance::RestoreCharacterState(APartyMemberCharacter* Member)
{
	if (Member == nullptr)
	{
		return false;
	}
	int32 Index = FindStateIndex(Member->StatsRowName);
	if (Index < 0)
	{
		return false;
	}
	float SavedHealthFraction = FloorStartStates[Index].HealthFraction;
	if (SavedHealthFraction <= 0.0f)
	{
		return false;
	}
	if (SavedHealthFraction > 1.0f)
	{
		SavedHealthFraction = 1.0f;
	}
	float SavedManaFraction = FloorStartStates[Index].ManaFraction;
	if (SavedManaFraction < 0.0f)
	{
		SavedManaFraction = 0.0f;
	}
	if (SavedManaFraction > 1.0f)
	{
		SavedManaFraction = 1.0f;
	}
	Member->Health->CurrentHealth = Member->Health->MaxHealth * SavedHealthFraction;
	Member->Stats->CurrentMana = Member->Stats->GetFinal(ECharacterStat::MaxMana) * SavedManaFraction;
	return true;
}
float UUnshackledGameInstance::GetHealthFraction(APartyMemberCharacter* Member) const
{
	if (Member->Health->MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	return Member->Health->CurrentHealth / Member->Health->MaxHealth;
}
float UUnshackledGameInstance::GetManaFraction(APartyMemberCharacter* Member) const
{
	float MaxMana = Member->Stats->GetFinal(ECharacterStat::MaxMana);
	if (MaxMana <= 0.0f)
	{
		return 0.0f;
	}
	return Member->Stats->CurrentMana / MaxMana;
}
int32 UUnshackledGameInstance::GetFloorsCleared() const
{
	int32 Cleared = CurrentFloor - 1;
	if (Cleared < 0)
	{
		Cleared = 0;
	}
	return Cleared;
}
float UUnshackledGameInstance::GetPartyStatScale() const
{
	return 1.0f + (PartyStatGrowthPerFloor * GetFloorsCleared());
}
float UUnshackledGameInstance::GetPartyManaCostScale() const
{
	return 1.0f + (PartyManaCostGrowthPerFloor * GetFloorsCleared());
}
float UUnshackledGameInstance::GetEnemyStatScale() const
{
	return 1.0f + (EnemyStatGrowthPerFloor * GetFloorsCleared());
}
void UUnshackledGameInstance::ReachCheckpoint(FName LevelName)
{
	SaveFloorStartStates();
	CheckpointLevelName = LevelName;
	FloorStartRecruitedIds = RecruitedIds;
	SaveCheckpoint();
}
FString UUnshackledGameInstance::GetCheckpointDescription() const
{
	if (CheckpointLevelName == OverworldLevelName)
	{
		return FString::Printf(TEXT("start of Floor %d"), CurrentFloor);
	}
	return FString::Printf(TEXT("Floor %d boss fight"), CurrentFloor);
}
bool UUnshackledGameInstance::HasSaveGame() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex);
}
bool UUnshackledGameInstance::SaveCheckpoint()
{
	USaveGame* NewSave = UGameplayStatics::CreateSaveGameObject(UUnshackledSaveGame::StaticClass());
	UUnshackledSaveGame* SaveData = Cast<UUnshackledSaveGame>(NewSave);
	if (SaveData == nullptr)
	{
		return false;
	}
	SaveData->Floor = CurrentFloor;
	SaveData->CheckpointLevelName = CheckpointLevelName;
	SaveData->RecruitedIds = FloorStartRecruitedIds;
	for (int32 i = 0; i < FloorStartStates.Num(); i++)
	{
		SaveData->CharacterIds.Add(FloorStartStates[i].CharacterId);
		SaveData->CharacterHealth.Add(FloorStartStates[i].HealthFraction);
		SaveData->CharacterMana.Add(FloorStartStates[i].ManaFraction);
	}
	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, SaveUserIndex);
	if (!bSaved)
	{
		UE_LOG(LogTemp, Warning, TEXT("Save: could not write slot %s"), *SaveSlotName);
	}
	return bSaved;
}
bool UUnshackledGameInstance::LoadCheckpoint()
{
	if (!HasSaveGame())
	{
		return false;
	}
	USaveGame* LoadedSave = UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex);
	UUnshackledSaveGame* SaveData = Cast<UUnshackledSaveGame>(LoadedSave);
	if (SaveData == nullptr)
	{
		return false;
	}
	CurrentFloor = FMath::Clamp(SaveData->Floor, 1, MaxFloor);
	CheckpointLevelName = SaveData->CheckpointLevelName;
	if (CheckpointLevelName.IsNone())
	{
		CheckpointLevelName = OverworldLevelName;
	}
	RecruitedIds = SaveData->RecruitedIds;
	if (!RecruitedIds.Contains(FName("Sirius")))
	{
		RecruitedIds.Insert(FName("Sirius"), 0);
	}
	FloorStartRecruitedIds = RecruitedIds;
	FloorStartStates.Empty();
	for (int32 i = 0; i < SaveData->CharacterIds.Num(); i++)
	{
		if (i < SaveData->CharacterHealth.Num())
		{
			if (i < SaveData->CharacterMana.Num())
			{
				FCharacterState LoadedState;
				LoadedState.CharacterId = SaveData->CharacterIds[i];
				LoadedState.HealthFraction = SaveData->CharacterHealth[i];
				LoadedState.ManaFraction = SaveData->CharacterMana[i];
				FloorStartStates.Add(LoadedState);
			}
		}
	}
	return true;
}
void UUnshackledGameInstance::DeleteSaveGame()
{
	if (HasSaveGame())
	{
		UGameplayStatics::DeleteGameInSlot(SaveSlotName, SaveUserIndex);
	}
}
void UUnshackledGameInstance::StartNewGame()
{
	DeleteSaveGame();
	ResetToFirstFloor();
	UGameplayStatics::OpenLevel(this, OverworldLevelName);
}
bool UUnshackledGameInstance::ContinueGame()
{
	bool bLoaded = LoadCheckpoint();
	if (!bLoaded)
	{
		return false;
	}
	UGameplayStatics::OpenLevel(this, CheckpointLevelName);
	return true;
}
void UUnshackledGameInstance::QuitToTitle()
{
	UGameplayStatics::OpenLevel(this, TitleLevelName);
}
void UUnshackledGameInstance::ExitGame(APlayerController* PlayerController)
{
	UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, false);
}
