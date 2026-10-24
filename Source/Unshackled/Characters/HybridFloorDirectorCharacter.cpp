#include "HybridFloorDirectorCharacter.h"
#include "Characters/PartyMemberCharacter.h"
#include "Characters/FlowerCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
AHybridFloorDirectorCharacter::AHybridFloorDirectorCharacter()
{
	StatsRowName = FName("HybridDirector");
	AttackDamage = 25.0f;
	Health->MaxHealth = 350.0f;
	Stats->BaseDefense = 50.0f;
	SummonTimeLeft = FirstSummonDelay;
}
void AHybridFloorDirectorCharacter::Tick(float DeltaTime)
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
	TickSummon(DeltaTime);
	if (CycleCooldownLeft > 0.0f)
	{
		CycleCooldownLeft = CycleCooldownLeft - DeltaTime;
	}
	if (State == EHybridDirectorState::Chasing)
	{
		TickChasing(DeltaTime);
	}
	else if (State == EHybridDirectorState::RamWinding)
	{
		TickRamWinding(DeltaTime);
	}
	else if (State == EHybridDirectorState::Charging)
	{
		TickCharging(DeltaTime);
	}
	else if (State == EHybridDirectorState::Recovering)
	{
		TickRecovering(DeltaTime);
	}
	else if (State == EHybridDirectorState::BeamWinding)
	{
		TickBeamWinding(DeltaTime);
	}
	else
	{
		TickBeaming(DeltaTime);
	}
}
void AHybridFloorDirectorCharacter::MoveToward(FVector Direction)
{
	FVector2D MoveInput = FVector2D(Direction.Y, Direction.X);
	MoveCharacter(MoveInput);
}
void AHybridFloorDirectorCharacter::TickChasing(float DeltaTime)
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
	if (Distance > RamTriggerDistance)
	{
		MoveToward(Direction);
		return;
	}
	if (CycleCooldownLeft <= 0.0f)
	{
		State = EHybridDirectorState::RamWinding;
		WindupTimeLeft = RamWindupTime;
		return;
	}
	if (Distance > StopDistance)
	{
		MoveToward(Direction);
	}
}
void AHybridFloorDirectorCharacter::TickRamWinding(float DeltaTime)
{
	AUnshackledCharacterBase* Target = FindClosestTarget();
	if (Target == nullptr)
	{
		State = EHybridDirectorState::Chasing;
		return;
	}
	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	FVector Direction = ToTarget;
	Direction.Normalize();
	FacingDirection = Direction;
	FVector LineStart = GetActorLocation();
	FVector LineEnd = LineStart + (Direction * RamDistance);
	DrawDebugLine(GetWorld(), LineStart, LineEnd, FColor::Red, false, 0.0f, 0, 8.0f);
	WindupTimeLeft = WindupTimeLeft - DeltaTime;
	if (WindupTimeLeft <= 0.0f)
	{
		StartCharge(Direction);
	}
}
void AHybridFloorDirectorCharacter::StartCharge(FVector Direction)
{
	ChargeDirection = Direction;
	ChargeDistanceLeft = RamDistance;
	State = EHybridDirectorState::Charging;
}
void AHybridFloorDirectorCharacter::TickCharging(float DeltaTime)
{
	float StepLength = RamSpeed * DeltaTime;
	if (StepLength > ChargeDistanceLeft)
	{
		StepLength = ChargeDistanceLeft;
	}
	FVector NewLocation = GetActorLocation() + (ChargeDirection * StepLength);
	SetActorLocation(NewLocation);
	ChargeDistanceLeft = ChargeDistanceLeft - StepLength;
	AUnshackledCharacterBase* Victim = FindRamVictim();
	if (Victim != nullptr)
	{
		HitWithRam(Victim);
		EndCharge();
		return;
	}
	if (ChargeDistanceLeft <= 0.0f)
	{
		EndCharge();
	}
}
void AHybridFloorDirectorCharacter::EndCharge()
{
	State = EHybridDirectorState::Recovering;
	RecoverTimeLeft = RamRecoverTime;
}
void AHybridFloorDirectorCharacter::TickRecovering(float DeltaTime)
{
	RecoverTimeLeft = RecoverTimeLeft - DeltaTime;
	if (RecoverTimeLeft <= 0.0f)
	{
		StartBeamWindup();
	}
}
void AHybridFloorDirectorCharacter::StartBeamWindup()
{
	BeamTarget = FindClosestTarget();
	if (BeamTarget == nullptr)
	{
		EndCycle();
		return;
	}
	WindupTimeLeft = BeamWindupTime;
	State = EHybridDirectorState::BeamWinding;
}
bool AHybridFloorDirectorCharacter::EnsureBeamTarget()
{
	bool bNeedsNewTarget = false;
	if (!IsValid(BeamTarget))
	{
		bNeedsNewTarget = true;
	}
	else if (BeamTarget->Health->IsDead())
	{
		bNeedsNewTarget = true;
	}
	if (bNeedsNewTarget)
	{
		BeamTarget = FindClosestTarget();
	}
	if (BeamTarget == nullptr)
	{
		return false;
	}
	return true;
}
void AHybridFloorDirectorCharacter::TickBeamWinding(float DeltaTime)
{
	if (!EnsureBeamTarget())
	{
		EndCycle();
		return;
	}
	FVector ToTarget = BeamTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	FVector Direction = ToTarget;
	Direction.Normalize();
	BeamDirection = Direction;
	FacingDirection = Direction;
	FVector LineStart = GetActorLocation();
	FVector LineEnd = LineStart + (BeamDirection * BeamLength);
	DrawDebugLine(GetWorld(), LineStart, LineEnd, FColor::Orange, false, 0.0f, 0, 4.0f);
	WindupTimeLeft = WindupTimeLeft - DeltaTime;
	if (WindupTimeLeft <= 0.0f)
	{
		StartBeam();
	}
}
void AHybridFloorDirectorCharacter::StartBeam()
{
	BeamTimeLeft = BeamDuration;
	BeamTickTimer = 0.0f;
	State = EHybridDirectorState::Beaming;
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Purple, TEXT("Hybrid director beam!"));
}
void AHybridFloorDirectorCharacter::TurnBeamToward(AUnshackledCharacterBase* Target, float DeltaTime)
{
	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	float TargetYaw = FMath::RadiansToDegrees(FMath::Atan2(ToTarget.Y, ToTarget.X));
	float CurrentYaw = FMath::RadiansToDegrees(FMath::Atan2(BeamDirection.Y, BeamDirection.X));
	float YawDifference = FMath::FindDeltaAngleDegrees(CurrentYaw, TargetYaw);
	float MaxTurn = BeamTurnSpeed * DeltaTime;
	float YawStep = FMath::Clamp(YawDifference, 0.0f - MaxTurn, MaxTurn);
	float NewYawRadians = FMath::DegreesToRadians(CurrentYaw + YawStep);
	BeamDirection = FVector(FMath::Cos(NewYawRadians), FMath::Sin(NewYawRadians), 0.0f);
	FacingDirection = BeamDirection;
}
void AHybridFloorDirectorCharacter::TickBeaming(float DeltaTime)
{
	if (EnsureBeamTarget())
	{
		TurnBeamToward(BeamTarget, DeltaTime);
	}
	FVector LineStart = GetActorLocation();
	FVector LineEnd = LineStart + (BeamDirection * BeamLength);
	DrawDebugLine(GetWorld(), LineStart, LineEnd, FColor::Purple, false, 0.0f, 0, 14.0f);
	BeamTickTimer = BeamTickTimer + DeltaTime;
	if (BeamTickTimer >= BeamTickInterval)
	{
		DamageVictimsInBeam(BeamTickTimer);
		BeamTickTimer = 0.0f;
	}
	BeamTimeLeft = BeamTimeLeft - DeltaTime;
	if (BeamTimeLeft <= 0.0f)
	{
		EndCycle();
	}
}
void AHybridFloorDirectorCharacter::EndCycle()
{
	State = EHybridDirectorState::Chasing;
	CycleCooldownLeft = CycleCooldown;
}
bool AHybridFloorDirectorCharacter::CanBeHit(AUnshackledCharacterBase* Candidate)
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
AUnshackledCharacterBase* AHybridFloorDirectorCharacter::FindRamVictim()
{
	for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
	{
		AUnshackledCharacterBase* Candidate = *It;
		if (CanBeHit(Candidate))
		{
			float Distance = FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation());
			if (Distance <= RamHitRadius)
			{
				return Candidate;
			}
		}
	}
	return nullptr;
}
void AHybridFloorDirectorCharacter::HitWithRam(AUnshackledCharacterBase* Victim)
{
	float RamDamage = Victim->Health->MaxHealth * RamMaxHealthFraction;
	float DamageDealt = Victim->ApplyHit(RamDamage, true, this);
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Hybrid director rammed!"));
	if (DamageDealt <= 0.0f)
	{
		return;
	}
	if (Victim->Health->IsDead())
	{
		return;
	}
	FStatModifier RamStun;
	RamStun.SourceId = FName("DirectorRamStun");
	RamStun.Stat = ECharacterStat::Speed;
	RamStun.Kind = EModifierKind::Flat;
	RamStun.Amount = 0.0f;
	RamStun.Duration = RamStunDuration;
	RamStun.Rule = EStackRule::RefreshSameSource;
	RamStun.Flag = EStatusFlag::Stun;
	Victim->Stats->AddModifier(RamStun);
}
bool AHybridFloorDirectorCharacter::IsInBeam(AUnshackledCharacterBase* Candidate)
{
	FVector ToCandidate = Candidate->GetActorLocation() - GetActorLocation();
	ToCandidate.Z = 0.0f;
	float DistanceAlongBeam = FVector::DotProduct(ToCandidate, BeamDirection);
	if (DistanceAlongBeam < 0.0f)
	{
		return false;
	}
	if (DistanceAlongBeam > BeamLength)
	{
		return false;
	}
	FVector SidewaysOffset = ToCandidate - BeamDirection * DistanceAlongBeam;
	float SidewaysDistance = SidewaysOffset.Size();
	if (SidewaysDistance > BeamHalfWidth)
	{
		return false;
	}
	return true;
}
void AHybridFloorDirectorCharacter::DamageVictimsInBeam(float ElapsedTime)
{
	for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
	{
		AUnshackledCharacterBase* Candidate = *It;
		if (CanBeHit(Candidate))
		{
			if (IsInBeam(Candidate))
			{
				HitWithBeam(Candidate, ElapsedTime);
			}
		}
	}
}
void AHybridFloorDirectorCharacter::HitWithBeam(AUnshackledCharacterBase* Victim, float ElapsedTime)
{
	float FullBeamDamage = Victim->Health->MaxHealth * BeamMaxHealthFraction;
	float TickDamage = FullBeamDamage * ElapsedTime / BeamDuration;
	float DamageDealt = Victim->ApplyHit(TickDamage, true, this);
	if (DamageDealt <= 0.0f)
	{
		return;
	}
	if (Victim->Health->IsDead())
	{
		return;
	}
	FStatModifier BeamSlow;
	BeamSlow.SourceId = FName("HybridDirectorBeamSlow");
	BeamSlow.Stat = ECharacterStat::Speed;
	BeamSlow.Kind = EModifierKind::Percent;
	BeamSlow.Amount = 0.0f - BeamSlowFraction;
	BeamSlow.Duration = BeamSlowDuration;
	BeamSlow.Rule = EStackRule::RefreshSameSource;
	Victim->Stats->AddModifier(BeamSlow);
}
void AHybridFloorDirectorCharacter::TickSummon(float DeltaTime)
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
int32 AHybridFloorDirectorCharacter::CountActiveSummons()
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
void AHybridFloorDirectorCharacter::SummonPair()
{
	TSubclassOf<AEnemyCharacter> ChosenClass = CasterClass;
	if (bSummonGuardsNext)
	{
		ChosenClass = GuardClass;
	}
	if (ChosenClass == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Hybrid director: summon class not set"));
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
	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Purple, TEXT("Hybrid director summons!"));
}
void AHybridFloorDirectorCharacter::TickMagicCircle(float DeltaTime)
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
void AHybridFloorDirectorCharacter::ApplyCircleDebuff(AUnshackledCharacterBase* Ally)
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
void AHybridFloorDirectorCharacter::TickRally(float DeltaTime)
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
void AHybridFloorDirectorCharacter::ApplyRally(AUnshackledCharacterBase* Ally)
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
