#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Items/ItemRows.h"
#include "UnshackledGameInstance.generated.h"
class APartyMemberCharacter;
class APlayerController;
struct FCharacterState
{
	FName CharacterId;
	float HealthFraction = 0.0f;
	float ManaFraction = 0.0f;
};
UCLASS()
class UNSHACKLED_API UUnshackledGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;
	UPROPERTY(VisibleAnywhere, Category = "Progress")
	int32 CurrentFloor = 1;
	UPROPERTY(EditAnywhere, Category = "Progress")
	int32 MaxFloor = 5;
	UPROPERTY(VisibleAnywhere, Category = "Progress")
	FName CheckpointLevelName;
	UPROPERTY(EditAnywhere, Category = "Levels")
	FName TitleLevelName = FName("Level_Title");
	UPROPERTY(EditAnywhere, Category = "Levels")
	FName OverworldLevelName = FName("Level_Prototype");
	UPROPERTY(EditAnywhere, Category = "Save")
	FString SaveSlotName = TEXT("UnshackledSave");
	UPROPERTY(EditAnywhere, Category = "Save")
	int32 SaveUserIndex = 0;
	UPROPERTY(EditAnywhere, Category = "Floor Growth")
	float PartyStatGrowthPerFloor = 0.12f;
	UPROPERTY(EditAnywhere, Category = "Floor Growth")
	float PartyManaCostGrowthPerFloor = 0.04f;
	UPROPERTY(EditAnywhere, Category = "Floor Growth")
	float EnemyStatGrowthPerFloor = 0.06f;
	UPROPERTY(EditAnywhere, Category = "Items")
	FString ItemTablePath = TEXT("/Game/Data/DT_Items.DT_Items");
	UPROPERTY(EditAnywhere, Category = "Items")
	FString ArtifactTablePath = TEXT("/Game/Data/DT_Artifacts.DT_Artifacts");
	UPROPERTY(EditAnywhere, Category = "Items")
	float EnemyDropChance = 0.2f;
	UPROPERTY(EditAnywhere, Category = "Items")
	int32 MinDropCount = 1;
	UPROPERTY(EditAnywhere, Category = "Items")
	int32 MaxDropCount = 3;
	UPROPERTY(EditAnywhere, Category = "Items")
	int32 MaxLootFloor = 4;
	UPROPERTY(EditAnywhere, Category = "Items")
	int32 MaxEquippedArtifacts = 3;
	UPROPERTY(EditAnywhere, Category = "Items")
	float MaxCooldownReduction = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Items")
	float BattleFailChanceScale = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Items")
	float OverworldManaRegenScale = 0.75f;
	UPROPERTY(EditAnywhere, Category = "Items")
	int32 StarterHealthPotions = 2;
	UPROPERTY(EditAnywhere, Category = "Items")
	int32 StarterManaPotions = 1;
	bool IsLastFloor() const;
	bool AdvanceFloor();
	int32 GetFloorsCleared() const;
	float GetPartyStatScale() const;
	float GetPartyManaCostScale() const;
	float GetEnemyStatScale() const;
	void ResetToFirstFloor();
	void SaveFloorStartStates();
	bool RestoreCharacterState(APartyMemberCharacter* Member);
	bool IsRecruited(FName CharacterId) const;
	void Recruit(FName CharacterId);
	int32 GetRecruitedCount() const;
	void RestoreFloorStartRecruits();
	void ReachCheckpoint(FName LevelName);
	FString GetCheckpointDescription() const;
	bool HasSaveGame() const;
	bool SaveCheckpoint();
	bool LoadCheckpoint();
	void DeleteSaveGame();
	void StartNewGame();
	bool ContinueGame();
	void QuitToTitle();
	void ExitGame(APlayerController* PlayerController);
	const FActiveItemRow* GetItemRow(FName ItemId);
	const FArtifactRow* GetArtifactRow(FName ArtifactId);
	FString GetItemName(FName ItemId);
	FString GetArtifactName(FName ArtifactId);
	TArray<FName> GetHeldItemIds() const;
	int32 GetItemCount(FName ItemId) const;
	void AddItem(FName ItemId, int32 Count);
	bool ConsumeItem(FName ItemId);
	TArray<FName> RollEnemyDrops();
	TArray<FName> GetOwnedArtifacts() const;
	bool IsArtifactEquipped(FName ArtifactId) const;
	int32 GetEquippedCount() const;
	bool ToggleArtifact(FName ArtifactId);
	FName AwardArtifact();
	TArray<FName> GetEquippedArtifactIds() const;
	float GetCooldownReduction();
	float GetManaRegenBonus();
	float GetDropChanceBonus();
	float GetHealShieldBonus();
	float GetEnemyFailChance(bool bInBattle);
	float GetEnemyBuffScale();
	float GetOverworldManaRegenScale();
	void RefreshPartyArtifacts();
	void RestoreFloorStartInventory();
	UFUNCTION(Exec)
	void GiveAllItems();
private:
	TArray<FName> RecruitedIds;
	TArray<FName> FloorStartRecruitedIds;
	TArray<FCharacterState> FloorStartStates;
	TArray<FName> ItemIds;
	TArray<int32> ItemCounts;
	TArray<FName> ArtifactIds;
	TArray<FName> EquippedArtifactIds;
	TArray<FName> FloorStartItemIds;
	TArray<int32> FloorStartItemCounts;
	TArray<FName> FloorStartArtifactIds;
	TArray<FName> FloorStartEquippedIds;
	UPROPERTY()
	UDataTable* ItemTable = nullptr;
	UPROPERTY()
	UDataTable* ArtifactTable = nullptr;
	int32 FindStateIndex(FName CharacterId) const;
	float GetHealthFraction(APartyMemberCharacter* Member) const;
	float GetManaFraction(APartyMemberCharacter* Member) const;
	UDataTable* GetItemTable();
	UDataTable* GetArtifactTable();
	int32 GetLootFloor() const;
	void SaveFloorStartInventory();
	void ClearInventory();
	void GiveStarterItems();
	TArray<FName> GetUnownedArtifactsOnFloor(int32 Floor);
};
