#include "MagicFloorDirectorCharacter.h"
#include "Characters/PartyMemberCharacter.h"
#include "Characters/FlowerCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
AMagicFloorDirectorCharacter::AMagicFloorDirectorCharacter()
{
	StatsRowName = FName("MagicDirector");
	AttackDamage = 20.0f;
	Health->MaxHealth = 280.0f;
	Stats->BaseDefense = 40.0f;
	SummonTimeLeft = FirstSummonDelay;
}
void AMagicFloorDirectorCharacter::Tick(float DeltaTime)
{
	AUnshackledCharacterBase::Tick(DeltaTime);
	if (Health->IsDead())
	{
		return;
	}
	TickMagicCircle(DeltaTime);
	TickRally(DeltaTime);
	if (!Stats->CanAct())
	{
		return;
	}
	if (LaserCooldownLeft > 0.0f)
	{
		LaserCooldownLeft = LaserCooldownLeft - DeltaTime;
	}
	if (State == EMagicDirectorState::Idle)
	{
		TickIdle(DeltaTime);
	}
	else if (State == EMagicDirectorState::Winding)
	{
		TickWinding(DeltaTime);
	}
	else
	{
		TickFiring(DeltaTime);
	}
}
void AMagicFloorDirectorCharacter::TickIdle(float DeltaTime)
{
	TickSummon(DeltaTime);
	if (LaserCooldownLeft > 0.0f)
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
void AMagicFloorDirectorCharacter::StartWindup(AUnshackledCharacterBase* Target)
{
	LockedTarget = Target;
	WindupTimeLeft = LaserWindupTime;
	State = EMagicDirectorState::Winding;
	AimAtLockedTarget();
}
void AMagicFloorDirectorCharacter::AimAtLockedTarget()
{
	FVector ToTarget = LockedTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	FVector Direction = ToTarget;
	Direction.Normalize();
	LaserDirection = Direction;
	FacingDirection = Direction;
}
void AMagicFloorDirectorCharacter::TickWinding(float DeltaTime)
{
	if (!IsValid(LockedTarget))
	{
		State = EMagicDirectorState::Idle;
		return;
	}
	if (LockedTarget->Health->IsDead())
	{
		State = EMagicDirectorState::Idle;
		return;
	}
	AimAtLockedTarget();
	FVector Start = GetActorLocation();
	FVector End = Start + LaserDirection * LaserLength;
	DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, 0.0f, 0, 4.0f);
	WindupTimeLeft = WindupTimeLeft - DeltaTime;
	if (WindupTimeLeft <= 0.0f)
	{
		StartFiring();
	}
}
void AMagicFloorDirectorCharacter::StartFiring()
{
	LaserHitTargets.Empty();
	FiringTimeLeft = LaserDuration;
	State = EMagicDirectorState::Firing;
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Purple, TEXT("Magic director laser!"));
}
void AMagicFloorDirectorCharacter::TickFiring(float DeltaTime)
{
	FVector Start = GetActorLocation();
	FVector End = Start + LaserDirection * LaserLength;
	DrawDebugLine(GetWorld(), Start, End, FColor::Purple, false, 0.0f, 0, 14.0f);
	ApplyLaserToVictims();
	FiringTimeLeft = FiringTimeLeft - DeltaTime;
	if (FiringTimeLeft <= 0.0f)
	{
		EndFiring();
	}
}
void AMagicFloorDirectorCharacter::EndFiring()
{
	State = EMagicDirectorState::Idle;
	LaserCooldownLeft = LaserCooldown;
	LaserHitTargets.Empty();
}
bool AMagicFloorDirectorCharacter::CanBeLasered(AUnshackledCharacterBase* Candidate)
{
	if (Candidate->bIsEnemy == bIsEnemy)
	{
		return false;
	}
	if (Candidate->Health->IsDead())
	{
		return false;
	}
	APartyMemberCharacter* Member = Cast<APartyMemberCharacter>(Candidate);
	if (Member != nullptr)
	{
		if (Member->IsPlayerControlled())
		{
			return true;
		}
		if (Member->IsGrounded())
		{
			return true;
		}
		return false;
	}
	AFlowerCharacter* Flower = Cast<AFlowerCharacter>(Candidate);
	if (Flower != nullptr)
	{
		return true;
	}
	return false;
}
bool AMagicFloorDirectorCharacter::IsInBeam(AUnshackledCharacterBase* Candidate)
{
	FVector ToCandidate = Candidate->GetActorLocation() - GetActorLocation();
	ToCandidate.Z = 0.0f;
	float DistanceAlongBeam = FVector::DotProduct(ToCandidate, LaserDirection);
	if (DistanceAlongBeam < 0.0f)
	{
		return false;
	}
	if (DistanceAlongBeam > LaserLength)
	{
		return false;
	}
	FVector SidewaysOffset = ToCandidate - LaserDirection * DistanceAlongBeam;
	float SidewaysDistance = SidewaysOffset.Size();
	if (SidewaysDistance > LaserHalfWidth)
	{
		return false;
	}
	return true;
}
bool AMagicFloorDirectorCharacter::WasAlreadyHit(AUnshackledCharacterBase* Candidate)
{
	for (int32 Index = 0; Index < LaserHitTargets.Num(); Index++)
	{
		if (LaserHitTargets[Index] == Candidate)
		{
			return true;
		}
	}
	return false;
}
void AMagicFloorDirectorCharacter::ApplyLaserToVictims()
{
	for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
	{
		AUnshackledCharacterBase* Candidate = *It;
		if (CanBeLasered(Candidate))
		{
			if (IsInBeam(Candidate))
			{
				if (!WasAlreadyHit(Candidate))
				{
					LaserHitTargets.Add(Candidate);
					HitWithLaser(Candidate);
				}
			}
		}
	}
}
void AMagicFloorDirectorCharacter::HitWithLaser(AUnshackledCharacterBase* Victim)
{
	float MissingHealth = Victim->Health->MaxHealth - Victim->Health->CurrentHealth;
	float MissingDamage = MissingHealth * LaserMissingHealthFraction;
	float MinimumDamage = Victim->Health->MaxHealth * LaserMinMaxHealthFraction;
	float LaserDamage = MissingDamage;
	if (MinimumDamage > LaserDamage)
	{
		LaserDamage = MinimumDamage;
	}
	float DamageDealt = Victim->ApplyHit(LaserDamage, true, this);
	if (DamageDealt <= 0.0f)
	{
		return;
	}
	if (Victim->Health->IsDead())
	{
		return;
	}
	FStatModifier DefenseLoss;
	DefenseLoss.SourceId = FName("MagicDirectorLaser");
	DefenseLoss.Stat = ECharacterStat::Defense;
	DefenseLoss.Kind = EModifierKind::Percent;
	DefenseLoss.Amount = 0.0f - LaserDefenseLoss;
	DefenseLoss.Duration = LaserDefenseLossDuration;
	DefenseLoss.Rule = EStackRule::RefreshSameSource;
	Victim->Stats->AddModifier(DefenseLoss);
}
void AMagicFloorDirectorCharacter::TickSummon(float DeltaTime)
{
	if (SummonTimeLeft > 0.0f)
	{
		SummonTimeLeft = SummonTimeLeft - DeltaTime;
		return;
	}
	int32 ActiveSummons = CountActiveSummons();
	if (ActiveSummons + SummonCount > MaxSummoned)
	{
		return;
	}
	SummonPair();
	SummonTimeLeft = SummonInterval;
}
int32 AMagicFloorDirectorCharacter::CountActiveSummons()
{
	TArray<AEnemyCharacter*> StillAlive;
	for (int32 Index = 0; Index < SummonedEnemies.Num(); Index++)
	{
		AEnemyCharacter* Summoned = SummonedEnemies[Index];
		if (IsValid(Summoned))
		{
			if (!Summoned->Health->IsDead())
			{
				StillAlive.Add(Summoned);
			}
		}
	}
	SummonedEnemies = StillAlive;
	return SummonedEnemies.Num();
}
void AMagicFloorDirectorCharacter::SummonPair()
{
	TSubclassOf<AEnemyCharacter> ChosenClass = CasterClass;
	if (bSummonGuardsNext)
	{
		ChosenClass = GuardClass;
	}
	if (ChosenClass == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Magic director: summon class not set"));
		return;
	}
	bSummonGuardsNext = !bSummonGuardsNext;
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	for (int32 Index = 0; Index < SummonCount; Index++)
	{
		float AngleDegrees = 360.0f * Index / SummonCount;
		float AngleRadians = FMath::DegreesToRadians(AngleDegrees);
		FVector RingOffset = FVector(FMath::Cos(AngleRadians), FMath::Sin(AngleRadians), 0.0f) * SummonRingRadius;
		FVector SpawnLocation = GetActorLocation() + RingOffset;
		AEnemyCharacter* NewEnemy = GetWorld()->SpawnActor<AEnemyCharacter>(ChosenClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		if (NewEnemy != nullptr)
		{
			SummonedEnemies.Add(NewEnemy);
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Purple, TEXT("Magic director summons!"));
}
void AMagicFloorDirectorCharacter::TickMagicCircle(float DeltaTime)
{
	DrawDebugCircle(GetWorld(), GetActorLocation(), CircleRadius, 48, FColor::Purple, false, 0.0f, 0, 3.0f, FVector(1.0f, 0.0f, 0.0f), FVector(0.0f, 1.0f, 0.0f), false);
	CircleTimer = CircleTimer + DeltaTime;
	if (CircleTimer < CircleInterval)
	{
		return;
	}
	CircleTimer = 0.0f;
	for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
	{
		AUnshackledCharacterBase* Candidate = *It;
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
void AMagicFloorDirectorCharacter::ApplyCircleDebuff(AUnshackledCharacterBase* Ally)
{
	FStatModifier DefenseLoss;
	DefenseLoss.SourceId = FName("MagicDirectorCircle");
	DefenseLoss.Stat = ECharacterStat::Defense;
	DefenseLoss.Kind = EModifierKind::Percent;
	DefenseLoss.Amount = 0.0f - CircleDefenseLoss;
	DefenseLoss.Duration = CircleDuration;
	DefenseLoss.Rule = EStackRule::RefreshSameSource;
	Ally->Stats->AddModifier(DefenseLoss);
}
void AMagicFloorDirectorCharacter::TickRally(float DeltaTime)
{
	RallyTimer = RallyTimer + DeltaTime;
	if (RallyTimer < RallyInterval)
	{
		return;
	}
	RallyTimer = 0.0f;
	for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
	{
		AUnshackledCharacterBase* Candidate = *It;
		if (Candidate != this)
		{
			if (Candidate->bIsEnemy == bIsEnemy)
			{
				if (!Candidate->Health->IsDead())
				{
					float Distance = FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation());
					if (Distance <= RallyRadius)
					{
						ApplyRally(Candidate);
					}
				}
			}
		}
	}
}
void AMagicFloorDirectorCharacter::ApplyRally(AUnshackledCharacterBase* Ally)
{
	FStatModifier DefenseBonus;
	DefenseBonus.SourceId = FName("DirectorRally");
	DefenseBonus.Stat = ECharacterStat::Defense;
	DefenseBonus.Kind = EModifierKind::Percent;
	DefenseBonus.Amount = RallyDefenseBonus;
	DefenseBonus.Duration = RallyDuration;
	DefenseBonus.Rule = EStackRule::RefreshSameSource;
	Ally->Stats->AddModifier(DefenseBonus);
	FStatModifier DamageBonus;
	DamageBonus.SourceId = FName("DirectorRally");
	DamageBonus.Stat = ECharacterStat::Damage;
	DamageBonus.Kind = EModifierKind::Percent;
	DamageBonus.Amount = RallyDamageBonus;
	DamageBonus.Duration = RallyDuration;
	DamageBonus.Rule = EStackRule::RefreshSameSource;
	Ally->Stats->AddModifier(DamageBonus);
}
