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
		FloorStartStates[Index].CurrentHealth = Member->Health->CurrentHealth;
		FloorStartStates[Index].CurrentMana = Member->Stats->CurrentMana;
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
	float SavedHealth = FloorStartStates[Index].CurrentHealth;
	if (SavedHealth <= 0.0f)
	{
		return false;
	}
	if (SavedHealth > Member->Health->MaxHealth)
	{
		SavedHealth = Member->Health->MaxHealth;
	}
	Member->Health->CurrentHealth = SavedHealth;
	Member->Stats->CurrentMana = FloorStartStates[Index].CurrentMana;
	return true;
}
void UUnshackledGameInstance::ReachCheckpoint(FName LevelName)
{
	SaveFloorStartStates();
	CheckpointLevelName = LevelName;
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
	SaveData->RecruitedIds = RecruitedIds;
	for (int32 i = 0; i < FloorStartStates.Num(); i++)
	{
		SaveData->CharacterIds.Add(FloorStartStates[i].CharacterId);
		SaveData->CharacterHealth.Add(FloorStartStates[i].CurrentHealth);
		SaveData->CharacterMana.Add(FloorStartStates[i].CurrentMana);
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
	FloorStartStates.Empty();
	for (int32 i = 0; i < SaveData->CharacterIds.Num(); i++)
	{
		if (i < SaveData->CharacterHealth.Num())
		{
			if (i < SaveData->CharacterMana.Num())
			{
				FCharacterState LoadedState;
				LoadedState.CharacterId = SaveData->CharacterIds[i];
				LoadedState.CurrentHealth = SaveData->CharacterHealth[i];
				LoadedState.CurrentMana = SaveData->CharacterMana[i];
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
