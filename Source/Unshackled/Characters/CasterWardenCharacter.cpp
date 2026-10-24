#include "CasterWardenCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
ACasterWardenCharacter::ACasterWardenCharacter()
{
	StatsRowName = FName("CasterWarden");
	AttackDamage = 14.0f;
	Health->MaxHealth = 50.0f;
	Stats->BaseDefense = 15.0f;
}
void ACasterWardenCharacter::Tick(float DeltaTime)
{
	AUnshackledCharacterBase::Tick(DeltaTime);
	if (Health->IsDead())
	{
		return;
	}
	TickMagicCircle(DeltaTime);
	if (!Stats->CanAct())
	{
		return;
	}
	if (AttackCooldownLeft > 0.0f)
	{
		AttackCooldownLeft = AttackCooldownLeft - DeltaTime;
	}
	if (State == ECasterWardenState::Idle)
	{
		TickIdle(DeltaTime);
	}
	else
	{
		TickWinding(DeltaTime);
	}
}
void ACasterWardenCharacter::TickIdle(float DeltaTime)
{
	if (AttackCooldownLeft > 0.0f)
	{
		return;
	}
	AUnshackledCharacterBase* Target = FindClosestTarget();
	if (Target == nullptr)
	{
		return;
	}
	StartWindup(Target);
}
void ACasterWardenCharacter::StartWindup(AUnshackledCharacterBase* Target)
{
	LockedTarget = Target;
	WindupTimeLeft = CometWindupTime;
	State = ECasterWardenState::Winding;
}
void ACasterWardenCharacter::TickWinding(float DeltaTime)
{
	if (!IsValid(LockedTarget))
	{
		State = ECasterWardenState::Idle;
		return;
	}
	if (LockedTarget->Health->IsDead())
	{
		State = ECasterWardenState::Idle;
		return;
	}
	FVector ToTarget = LockedTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	FVector Direction = ToTarget;
	Direction.Normalize();
	FacingDirection = Direction;
	DrawDebugSphere(GetWorld(), LockedTarget->GetActorLocation(), 60.0f, 12, FColor::Magenta, false, 0.0f);
	WindupTimeLeft = WindupTimeLeft - DeltaTime;
	if (WindupTimeLeft <= 0.0f)
	{
		FireComets();
	}
}
void ACasterWardenCharacter::FireComets()
{
	State = ECasterWardenState::Idle;
	AttackCooldownLeft = CometCooldown;
	if (!IsValid(LockedTarget))
	{
		return;
	}
	if (LockedTarget->Health->IsDead())
	{
		return;
	}
	float CometDamage = LockedTarget->Health->CurrentHealth * CometCurrentHealthFraction;
	float DamageDealt = LockedTarget->ApplyHit(CometDamage, true, this);
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Magenta, TEXT("Caster warden comet cluster!"));
	if (DamageDealt <= 0.0f)
	{
		return;
	}
	if (LockedTarget->Health->IsDead())
	{
		return;
	}
	FStatModifier DefenseLoss;
	DefenseLoss.SourceId = FName("CasterWardenComet");
	DefenseLoss.Stat = ECharacterStat::Defense;
	DefenseLoss.Kind = EModifierKind::Percent;
	DefenseLoss.Amount = 0.0f - CometDefenseLoss;
	DefenseLoss.Duration = CometDefenseLossDuration;
	DefenseLoss.Rule = EStackRule::RefreshSameSource;
	LockedTarget->Stats->AddModifier(DefenseLoss);
}
void ACasterWardenCharacter::TickMagicCircle(float DeltaTime)
{
	CircleTimer = CircleTimer + DeltaTime;
	if (CircleTimer < CircleInterval)
	{
		return;
	}
	CircleTimer = 0.0f;
	for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
	{
		AUnshackledCharacterBase* Candidate = *It;
		if (Candidate != this)
		{
			if (Candidate->bIsEnemy != bIsEnemy)
			{
				if (!Candidate->Health->IsDead())
				{
					float Distance = FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation());
					if (Distance <= CircleRadius)
					{
						ApplyCircleDebuff(Candidate);
					}
				}
			}
		}
	}
}
void ACasterWardenCharacter::ApplyCircleDebuff(AUnshackledCharacterBase* Ally)
{
	FStatModifier DefenseLoss;
	DefenseLoss.SourceId = FName("CasterWardenCircle");
	DefenseLoss.Stat = ECharacterStat::Defense;
	DefenseLoss.Kind = EModifierKind::Percent;
	DefenseLoss.Amount = 0.0f - CircleDefenseLoss;
	DefenseLoss.Duration = CircleDuration;
	DefenseLoss.Rule = EStackRule::RefreshSameSource;
	Ally->Stats->AddModifier(DefenseLoss);
}
