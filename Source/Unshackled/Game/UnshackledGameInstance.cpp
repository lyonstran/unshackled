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
	ClearInventory();
	GiveStarterItems();
	SaveFloorStartInventory();
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
	SaveFloorStartInventory();
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
	SaveData->ItemIds = FloorStartItemIds;
	SaveData->ItemCounts = FloorStartItemCounts;
	SaveData->ArtifactIds = FloorStartArtifactIds;
	SaveData->EquippedArtifactIds = FloorStartEquippedIds;
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
	ClearInventory();
	for (int32 i = 0; i < SaveData->ItemIds.Num(); i++)
	{
		if (i < SaveData->ItemCounts.Num())
		{
			AddItem(SaveData->ItemIds[i], SaveData->ItemCounts[i]);
		}
	}
	ArtifactIds = SaveData->ArtifactIds;
	EquippedArtifactIds = SaveData->EquippedArtifactIds;
	SaveFloorStartInventory();
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
UDataTable* UUnshackledGameInstance::GetItemTable()
{
	if (ItemTable == nullptr)
	{
		ItemTable = LoadObject<UDataTable>(nullptr, *ItemTablePath);
		if (ItemTable == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Items: no data table at %s"), *ItemTablePath);
		}
	}
	return ItemTable;
}
UDataTable* UUnshackledGameInstance::GetArtifactTable()
{
	if (ArtifactTable == nullptr)
	{
		ArtifactTable = LoadObject<UDataTable>(nullptr, *ArtifactTablePath);
		if (ArtifactTable == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Items: no data table at %s"), *ArtifactTablePath);
		}
	}
	return ArtifactTable;
}
const FActiveItemRow* UUnshackledGameInstance::GetItemRow(FName ItemId)
{
	UDataTable* Table = GetItemTable();
	if (Table == nullptr)
	{
		return nullptr;
	}
	if (ItemId.IsNone())
	{
		return nullptr;
	}
	return Table->FindRow<FActiveItemRow>(ItemId, TEXT("Items"), false);
}
const FArtifactRow* UUnshackledGameInstance::GetArtifactRow(FName ArtifactId)
{
	UDataTable* Table = GetArtifactTable();
	if (Table == nullptr)
	{
		return nullptr;
	}
	if (ArtifactId.IsNone())
	{
		return nullptr;
	}
	return Table->FindRow<FArtifactRow>(ArtifactId, TEXT("Artifacts"), false);
}
FString UUnshackledGameInstance::GetItemName(FName ItemId)
{
	const FActiveItemRow* Row = GetItemRow(ItemId);
	if (Row == nullptr)
	{
		return ItemId.ToString();
	}
	return Row->DisplayName.ToString();
}
FString UUnshackledGameInstance::GetArtifactName(FName ArtifactId)
{
	const FArtifactRow* Row = GetArtifactRow(ArtifactId);
	if (Row == nullptr)
	{
		return ArtifactId.ToString();
	}
	return Row->DisplayName.ToString();
}
int32 UUnshackledGameInstance::GetLootFloor() const
{
	int32 Floor = CurrentFloor;
	if (Floor > MaxLootFloor)
	{
		Floor = MaxLootFloor;
	}
	if (Floor < 1)
	{
		Floor = 1;
	}
	return Floor;
}
TArray<FName> UUnshackledGameInstance::GetHeldItemIds() const
{
	TArray<FName> HeldIds;
	for (int32 i = 0; i < ItemIds.Num(); i++)
	{
		if (ItemCounts[i] > 0)
		{
			HeldIds.Add(ItemIds[i]);
		}
	}
	return HeldIds;
}
int32 UUnshackledGameInstance::GetItemCount(FName ItemId) const
{
	int32 Index = ItemIds.Find(ItemId);
	if (Index == INDEX_NONE)
	{
		return 0;
	}
	return ItemCounts[Index];
}
void UUnshackledGameInstance::AddItem(FName ItemId, int32 Count)
{
	if (ItemId.IsNone())
	{
		return;
	}
	if (Count <= 0)
	{
		return;
	}
	int32 Index = ItemIds.Find(ItemId);
	if (Index == INDEX_NONE)
	{
		ItemIds.Add(ItemId);
		ItemCounts.Add(Count);
		return;
	}
	ItemCounts[Index] = ItemCounts[Index] + Count;
}
bool UUnshackledGameInstance::ConsumeItem(FName ItemId)
{
	int32 Index = ItemIds.Find(ItemId);
	if (Index == INDEX_NONE)
	{
		return false;
	}
	if (ItemCounts[Index] <= 0)
	{
		return false;
	}
	ItemCounts[Index] = ItemCounts[Index] - 1;
	if (ItemCounts[Index] <= 0)
	{
		ItemIds.RemoveAt(Index);
		ItemCounts.RemoveAt(Index);
	}
	return true;
}
TArray<FName> UUnshackledGameInstance::RollEnemyDrops()
{
	TArray<FName> Dropped;
	float DropChance = EnemyDropChance + GetDropChanceBonus();
	if (FMath::FRand() >= DropChance)
	{
		return Dropped;
	}
	UDataTable* Table = GetItemTable();
	if (Table == nullptr)
	{
		return Dropped;
	}
	int32 LootFloor = GetLootFloor();
	TArray<FName> Pool;
	TArray<FName> AllNames = Table->GetRowNames();
	for (int32 i = 0; i < AllNames.Num(); i++)
	{
		const FActiveItemRow* Row = GetItemRow(AllNames[i]);
		if (Row != nullptr)
		{
			if (Row->Floor == LootFloor)
			{
				Pool.Add(AllNames[i]);
			}
		}
	}
	if (Pool.Num() == 0)
	{
		return Dropped;
	}
	int32 DropCount = FMath::RandRange(MinDropCount, MaxDropCount);
	for (int32 i = 0; i < DropCount; i++)
	{
		int32 Pick = FMath::RandRange(0, Pool.Num() - 1);
		AddItem(Pool[Pick], 1);
		Dropped.Add(Pool[Pick]);
	}
	return Dropped;
}
TArray<FName> UUnshackledGameInstance::GetOwnedArtifacts() const
{
	return ArtifactIds;
}
bool UUnshackledGameInstance::IsArtifactEquipped(FName ArtifactId) const
{
	return EquippedArtifactIds.Contains(ArtifactId);
}
int32 UUnshackledGameInstance::GetEquippedCount() const
{
	return EquippedArtifactIds.Num();
}
TArray<FName> UUnshackledGameInstance::GetEquippedArtifactIds() const
{
	return EquippedArtifactIds;
}
bool UUnshackledGameInstance::ToggleArtifact(FName ArtifactId)
{
	if (!ArtifactIds.Contains(ArtifactId))
	{
		return false;
	}
	if (EquippedArtifactIds.Contains(ArtifactId))
	{
		EquippedArtifactIds.Remove(ArtifactId);
		RefreshPartyArtifacts();
		return true;
	}
	if (EquippedArtifactIds.Num() >= MaxEquippedArtifacts)
	{
		return false;
	}
	EquippedArtifactIds.Add(ArtifactId);
	RefreshPartyArtifacts();
	return true;
}
TArray<FName> UUnshackledGameInstance::GetUnownedArtifactsOnFloor(int32 Floor)
{
	TArray<FName> Pool;
	UDataTable* Table = GetArtifactTable();
	if (Table == nullptr)
	{
		return Pool;
	}
	TArray<FName> AllNames = Table->GetRowNames();
	for (int32 i = 0; i < AllNames.Num(); i++)
	{
		const FArtifactRow* Row = GetArtifactRow(AllNames[i]);
		if (Row != nullptr)
		{
			if (Row->Floor == Floor)
			{
				if (!ArtifactIds.Contains(AllNames[i]))
				{
					Pool.Add(AllNames[i]);
				}
			}
		}
	}
	return Pool;
}
FName UUnshackledGameInstance::AwardArtifact()
{
	int32 Floor = GetLootFloor();
	while (Floor >= 1)
	{
		TArray<FName> Pool = GetUnownedArtifactsOnFloor(Floor);
		if (Pool.Num() > 0)
		{
			int32 Pick = FMath::RandRange(0, Pool.Num() - 1);
			FName Found = Pool[Pick];
			ArtifactIds.Add(Found);
			if (EquippedArtifactIds.Num() < MaxEquippedArtifacts)
			{
				EquippedArtifactIds.Add(Found);
				RefreshPartyArtifacts();
			}
			return Found;
		}
		Floor = Floor - 1;
	}
	return NAME_None;
}
float UUnshackledGameInstance::GetCooldownReduction()
{
	float Total = 0.0f;
	for (int32 i = 0; i < EquippedArtifactIds.Num(); i++)
	{
		const FArtifactRow* Row = GetArtifactRow(EquippedArtifactIds[i]);
		if (Row != nullptr)
		{
			Total = Total + Row->CooldownReduction;
		}
	}
	if (Total > MaxCooldownReduction)
	{
		Total = MaxCooldownReduction;
	}
	return Total;
}
float UUnshackledGameInstance::GetManaRegenBonus()
{
	float Total = 0.0f;
	for (int32 i = 0; i < EquippedArtifactIds.Num(); i++)
	{
		const FArtifactRow* Row = GetArtifactRow(EquippedArtifactIds[i]);
		if (Row != nullptr)
		{
			Total = Total + Row->ManaRegenBonus;
		}
	}
	return Total;
}
float UUnshackledGameInstance::GetDropChanceBonus()
{
	float Total = 0.0f;
	for (int32 i = 0; i < EquippedArtifactIds.Num(); i++)
	{
		const FArtifactRow* Row = GetArtifactRow(EquippedArtifactIds[i]);
		if (Row != nullptr)
		{
			Total = Total + Row->DropChanceBonus;
		}
	}
	return Total;
}
float UUnshackledGameInstance::GetHealShieldBonus()
{
	float Total = 0.0f;
	for (int32 i = 0; i < EquippedArtifactIds.Num(); i++)
	{
		const FArtifactRow* Row = GetArtifactRow(EquippedArtifactIds[i]);
		if (Row != nullptr)
		{
			Total = Total + Row->HealShieldBonus;
		}
	}
	return Total;
}
float UUnshackledGameInstance::GetEnemyFailChance(bool bInBattle)
{
	float Total = 0.0f;
	for (int32 i = 0; i < EquippedArtifactIds.Num(); i++)
	{
		const FArtifactRow* Row = GetArtifactRow(EquippedArtifactIds[i]);
		if (Row != nullptr)
		{
			Total = Total + Row->EnemyFailChance;
		}
	}
	if (bInBattle)
	{
		Total = Total * BattleFailChanceScale;
	}
	return Total;
}
float UUnshackledGameInstance::GetEnemyBuffScale()
{
	float Reduction = 0.0f;
	for (int32 i = 0; i < EquippedArtifactIds.Num(); i++)
	{
		const FArtifactRow* Row = GetArtifactRow(EquippedArtifactIds[i]);
		if (Row != nullptr)
		{
			Reduction = Reduction + Row->EnemyBuffReduction;
		}
	}
	float Scale = 1.0f - Reduction;
	if (Scale < 0.0f)
	{
		Scale = 0.0f;
	}
	return Scale;
}
float UUnshackledGameInstance::GetOverworldManaRegenScale()
{
	return OverworldManaRegenScale * (1.0f + GetManaRegenBonus());
}
void UUnshackledGameInstance::RefreshPartyArtifacts()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	for (TActorIterator<APartyMemberCharacter> It(World); It; ++It)
	{
		APartyMemberCharacter* Member = *It;
		Member->RefreshArtifactEffects();
	}
}
void UUnshackledGameInstance::SaveFloorStartInventory()
{
	FloorStartItemIds = ItemIds;
	FloorStartItemCounts = ItemCounts;
	FloorStartArtifactIds = ArtifactIds;
	FloorStartEquippedIds = EquippedArtifactIds;
}
void UUnshackledGameInstance::RestoreFloorStartInventory()
{
	ItemIds = FloorStartItemIds;
	ItemCounts = FloorStartItemCounts;
	ArtifactIds = FloorStartArtifactIds;
	EquippedArtifactIds = FloorStartEquippedIds;
}
void UUnshackledGameInstance::ClearInventory()
{
	ItemIds.Empty();
	ItemCounts.Empty();
	ArtifactIds.Empty();
	EquippedArtifactIds.Empty();
}
void UUnshackledGameInstance::GiveStarterItems()
{
	AddItem(FName("HealthPotion_Basic"), StarterHealthPotions);
	AddItem(FName("ManaPotion_Basic"), StarterManaPotions);
}
void UUnshackledGameInstance::GiveAllItems()
{
	UDataTable* Items = GetItemTable();
	if (Items != nullptr)
	{
		TArray<FName> ItemNames = Items->GetRowNames();
		for (int32 i = 0; i < ItemNames.Num(); i++)
		{
			AddItem(ItemNames[i], 3);
		}
	}
	UDataTable* Artifacts = GetArtifactTable();
	if (Artifacts != nullptr)
	{
		TArray<FName> ArtifactNames = Artifacts->GetRowNames();
		for (int32 i = 0; i < ArtifactNames.Num(); i++)
		{
			if (!ArtifactIds.Contains(ArtifactNames[i]))
			{
				ArtifactIds.Add(ArtifactNames[i]);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Items: gave 3 of every item and every artifact"));
}
