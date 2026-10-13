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

	// Never go below the floor (the floor is 0 unless an ability raised it).
	if (CurrentHealth < HealthFloor)
	{
		CurrentHealth = HealthFloor;
	}
}

void UHealthComponent::Heal(float Amount)
{
	// Healing does nothing on a dead character. Only Revive brings one back.
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

// When max health goes UP, current health goes up by the same amount, so the gain is real.
// When it goes DOWN, current health is only cut if it is now above the new max.
void UHealthComponent::SetMaxHealthFromStats(float NewMaxHealth)
{
	float OldMaxHealth = MaxHealth;
	MaxHealth = NewMaxHealth;

	// Dead characters do not get healed by a max health change.
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