#include "SiriusCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
ASiriusCharacter::ASiriusCharacter()
{
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
        if (IsValid(ActiveVeil))
        {
                ActiveVeil->Destroy();
        }
        TSubclassOf<AVeilActor> ClassToSpawn = VeilClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = AVeilActor::StaticClass();
        }
        FVector SpawnLocation = GetActorLocation();
        FRotator SpawnRotation = FRotator::ZeroRotator;
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        ActiveVeil = GetWorld()->SpawnActor<AVeilActor>(ClassToSpawn, SpawnLocation, SpawnRotation, SpawnParameters);
        if (ActiveVeil != nullptr)
        {
                ActiveVeil->Setup(this);
        }
}