#include "UnshackledCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "Engine/Engine.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Game/UnshackledGameMode.h"
#include "Stats/CharacterStatsRow.h"
AUnshackledCharacterBase::AUnshackledCharacterBase()
{
        PrimaryActorTick.bCanEverTick = true;
        bUseControllerRotationYaw = false;
        GetCharacterMovement()->bOrientRotationToMovement = false;
        GetCharacterMovement()->BrakingDecelerationFlying = 1000.0f;
        GetCharacterMovement()->MaxAcceleration = 3500.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 3000.0f;
        GetSprite()->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
        Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
        Stats = CreateDefaultSubobject<UStatsComponent>(TEXT("Stats"));
}
void AUnshackledCharacterBase::BeginPlay()
{
        Super::BeginPlay();
        ApplyStatsRow();
        Stats->BaseMaxHealth = Health->MaxHealth;
        Stats->BaseDamage = AttackDamage;
}
void AUnshackledCharacterBase::ApplyStatsRow()
{
        if (StatsRowName.IsNone())
        {
                return;
        }
        UWorld* World = GetWorld();
        if (World == nullptr)
        {
                return;
        }
        AUnshackledGameMode* GameMode = World->GetAuthGameMode<AUnshackledGameMode>();
        if (GameMode == nullptr)
        {
                return;
        }
        if (GameMode->CharacterStatsTable == nullptr)
        {
                return;
        }
        FCharacterStatsRow* Row = GameMode->CharacterStatsTable->FindRow<FCharacterStatsRow>(StatsRowName, TEXT("ApplyStatsRow"));
        if (Row == nullptr)
        {
                UE_LOG(LogTemp, Warning, TEXT("No stats row named %s in the character stats table"), *StatsRowName.ToString());
                return;
        }
        Health->MaxHealth = Row->MaxHealth;
        Health->CurrentHealth = Row->MaxHealth;
        AttackDamage = Row->Damage;
        Stats->BaseDefense = Row->Defense;
        Stats->BaseSpeed = Row->Speed;
        Stats->BaseMaxMana = Row->MaxMana;
        ManaRegenPerSecond = Row->ManaRegenPerSecond;
        AttackCooldown = Row->AttackCooldown;
        DashCooldown = Row->DashCooldown;
        SpecialManaCost = Row->SpecialManaCost;
        SpecialCooldown = Row->SpecialCooldown;
        SpecialWindup = Row->SpecialWindup;
        UltimateManaCost = Row->UltimateManaCost;
        UltimateCooldown = Row->UltimateCooldown;
        UltimateWindup = Row->UltimateWindup;
        if (Stats->bHasMana)
        {
                Stats->CurrentMana = Stats->GetFinal(ECharacterStat::MaxMana);
        }
}
void AUnshackledCharacterBase::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        ApplyStatEffects();
        RegenerateMana(DeltaTime);
}
void AUnshackledCharacterBase::ApplyStatEffects()
{
        float FinalSpeed = Stats->GetFinal(ECharacterStat::Speed);
        GetCharacterMovement()->MaxFlySpeed = FinalSpeed;
        float FinalMaxHealth = Stats->GetFinal(ECharacterStat::MaxHealth);
        Health->SetMaxHealthFromStats(FinalMaxHealth);
}
void AUnshackledCharacterBase::RegenerateMana(float DeltaTime)
{
        if (!Stats->bHasMana)
        {
                return;
        }
        if (Health->IsDead())
        {
                return;
        }
        float ManaGained = ManaRegenPerSecond * DeltaTime;
        Stats->RestoreMana(ManaGained);
}
FVector AUnshackledCharacterBase::GetFacingDirection() const
{
        return FacingDirection;
}
void AUnshackledCharacterBase::MoveCharacter(FVector2D Direction)
{
        if (!Direction.IsNearlyZero())
        {
                FVector NewFacing = FVector(Direction.Y, Direction.X, 0.0f);
                NewFacing.Normalize();
                FacingDirection = NewFacing;
        }
        if (!Stats->CanMove())
        {
                return;
        }
        AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Direction.Y);
        AddMovementInput(FVector(0.0f, 1.0f, 0.0f), Direction.X);
}
float AUnshackledCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser)
{
        AUnshackledCharacterBase* Attacker = Cast<AUnshackledCharacterBase>(DamageCauser);
        return ApplyHit(DamageAmount, false, Attacker);
}
float AUnshackledCharacterBase::ApplyHit(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker)
{
        if (Health->IsDead())
        {
                return 0.0f;
        }
        if (bInvulnerable)
        {
                return 0.0f;
        }
        float FinalDamage = CalculateFinalDamage(RawDamage, bTrueDamage);
        Stats->ConsumeHit();
        float HealthBefore = Health->CurrentHealth;
        Health->TakeHit(FinalDamage);
        float HealthAfter = Health->CurrentHealth;
        float DamageDealt = HealthBefore - HealthAfter;
        CheckForDeath(Attacker);
        if (Attacker != nullptr)
        {
                Attacker->OnDealtDamage(this, DamageDealt);
        }
        return DamageDealt;
}
float AUnshackledCharacterBase::CalculateFinalDamage(float RawDamage, bool bTrueDamage)
{
        float FinalDamage = RawDamage;
        if (!bTrueDamage)
        {
                float DefenseValue = Stats->GetFinal(ECharacterStat::Defense);
                float DefenseScale = Stats->DefenseScale;
                float Mitigation = DefenseScale / (DefenseScale + DefenseValue);
                FinalDamage = FinalDamage * Mitigation;
        }
        float IncomingMultiplier = Stats->GetFinal(ECharacterStat::IncomingDamage);
        FinalDamage = FinalDamage * IncomingMultiplier;
        return FinalDamage;
}
void AUnshackledCharacterBase::CheckForDeath(AUnshackledCharacterBase* Attacker)
{
        if (!Health->IsDead())
        {
                return;
        }
        bool bDeathCancelled = WouldDie();
        if (bDeathCancelled)
        {
                return;
        }
        Die();
        if (Attacker != nullptr)
        {
                Attacker->OnKilledTarget(this);
        }
}
void AUnshackledCharacterBase::DealDamage(AUnshackledCharacterBase* Target, float AbilityScale, bool bTrueDamage)
{
        if (Target == nullptr)
        {
                return;
        }
        float DamageStat = Stats->GetFinal(ECharacterStat::Damage);
        float RawDamage = DamageStat * AbilityScale;
        Target->ApplyHit(RawDamage, bTrueDamage, this);
}
void AUnshackledCharacterBase::OnDealtDamage(AUnshackledCharacterBase* Target, float DamageDealt)
{
        float LifestealFraction = Stats->GetFinal(ECharacterStat::Lifesteal);
        if (LifestealFraction > 0.0f)
        {
                float HealthToGain = DamageDealt * LifestealFraction;
                Health->Heal(HealthToGain);
        }
}
void AUnshackledCharacterBase::OnKilledTarget(AUnshackledCharacterBase* Target)
{
}
bool AUnshackledCharacterBase::WouldDie()
{
        return false;
}
void AUnshackledCharacterBase::Die()
{
        GetCharacterMovement()->DisableMovement();
        SetActorEnableCollision(false);
}
void AUnshackledCharacterBase::BasicAttack()
{
        AttackInDirection(FacingDirection);
}
void AUnshackledCharacterBase::AttackInDirection(FVector Direction)
{
        Direction.Z = 0.0f;
        Direction.Normalize();
        if (!bCanAttack)
        {
                return;
        }
        if (Health->IsDead())
        {
                return;
        }
        if (!Stats->CanAct())
        {
                return;
        }
        bCanAttack = false;
        GetWorldTimerManager().SetTimer(AttackTimer, this, &AUnshackledCharacterBase::ResetAttack, AttackCooldown, false);
        PerformBasicAttack(Direction);
}
void AUnshackledCharacterBase::PerformBasicAttack(FVector Direction)
{
        FVector SphereCenter = GetActorLocation() + (Direction * AttackRange);
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRadius);
        GetWorld()->OverlapMultiByObjectType(OverlapResults, SphereCenter, FQuat::Identity, ObjectParams, Sphere, QueryParams);
        TArray<AActor*> ActorsAlreadyHit;
        for (int32 i = 0; i < OverlapResults.Num(); i++)
        {
                AActor* HitActor = OverlapResults[i].GetActor();
                if (HitActor == nullptr)
                {
                        continue;
                }
                if (ActorsAlreadyHit.Contains(HitActor))
                {
                        continue;
                }
                ActorsAlreadyHit.Add(HitActor);
                AUnshackledCharacterBase* OtherCharacter = Cast<AUnshackledCharacterBase>(HitActor);
                if (OtherCharacter == nullptr)
                {
                        continue;
                }
                if (OtherCharacter->bIsEnemy == bIsEnemy)
                {
                        continue;
                }
                DealDamage(OtherCharacter, 1.0f, false);
        }
        DrawDebugSphere(GetWorld(), SphereCenter, AttackRadius, 12, FColor::Red, false, 0.2f);
}
void AUnshackledCharacterBase::ResetAttack()
{
        bCanAttack = true;
}
void AUnshackledCharacterBase::Dash()
{
        if (!bCanDash)
        {
                return;
        }
        if (Health->IsDead())
        {
                return;
        }
        if (!Stats->CanMove())
        {
                return;
        }
        bCanDash = false;
        bInvulnerable = true;
        FVector DashVelocity = FacingDirection * DashSpeed;
        LaunchCharacter(DashVelocity, true, true);
        GetWorldTimerManager().SetTimer(DashDurationTimer, this, &AUnshackledCharacterBase::EndDash, DashDuration, false);
        GetWorldTimerManager().SetTimer(DashImmunityTimer, this,
                &AUnshackledCharacterBase::EndInvulnerability, DashImmunityTime, false);
        GetWorldTimerManager().SetTimer(DashCooldownTimer,
                this, &AUnshackledCharacterBase::ResetDash, DashCooldown, false);
}
void AUnshackledCharacterBase::EndDash()
{
        GetCharacterMovement()->SetMovementMode(MOVE_Flying);
        GetCharacterMovement()->Velocity = FVector::ZeroVector;
}
void AUnshackledCharacterBase::EndInvulnerability()
{
        bInvulnerable = false;
}
void AUnshackledCharacterBase::ResetDash()
{
        bCanDash = true;
}
void AUnshackledCharacterBase::ReviveCharacter()
{
        if (!Health->IsDead())
        {
                return;
        }
        Health->Revive();
        SetActorEnableCollision(true);
        GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}
bool AUnshackledCharacterBase::TryPayMana(float ManaCost)
{
        if (ManaCost <= 0.0f)
        {
                return true;
        }
        bool bPaid = Stats->SpendMana(ManaCost);
        if (!bPaid)
        {
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, TEXT("Not enough mana"));
        }
        return bPaid;
}
FVector AUnshackledCharacterBase::CleanAimDirection(FVector AimDirection)
{
        AimDirection.Z = 0.0f;
        if (AimDirection.IsNearlyZero())
        {
                return FacingDirection;
        }
        AimDirection.Normalize();
        return AimDirection;
}
void AUnshackledCharacterBase::UseSpecial(FVector AimDirection)
{
        if (!bCanUseSpecial)
        {
                return;
        }
        if (Health->IsDead())
        {
                return;
        }
        if (!Stats->CanAct())
        {
                return;
        }
        bool bPaid = TryPayMana(SpecialManaCost);
        if (!bPaid)
        {
                return;
        }
        if (SpecialCooldown > 0.0f)
        {
                bCanUseSpecial = false;
                GetWorldTimerManager().SetTimer(SpecialCooldownTimer, this,
                        &AUnshackledCharacterBase::ResetSpecial, SpecialCooldown, false);
        }
        PendingSpecialAim = CleanAimDirection(AimDirection);
        if (SpecialWindup > 0.0f)
        {
                GetWorldTimerManager().SetTimer(SpecialWindupTimer, this,
                        &AUnshackledCharacterBase::FinishSpecialWindup, SpecialWindup, false);
        }
        else
        {
                FinishSpecialWindup();
        }
}
void AUnshackledCharacterBase::FinishSpecialWindup()
{
        if (Health->IsDead())
        {
                return;
        }
        if (!Stats->CanAct())
        {
                return;
        }
        PerformSpecial(PendingSpecialAim);
}
void AUnshackledCharacterBase::ResetSpecial()
{
        bCanUseSpecial = true;
}
void AUnshackledCharacterBase::UseUltimate(FVector AimDirection)
{
        if (!bCanUseUltimate)
        {
                return;
        }
        if (Health->IsDead())
        {
                return;
        }
        if (!Stats->CanAct())
        {
                return;
        }
        bool bPaid = TryPayMana(UltimateManaCost);
        if (!bPaid)
        {
                return;
        }
        if (UltimateCooldown > 0.0f)
        {
                bCanUseUltimate = false;
                GetWorldTimerManager().SetTimer(UltimateCooldownTimer, this,
                        &AUnshackledCharacterBase::ResetUltimate, UltimateCooldown, false);
        }
        PendingUltimateAim = CleanAimDirection(AimDirection);
        if (UltimateWindup > 0.0f)
        {
                GetWorldTimerManager().SetTimer(UltimateWindupTimer, this,
                        &AUnshackledCharacterBase::FinishUltimateWindup, UltimateWindup, false);
        }
        else
        {
                FinishUltimateWindup();
        }
}
void AUnshackledCharacterBase::FinishUltimateWindup()
{
        if (Health->IsDead())
        {
                return;
        }
        if (!Stats->CanAct())
        {
                return;
        }
        PerformUltimate(PendingUltimateAim);
}
void AUnshackledCharacterBase::ResetUltimate()
{
        bCanUseUltimate = true;
}
void AUnshackledCharacterBase::PerformSpecial(FVector AimDirection)
{
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Cyan, TEXT("Special used (placeholder)"));
}
void AUnshackledCharacterBase::PerformUltimate(FVector AimDirection)
{
        GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Magenta, TEXT("Ultimate used (placeholder)"));
}
