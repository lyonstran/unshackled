#include "FloorDirectorCharacter.h"
#include "Characters/PartyMemberCharacter.h"
#include "Characters/FlowerCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
AFloorDirectorCharacter::AFloorDirectorCharacter()
{
        StatsRowName = FName("FloorDirector");
        AttackDamage = 25.0f;
        Health->MaxHealth = 300.0f;
        Stats->BaseDefense = 50.0f;
}
void AFloorDirectorCharacter::Tick(float DeltaTime)
{
        AUnshackledCharacterBase::Tick(DeltaTime);
        if (Health->IsDead())
        {
                return;
        }
        TickRally(DeltaTime);
        if (!Stats->CanAct())
        {
                return;
        }
        if (RamCooldownLeft > 0.0f)
        {
                RamCooldownLeft = RamCooldownLeft - DeltaTime;
        }
        if (State == EDirectorState::Chasing)
        {
                TickChasing(DeltaTime);
        }
        else if (State == EDirectorState::Winding)
        {
                TickWinding(DeltaTime);
        }
        else if (State == EDirectorState::Charging)
        {
                TickCharging(DeltaTime);
        }
        else
        {
                TickRecovering(DeltaTime);
        }
}
void AFloorDirectorCharacter::MoveToward(FVector Direction)
{
        FVector2D MoveInput = FVector2D(Direction.Y, Direction.X);
        MoveCharacter(MoveInput);
}
void AFloorDirectorCharacter::TickChasing(float DeltaTime)
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
        if (RamCooldownLeft <= 0.0f)
        {
                State = EDirectorState::Winding;
                WindupTimeLeft = RamWindupTime;
                return;
        }
        if (Distance > StopDistance)
        {
                MoveToward(Direction);
        }
}
void AFloorDirectorCharacter::TickWinding(float DeltaTime)
{
        AUnshackledCharacterBase* Target = FindClosestTarget();
        if (Target == nullptr)
        {
                State = EDirectorState::Chasing;
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
void AFloorDirectorCharacter::StartCharge(FVector Direction)
{
        ChargeDirection = Direction;
        ChargeDistanceLeft = RamDistance;
        State = EDirectorState::Charging;
}
void AFloorDirectorCharacter::TickCharging(float DeltaTime)
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
                HitVictim(Victim);
                EndCharge();
                return;
        }
        if (ChargeDistanceLeft <= 0.0f)
        {
                EndCharge();
        }
}
void AFloorDirectorCharacter::EndCharge()
{
        State = EDirectorState::Recovering;
        RecoverTimeLeft = RamRecoverTime;
        RamCooldownLeft = RamCooldown;
}
void AFloorDirectorCharacter::TickRecovering(float DeltaTime)
{
        RecoverTimeLeft = RecoverTimeLeft - DeltaTime;
        if (RecoverTimeLeft <= 0.0f)
        {
                State = EDirectorState::Chasing;
        }
}
bool AFloorDirectorCharacter::CanBeRammed(AUnshackledCharacterBase* Candidate)
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
AUnshackledCharacterBase* AFloorDirectorCharacter::FindRamVictim()
{
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (CanBeRammed(Candidate))
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
void AFloorDirectorCharacter::HitVictim(AUnshackledCharacterBase* Victim)
{
        float RamDamage = Victim->Health->MaxHealth * RamMaxHealthFraction;
        float DamageDealt = Victim->ApplyHit(RamDamage, true, this);
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Floor director rammed!"));
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
void AFloorDirectorCharacter::TickRally(float DeltaTime)
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
void AFloorDirectorCharacter::ApplyRally(AUnshackledCharacterBase* Ally)
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
