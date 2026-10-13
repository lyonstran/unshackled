#include "UnshackledCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "Engine/Engine.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

AUnshackledCharacterBase::AUnshackledCharacterBase()
{
        // We tick every frame so stat effects (slows, buffs) reach the movement and health components.
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

        // Stats start from the numbers already set in the Blueprint
        Stats->BaseMaxHealth = Health->MaxHealth;
        Stats->BaseDamage = AttackDamage;
}

void AUnshackledCharacterBase::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);

        ApplyStatEffects();
}

// Pushes the stat results onto the movement and health components every frame.
void AUnshackledCharacterBase::ApplyStatEffects()
{
        // Flying movement uses MaxFlySpeed (MaxWalkSpeed is ignored while flying).
        float FinalSpeed = Stats->GetFinal(ECharacterStat::Speed);
        GetCharacterMovement()->MaxFlySpeed = FinalSpeed;

        // If a buff changed our max health, let the health component adjust.
        float FinalMaxHealth = Stats->GetFinal(ECharacterStat::MaxHealth);
        Health->SetMaxHealthFromStats(FinalMaxHealth);
}

void AUnshackledCharacterBase::MoveCharacter(FVector2D Direction)
{
        // Remember which way we're facing so attacks and dashes know where to go.
        // Direction.Y moves along world X and Direction.X moves along world Y.
        if (!Direction.IsNearlyZero())
        {
                FVector NewFacing = FVector(Direction.Y, Direction.X, 0.0f);
                NewFacing.Normalize();
                FacingDirection = NewFacing;
        }

        // Rooted, stunned, or frozen characters can turn but not walk.
        if (!Stats->CanMove())
        {
                return;
        }

        AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Direction.Y);
        AddMovementInput(FVector(0.0f, 1.0f, 0.0f), Direction.X);
}

// Anything that still uses ApplyDamage ends up here. The causer is the attacker.
float AUnshackledCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser)
{
        AUnshackledCharacterBase* Attacker = Cast<AUnshackledCharacterBase>(DamageCauser);
        return ApplyHit(DamageAmount, false, Attacker);
}

// Every hit goes through here: defense, damage taken, health loss, death, then hooks.
float AUnshackledCharacterBase::ApplyHit(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker)
{
        // Dead or dashing characters take nothing.
        if (Health->IsDead())
        {
                return 0.0f;
        }
        if (bInvulnerable)
        {
                return 0.0f;
        }

        // Work out how much damage gets through our defense.
        float FinalDamage = CalculateFinalDamage(RawDamage, bTrueDamage);

        // This hit uses up any "next hit" effects. This happens AFTER the damage was calculated above.
        Stats->ConsumeHit();

        // Remember health before the hit, so we know how much was really lost (overkill does not count).
        float HealthBefore = Health->CurrentHealth;
        Health->TakeHit(FinalDamage);
        float HealthAfter = Health->CurrentHealth;
        float DamageDealt = HealthBefore - HealthAfter;

        // See whether the hit killed us.
        CheckForDeath(Attacker);

        // Tell the attacker how much damage it really did (used by lifesteal, execute, etc.).
        if (Attacker != nullptr)
        {
                Attacker->OnDealtDamage(this, DamageDealt);
        }

        return DamageDealt;
}

// Defense shrinks normal damage: raw * Scale / (Scale + Defense). True damage skips defense.
// The IncomingDamage stat (veils and similar) applies to both kinds.
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

// If health hit 0, a passive may cancel the death. Otherwise we die and credit the attacker.
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

// Base version only does lifesteal. Characters override this for passives.
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
        // Nothing by default.
}

bool AUnshackledCharacterBase::WouldDie()
{
        // Nothing cancels death by default.
        return false;
}

void AUnshackledCharacterBase::Die()
{
        GetCharacterMovement()->DisableMovement();
        SetActorEnableCollision(false);
}

void AUnshackledCharacterBase::BasicAttack()
{
        // Enemies and anything without mouse aim just attack the way they face.
        AttackInDirection(FacingDirection);
}

void AUnshackledCharacterBase::AttackInDirection(FVector Direction)
{
        // Make the direction flat and exactly length 1, so AttackRange means the same thing every time.
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

        // Stunned or frozen characters cannot attack (rooted ones still can).
        if (!Stats->CanAct())
        {
                return;
        }

        // Block more attacks until the cooldown finishes.
        bCanAttack = false;
        GetWorldTimerManager().SetTimer(AttackTimer, this, &AUnshackledCharacterBase::ResetAttack, AttackCooldown, false);

        // Put an invisible sphere in front of the character, toward the given direction.
        FVector SphereCenter = GetActorLocation() + (Direction * AttackRange);

        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this); // don't hit ourselves

        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

        FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRadius);

        GetWorld()->OverlapMultiByObjectType(OverlapResults, SphereCenter, FQuat::Identity, ObjectParams, Sphere, QueryParams);

        // Go through everything the sphere touched.
        TArray<AActor*> ActorsAlreadyHit;
        for (int32 i = 0; i < OverlapResults.Num(); i++)
        {
                AActor* HitActor = OverlapResults[i].GetActor();
                if (HitActor == nullptr)
                {
                        continue;
                }

                // One character can overlap with several components, so only hit each once.
                if (ActorsAlreadyHit.Contains(HitActor))
                {
                        continue;
                }
                ActorsAlreadyHit.Add(HitActor);

                // Only hit characters from the other side.
                AUnshackledCharacterBase* OtherCharacter = Cast<AUnshackledCharacterBase>(HitActor);
                if (OtherCharacter == nullptr)
                {
                        continue;
                }
                if (OtherCharacter->bIsEnemy == bIsEnemy)
                {
                        continue;
                }

                // Our Damage stat times 1.0 (a basic attack), normal damage so defense applies.
                DealDamage(OtherCharacter, 1.0f, false);
        }

        // Red circle so we can see the attack area while gray-boxing. Remove later.
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

        // Rooted, stunned, or frozen characters cannot dash.
        if (!Stats->CanMove())
        {
                return;
        }

        bCanDash = false;
        bInvulnerable = true;

        // Shove the character in the direction they're facing.
        FVector DashVelocity = FacingDirection * DashSpeed;
        LaunchCharacter(DashVelocity, true, true);
        GetWorldTimerManager().SetTimer(DashDurationTimer, this, &AUnshackledCharacterBase::EndDash, DashDuration, false);

        // One timer ends the immunity, a second one ends the cooldown.
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