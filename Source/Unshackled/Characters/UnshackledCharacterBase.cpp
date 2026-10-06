#include "UnshackledCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

AUnshackledCharacterBase::AUnshackledCharacterBase()
{
      bUseControllerRotationYaw = false;
      GetCharacterMovement()->bOrientRotationToMovement = false;
      GetCharacterMovement()->MaxWalkSpeed = 300.0f;
      GetCharacterMovement()->BrakingDecelerationFlying = 1000.0f;
      GetCharacterMovement()->MaxAcceleration = 3500.0f;
      GetCharacterMovement()->BrakingDecelerationWalking = 3000.0f;
      GetSprite()->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));

      Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
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

      AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Direction.Y);
      AddMovementInput(FVector(0.0f, 1.0f, 0.0f), Direction.X);
}

float AUnshackledCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
      AController* EventInstigator, AActor* DamageCauser)
{
      //immune damage if already dead or in the middle of a dash.
      if (Health->IsDead())
      {
              return 0.0f;
      }
      if (bInvulnerable)
      {
              return 0.0f;
      }

      Health->TakeHit(DamageAmount);

      // Check right after the hit whether it killed us.
      if (Health->IsDead())
      {
              Die();
      }

      return DamageAmount;
}

void AUnshackledCharacterBase::Die()
{
      GetCharacterMovement()->DisableMovement();
      SetActorEnableCollision(false);
}

void AUnshackledCharacterBase::BasicAttack()
{
      if (!bCanAttack)
      {
              return;
      }
      if (Health->IsDead())
      {
              return;
      }

      // Block more attacks until the cooldown finishes.
      bCanAttack = false;
      GetWorldTimerManager().SetTimer(AttackTimer, this, &AUnshackledCharacterBase::ResetAttack, AttackCooldown, false);

      // Put an invisible sphere in front of the character.
      FVector SphereCenter = GetActorLocation() + (FacingDirection * AttackRange);

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

              UGameplayStatics::ApplyDamage(HitActor, AttackDamage, GetController(), this, nullptr);
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

      bCanDash = false;
      bInvulnerable = true;

      // Shove the character in the direction they're facing.
      FVector DashVelocity = FacingDirection * DashSpeed;
      LaunchCharacter(DashVelocity, true, true);

      // One timer ends the immunity, a second one ends the cooldown.
      GetWorldTimerManager().SetTimer(DashImmunityTimer, this, &AUnshackledCharacterBase::EndInvulnerability, DashImmunityTime, false);
      GetWorldTimerManager().SetTimer(DashCooldownTimer, this, &AUnshackledCharacterBase::ResetDash, DashCooldown, false);
}

void AUnshackledCharacterBase::EndInvulnerability()
{
      bInvulnerable = false;
}

void AUnshackledCharacterBase::ResetDash()
{
      bCanDash = true;
}