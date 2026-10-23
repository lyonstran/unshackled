#include "SiriusCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
ASiriusCharacter::ASiriusCharacter()
{
        StatsRowName = FName("Sirius");
}
void ASiriusCharacter::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        TickRetribution(DeltaTime);
}
void ASiriusCharacter::OnDealtDamage(AUnshackledCharacterBase* Target, float DamageDealt)
{
        Super::OnDealtDamage(Target, DamageDealt);
        TryExecute(Target);
}
void ASiriusCharacter::OnSwappedOut(APartyMemberCharacter* NewLeader)
{
        Super::OnSwappedOut(NewLeader);
        if (NewLeader == nullptr)
        {
                return;
        }
        if (Health->IsDead())
        {
                return;
        }
        GiveSwapVeil(NewLeader);
}
void ASiriusCharacter::GiveSwapVeil(APartyMemberCharacter* Ally)
{
        FStatModifier SwapVeil;
        SwapVeil.SourceId = FName("SiriusSwapVeil");
        SwapVeil.Stat = ECharacterStat::IncomingDamage;
        SwapVeil.Kind = EModifierKind::Percent;
        SwapVeil.Amount = 0.0f - SwapVeilReduction;
        SwapVeil.Duration = SwapVeilDuration;
        SwapVeil.Rule = EStackRule::ConsumeOnHit;
        SwapVeil.HitsRemaining = SwapVeilHits;
        Ally->Stats->AddModifier(SwapVeil);
        FString Message = FString::Printf(TEXT("Swap veil on %s"), *Ally->DisplayName);
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, Message);
}
void ASiriusCharacter::TryExecute(AUnshackledCharacterBase* Target)
{
        if (Target == nullptr)
        {
                return;
        }
        if (Target->bIsEnemy == bIsEnemy)
        {
                return;
        }
        if (Target->Health->IsDead())
        {
                return;
        }
        float MaxHealthValue = Target->Health->MaxHealth;
        if (MaxHealthValue > 0.0f)
        {
                float HealthFraction = Target->Health->CurrentHealth / MaxHealthValue;
                if (HealthFraction < ExecuteThreshold)
                {
                        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Executed!"));
                        Target->ApplyHit(ExecuteDamage, true, this);
                }
        }
}
void ASiriusCharacter::PerformRetribution()
{
        Super::PerformRetribution();
        RetributionTimeLeft = RetributionDuration;
        RetributionAuraTimer = 0.0f;
        FStatModifier LifestealModifier;
        LifestealModifier.SourceId = FName("SiriusRetribution");
        LifestealModifier.Stat = ECharacterStat::Lifesteal;
        LifestealModifier.Kind = EModifierKind::Flat;
        LifestealModifier.Amount = RetributionLifesteal;
        LifestealModifier.Duration = RetributionDuration;
        LifestealModifier.Rule = EStackRule::RefreshSameSource;
        Stats->AddModifier(LifestealModifier);
        ApplyRetributionToEnemies();
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Sirius Retribution"));
}
void ASiriusCharacter::TickRetribution(float DeltaTime)
{
        if (RetributionTimeLeft <= 0.0f)
        {
                return;
        }
        if (Health->IsDead())
        {
                RetributionTimeLeft = 0.0f;
                return;
        }
        RetributionTimeLeft = RetributionTimeLeft - DeltaTime;
        RetributionAuraTimer = RetributionAuraTimer + DeltaTime;
        if (RetributionAuraTimer >= RetributionAuraInterval)
        {
                RetributionAuraTimer = 0.0f;
                ApplyRetributionToEnemies();
        }
}
void ASiriusCharacter::ApplyRetributionToEnemies()
{
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (Candidate->bIsEnemy != bIsEnemy)
                {
                        if (!Candidate->Health->IsDead())
                        {
                                FStatModifier DefenseModifier;
                                DefenseModifier.SourceId = FName("SiriusRetribution");
                                DefenseModifier.Stat = ECharacterStat::Defense;
                                DefenseModifier.Kind = EModifierKind::Percent;
                                DefenseModifier.Amount = 0.0f - RetributionDefenseReduction;
                                DefenseModifier.Duration = RetributionAuraDuration;
                                DefenseModifier.Rule = EStackRule::RefreshSameSource;
                                Candidate->Stats->AddModifier(DefenseModifier);
                                FStatModifier SlowModifier;
                                SlowModifier.SourceId = FName("SiriusRetribution");
                                SlowModifier.Stat = ECharacterStat::Speed;
                                SlowModifier.Kind = EModifierKind::Percent;
                                SlowModifier.Amount = 0.0f - RetributionSlow;
                                SlowModifier.Duration = RetributionAuraDuration;
                                SlowModifier.Rule = EStackRule::RefreshSameSource;
                                Candidate->Stats->AddModifier(SlowModifier);
                        }
                }
        }
}
void ASiriusCharacter::GroundedTick(float DeltaTime)
{
        Super::GroundedTick(DeltaTime);
        if (!Stats->CanAct())
        {
                return;
        }
        AirBladeTimeLeft = AirBladeTimeLeft - DeltaTime;
        if (AirBladeTimeLeft > 0.0f)
        {
                return;
        }
        AirBladeTimeLeft = 0.0f;
        AUnshackledCharacterBase* Target = FindNearestEnemy();
        if (Target == nullptr)
        {
                return;
        }
        FireAirBlade(Target);
        AirBladeTimeLeft = AirBladeInterval;
}
AUnshackledCharacterBase* ASiriusCharacter::FindNearestEnemy()
{
        AUnshackledCharacterBase* NearestEnemy = nullptr;
        float NearestDistance = AirBladeSearchRange;
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (Candidate->bIsEnemy != bIsEnemy)
                {
                        if (!Candidate->Health->IsDead())
                        {
                                float Distance = FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation());
                                if (Distance <= NearestDistance)
                                {
                                        NearestDistance = Distance;
                                        NearestEnemy = Candidate;
                                }
                        }
                }
        }
        return NearestEnemy;
}
void ASiriusCharacter::FireAirBlade(AUnshackledCharacterBase* Target)
{
        FVector Direction = Target->GetActorLocation() - GetActorLocation();
        Direction.Z = 0.0f;
        Direction.Normalize();
        TSubclassOf<AAbilityProjectile> ClassToSpawn = AirBladeClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = AAbilityProjectile::StaticClass();
        }
        FVector SpawnLocation = GetActorLocation() + (Direction * AirBladeSpawnOffset);
        FRotator SpawnRotation = Direction.Rotation();
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AAbilityProjectile* Blade = GetWorld()->SpawnActor<AAbilityProjectile>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
        if (Blade != nullptr)
        {
                Blade->Speed = AirBladeSpeed;
                Blade->MaxRange = AirBladeRange;
                Blade->Radius = AirBladeRadius;
                Blade->AllySlowFraction = AirBladeAllySlow;
                Blade->AllySlowDuration = AirBladeAllySlowDuration;
                Blade->Launch(this, Direction, AirBladeDamageScale, 1.0f);
        }
}
void ASiriusCharacter::PerformBasicAttack(FVector Direction)
{
        if (EmpoweredAttacksLeft > 0)
        {
                FireCrescent(Direction, EmpoweredDamageScale, EmpoweredSizeScale);
                EmpoweredAttacksLeft = EmpoweredAttacksLeft - 1;
                FString Message = FString::Printf(TEXT("Empowered attacks left: %d"), EmpoweredAttacksLeft);
                GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, Message);
                if (EmpoweredAttacksLeft == 0)
                {
                        SpawnTornado(Direction);
                }
        }
        else
        {
                FireCrescent(Direction, 1.0f, 1.0f);
        }
}
void ASiriusCharacter::PerformSpecial(FVector AimDirection)
{
        EmpoweredAttacksLeft = EmpoweredAttackCount;
        FString Message = FString::Printf(TEXT("Empowered attacks: %d"), EmpoweredAttacksLeft);
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, Message);
}
void ASiriusCharacter::PerformUltimate(FVector AimDirection)
{
        SpawnVeil();
}
void ASiriusCharacter::FireCrescent(FVector Direction, float DamageScale, float SizeScale)
{
        TSubclassOf<AAbilityProjectile> ClassToSpawn = CrescentClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = AAbilityProjectile::StaticClass();
        }
        FVector SpawnLocation = GetActorLocation() + (Direction * CrescentSpawnOffset);
        FRotator SpawnRotation = Direction.Rotation();
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AAbilityProjectile* Crescent = GetWorld()->SpawnActor<AAbilityProjectile>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
        if (Crescent != nullptr)
        {
                Crescent->Launch(this, Direction, DamageScale, SizeScale);
        }
}
void ASiriusCharacter::SpawnTornado(FVector Direction)
{
        TSubclassOf<ATornadoActor> ClassToSpawn = TornadoClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = ATornadoActor::StaticClass();
        }
        FVector SpawnLocation = GetActorLocation() + (Direction * TornadoSpawnOffset);
        FRotator SpawnRotation = FRotator::ZeroRotator;
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        ATornadoActor* Tornado = GetWorld()->SpawnActor<ATornadoActor>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
        if (Tornado != nullptr)
        {
                Tornado->Setup(this);
        }
}
void ASiriusCharacter::SpawnVeil()
{
        DestroyOldVeils();
        ActiveVeil = SpawnVeilOn(this);
        SpawnVeilsOnGroundedAllies();
}
void ASiriusCharacter::DestroyOldVeils()
{
        if (IsValid(ActiveVeil))
        {
                ActiveVeil->Destroy();
        }
        for (int32 i = 0; i < AllyVeils.Num(); i++)
        {
                if (IsValid(AllyVeils[i]))
                {
                        AllyVeils[i]->Destroy();
                }
        }
        AllyVeils.Empty();
}
AVeilActor* ASiriusCharacter::SpawnVeilOn(AUnshackledCharacterBase* VeilOwner)
{
        TSubclassOf<AVeilActor> ClassToSpawn = VeilClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = AVeilActor::StaticClass();
        }
        FVector SpawnLocation = VeilOwner->GetActorLocation();
        FRotator SpawnRotation = FRotator::ZeroRotator;
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AVeilActor* NewVeil = GetWorld()->SpawnActor<AVeilActor>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
        if (NewVeil != nullptr)
        {
                NewVeil->Setup(VeilOwner);
        }
        return NewVeil;
}
void ASiriusCharacter::SpawnVeilsOnGroundedAllies()
{
        for (TActorIterator<APartyMemberCharacter> It(GetWorld()); It; ++It)
        {
                APartyMemberCharacter* Ally = *It;
                if (Ally != this)
                {
                        if (Ally->IsGrounded())
                        {
                                if (!Ally->Health->IsDead())
                                {
                                        AVeilActor* AllyVeil = SpawnVeilOn(Ally);
                                        if (AllyVeil != nullptr)
                                        {
                                                AllyVeils.Add(AllyVeil);
                                        }
                                }
                        }
                }
        }
}
