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
	CurrentHealth -= Amount;
	if (CurrentHealth < 0.0f)
	{
		CurrentHealth = 0.0f;
	}
}

void UHealthComponent::Heal(float Amount)
{
	if (IsDead())
	{
		return;
	}
	CurrentHealth += Amount;
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
