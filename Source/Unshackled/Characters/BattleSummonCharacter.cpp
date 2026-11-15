#include "Characters/BattleSummonCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Characters/PartyMemberCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
ABattleSummonCharacter::ABattleSummonCharacter()
{
	bIsEnemy = true;
	DisplayName = TEXT("Summon");
	bPicksMovesRandomly = true;
}
void ABattleSummonCharacter::SetBattleMaxHealth(float NewMaxHealth)
{
	Stats->BaseMaxHealth = NewMaxHealth;
	Health->MaxHealth = NewMaxHealth;
	Health->CurrentHealth = NewMaxHealth;
}
void ABattleSummonCharacter::TakeBattleTurn(ATurnBattleManager* Manager)
{
	if (Manager == nullptr)
	{
		return;
	}
	if (TakeTurnFromMoveTable(Manager))
	{
		return;
	}
	int32 TargetIndex = Manager->PickRandomLivingMember();
	APartyMemberCharacter* Target = Manager->GetPartyMember(TargetIndex);
	if (Target == nullptr)
	{
		return;
	}
	float Scale = StrikePower;
	if (Manager->IsMemberBlocking(TargetIndex))
	{
		Scale = Scale * BlockMultiplier;
	}
	DealDamage(Target, Scale, false);
	Manager->ShowMessage(FString::Printf(TEXT("%s strikes %s"), *DisplayName, *Target->DisplayName), FColor::Orange);
}
void ABattleSummonCharacter::DieInBattle()
{
	if (Health->IsDead())
	{
		return;
	}
	Health->CurrentHealth = 0.0f;
	Die();
}
void ABattleSummonCharacter::Die()
{
	Super::Die();
	SetActorHiddenInGame(true);
}