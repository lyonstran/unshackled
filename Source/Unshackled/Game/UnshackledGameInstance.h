#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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
private:
	TArray<FName> RecruitedIds;
	TArray<FName> FloorStartRecruitedIds;
	TArray<FCharacterState> FloorStartStates;
	int32 FindStateIndex(FName CharacterId) const;
	float GetHealthFraction(APartyMemberCharacter* Member) const;
	float GetManaFraction(APartyMemberCharacter* Member) const;
};
