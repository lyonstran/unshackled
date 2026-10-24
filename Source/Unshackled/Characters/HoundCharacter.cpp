#include "HoundCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
AHoundCharacter::AHoundCharacter()
{
	StatsRowName = FName("Hound");
	AttackDamage = 18.0f;
	Health->MaxHealth = 90.0f;
	Stats->BaseDefense = 20.0f;
}
void AHoundCharacter::Tick(float DeltaTime)
{
	AUnshackledCharacterBase::Tick(DeltaTime);
	if (Health->IsDead())
	{
		return;
	}
	if (!Stats->CanAct())
	{
		return;
	}
	SetupIfNeeded();
	if (PounceCooldownLeft > 0.0f)
	{
		PounceCooldownLeft = PounceCooldownLeft - DeltaTime;
	}
	if (State == EHoundState::Sniffing)
	{
		TickSniffing(DeltaTime);
	}
	else if (State == EHoundState::Hunting)
	{
		TickHunting(DeltaTime);
	}
	else if (State == EHoundState::Pouncing)
	{
		TickPouncing(DeltaTime);
	}
	else
	{
		TickLeapingBack(DeltaTime);
	}
}
void AHoundCharacter::SetupIfNeeded()
{
	if (bSetupDone)
	{
		return;
	}
	bSetupDone = true;
	HomeLocation = GetActorLocation();
	WanderPoint = HomeLocation;
	FStatModifier SniffSlow;
	SniffSlow.SourceId = FName("HoundSniff");
	SniffSlow.Stat = ECharacterStat::Speed;
	SniffSlow.Kind = EModifierKind::Percent;
	SniffSlow.Amount = 0.0f - SniffSlowFraction;
	SniffSlow.Duration = 0.0f;
	SniffSlow.Rule = EStackRule::RefreshSameSource;
	Stats->AddModifier(SniffSlow);
}
void AHoundCharacter::MoveToward(FVector Direction)
{
	FVector2D MoveInput = FVector2D(Direction.Y, Direction.X);
	MoveCharacter(MoveInput);
}
void AHoundCharacter::TickSniffing(float DeltaTime)
{
	DrawDebugCircle(GetWorld(), GetActorLocation(), SniffRadius, 48, FColor::Yellow, false, 0.0f, 0, 2.0f, FVector(1.0f, 0.0f, 0.0f), FVector(0.0f, 1.0f, 0.0f), false);
	AUnshackledCharacterBase* Target = FindClosestTarget();
	if (Target != nullptr)
	{
		float Distance = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());
		if (Distance <= SniffRadius)
		{
			StartHunting();
			return;
		}
	}
	WanderTimeLeft = WanderTimeLeft - DeltaTime;
	if (WanderTimeLeft <= 0.0f)
	{
		PickWanderPoint();
	}
	FVector ToPoint = WanderPoint - GetActorLocation();
	ToPoint.Z = 0.0f;
	if (ToPoint.Size() > 30.0f)
	{
		FVector Direction = ToPoint;
		Direction.Normalize();
		MoveToward(Direction);
	}
}
void AHoundCharacter::PickWanderPoint()
{
	WanderTimeLeft = WanderInterval;
	float AngleRadians = FMath::FRandRange(0.0f, 2.0f * PI);
	float Distance = WanderRadius * FMath::Sqrt(FMath::FRand());
	FVector Offset = FVector(FMath::Cos(AngleRadians), FMath::Sin(AngleRadians), 0.0f) * Distance;
	WanderPoint = HomeLocation + Offset;
}
void AHoundCharacter::StartHunting()
{
	Stats->RemoveModifiersFromSource(FName("HoundSniff"));
	FStatModifier HuntHaste;
	HuntHaste.SourceId = FName("HoundHunt");
	HuntHaste.Stat = ECharacterStat::Speed;
	HuntHaste.Kind = EModifierKind::Percent;
	HuntHaste.Amount = HuntSpeedBonus;
	HuntHaste.Duration = 0.0f;
	HuntHaste.Rule = EStackRule::RefreshSameSource;
	Stats->AddModifier(HuntHaste);
	State = EHoundState::Hunting;
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow, TEXT("Hound caught a scent!"));
}
void AHoundCharacter::TickHunting(float DeltaTime)
{
	AUnshackledCharacterBase* Target = FindClosestTarget();
	if (Target == nullptr)
	{
		return;
	}
	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	float Distance = ToTarget.Size();
	FVector Direction = ToTarget;
	Direction.Normalize();
	if (Distance <= PounceTriggerDistance)
	{
		if (PounceCooldownLeft <= 0.0f)
		{
			StartPounce(Target);
			return;
		}
	}
	if (Distance > BiteStopDistance)
	{
		MoveToward(Direction);
	}
}
void AHoundCharacter::StartPounce(AUnshackledCharacterBase* Target)
{
	PounceTarget = Target;
	PounceTimeLeft = PounceMaxTime;
	State = EHoundState::Pouncing;
}
void AHoundCharacter::TickPouncing(float DeltaTime)
{
	if (!IsValid(PounceTarget))
	{
		State = EHoundState::Hunting;
		return;
	}
	if (PounceTarget->Health->IsDead())
	{
		State = EHoundState::Hunting;
		return;
	}
	FVector ToTarget = PounceTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	float Distance = ToTarget.Size();
	FVector Direction = ToTarget;
	Direction.Normalize();
	FacingDirection = Direction;
	if (Distance <= PounceHitDistance)
	{
		BiteTarget(PounceTarget);
		StartLeapBack();
		return;
	}
	float StepLength = PounceSpeed * DeltaTime;
	if (StepLength > Distance)
	{
		StepLength = Distance;
	}
	SetActorLocation(GetActorLocation() + (Direction * StepLength));
	PounceTimeLeft = PounceTimeLeft - DeltaTime;
	if (PounceTimeLeft <= 0.0f)
	{
		StartLeapBack();
	}
}
void AHoundCharacter::BiteTarget(AUnshackledCharacterBase* Target)
{
	float HealthBefore = Target->Health->CurrentHealth;
	DealDamage(Target, PounceDamageScale, false);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Hound bite!"));
	if (Target->Health->IsDead())
	{
		return;
	}
	if (Target->Health->CurrentHealth >= HealthBefore)
	{
		return;
	}
	FStatModifier BiteSlow;
	BiteSlow.SourceId = FName("HoundBiteSlow");
	BiteSlow.Stat = ECharacterStat::Speed;
	BiteSlow.Kind = EModifierKind::Percent;
	BiteSlow.Amount = 0.0f - BiteSlowFraction;
	BiteSlow.Duration = BiteSlowDuration;
	BiteSlow.Rule = EStackRule::RefreshSameSource;
	Target->Stats->AddModifier(BiteSlow);
}
void AHoundCharacter::StartLeapBack()
{
	FVector AwayFromTarget = FVector(-1.0f, 0.0f, 0.0f);
	if (IsValid(PounceTarget))
	{
		AwayFromTarget = GetActorLocation() - PounceTarget->GetActorLocation();
		AwayFromTarget.Z = 0.0f;
		AwayFromTarget.Normalize();
	}
	LeapDirection = AwayFromTarget;
	LeapDistanceLeft = LeapBackDistance;
	State = EHoundState::LeapingBack;
}
void AHoundCharacter::TickLeapingBack(float DeltaTime)
{
	float StepLength = LeapBackSpeed * DeltaTime;
	if (StepLength > LeapDistanceLeft)
	{
		StepLength = LeapDistanceLeft;
	}
	SetActorLocation(GetActorLocation() + (LeapDirection * StepLength));
	LeapDistanceLeft = LeapDistanceLeft - StepLength;
	if (LeapDistanceLeft <= 0.0f)
	{
		EndLeapBack();
	}
}
void AHoundCharacter::EndLeapBack()
{
	State = EHoundState::Hunting;
	PounceCooldownLeft = PounceCooldown;
}
