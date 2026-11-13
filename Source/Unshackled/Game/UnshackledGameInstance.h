#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UnshackledGameInstance.generated.h"
class APartyMemberCharacter;
class APlayerController;
struct FCharacterState
{
	FName CharacterId;
	float CurrentHealth = 0.0f;
	float CurrentMana = 0.0f;
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
	bool IsLastFloor() const;
	bool AdvanceFloor();
	void ResetToFirstFloor();
	void SaveFloorStartStates();
	bool RestoreCharacterState(APartyMemberCharacter* Member);
	bool IsRecruited(FName CharacterId) const;
	void Recruit(FName CharacterId);
	int32 GetRecruitedCount() const;
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
	TArray<FCharacterState> FloorStartStates;
	int32 FindStateIndex(FName CharacterId) const;
};
