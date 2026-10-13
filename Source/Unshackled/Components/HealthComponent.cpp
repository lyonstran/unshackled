#include "HealthComponent.h"
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}
void UHealthComponent::TakeHit(float Amount)
{
	if (IsDead())
	{
		return;
	}
	CurrentHealth = CurrentHealth - Amount;
	if (CurrentHealth < HealthFloor)
	{
		CurrentHealth = HealthFloor;
	}
}
void UHealthComponent::Heal(float Amount)
{
	if (IsDead())
	{
		return;
	}
	CurrentHealth = CurrentHealth + Amount;
	if (CurrentHealth > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
}
void UHealthComponent::Revive()
{
	CurrentHealth = MaxHealth / 2.0f;
}
bool UHealthComponent::IsDead() const
{
	return CurrentHealth <= 0.0f;
}
void UHealthComponent::SetMaxHealthFromStats(float NewMaxHealth)
{
	float OldMaxHealth = MaxHealth;
	MaxHealth = NewMaxHealth;
	if (IsDead())
	{
		return;
	}
	if (NewMaxHealth > OldMaxHealth)
	{
		float HealthGained = NewMaxHealth - OldMaxHealth;
		CurrentHealth = CurrentHealth + HealthGained;
	}
	if (CurrentHealth > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
}