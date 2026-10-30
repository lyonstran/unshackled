#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UnshackledGameInstance.generated.h"
class APartyMemberCharacter;
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
	bool IsLastFloor() const;
	bool AdvanceFloor();
	void ResetToFirstFloor();
	void SaveFloorStartStates();
	bool RestoreCharacterState(APartyMemberCharacter* Member);
	bool IsRecruited(FName CharacterId) const;
	void Recruit(FName CharacterId);
	int32 GetRecruitedCount() const;
private:
	TArray<FName> RecruitedIds;
	TArray<FCharacterState> FloorStartStates;
	int32 FindStateIndex(FName CharacterId) const;
};
