#include "UnshackledGameInstance.h"
#include "Characters/PartyMemberCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "EngineUtils.h"
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
