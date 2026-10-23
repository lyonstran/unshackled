#include "EnemyCharacter.h"
#include "Characters/PartyMemberCharacter.h"
#include "Characters/FlowerCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
AEnemyCharacter::AEnemyCharacter()
{
        PrimaryActorTick.bCanEverTick = true;
        bIsEnemy = true;
        AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}
AUnshackledCharacterBase* AEnemyCharacter::FindClosestTarget()
{
        AUnshackledCharacterBase* ClosestTarget = nullptr;
        float ClosestDistance = 0.0f;
        for (TActorIterator<APartyMemberCharacter> It(GetWorld()); It; ++It)
        {
                APartyMemberCharacter* Member = *It;
                if (Member != nullptr)
                {
                        bool bCanBeTargeted = Member->IsPlayerControlled();
                        if (Member->IsGrounded())
                        {
                                bCanBeTargeted = true;
                        }
                        if (Member->Health->IsDead())
                        {
                                bCanBeTargeted = false;
                        }
                        if (bCanBeTargeted)
                        {
                                float Distance = FVector::Dist2D(Member->GetActorLocation(), GetActorLocation());
                                bool bIsCloser = false;
                                if (ClosestTarget == nullptr)
                                {
                                        bIsCloser = true;
                                }
                                else
                                {
                                        if (Distance < ClosestDistance)
                                        {
                                                bIsCloser = true;
                                        }
                                }
                                if (bIsCloser)
                                {
                                        ClosestTarget = Member;
                                        ClosestDistance = Distance;
                                }
                        }
                }
        }
        for (TActorIterator<AFlowerCharacter> FlowerIt(GetWorld()); FlowerIt; ++FlowerIt)
        {
                AFlowerCharacter* Flower = *FlowerIt;
                if (Flower != nullptr)
                {
                        if (!Flower->Health->IsDead())
                        {
                                float FlowerDistance = FVector::Dist2D(Flower->GetActorLocation(), GetActorLocation());
                                bool bFlowerIsCloser = false;
                                if (ClosestTarget == nullptr)
                                {
                                        bFlowerIsCloser = true;
                                }
                                else
                                {
                                        if (FlowerDistance < ClosestDistance)
                                        {
                                                bFlowerIsCloser = true;
                                        }
                                }
                                if (bFlowerIsCloser)
                                {
                                        ClosestTarget = Flower;
                                        ClosestDistance = FlowerDistance;
                                }
                        }
                }
        }
        return ClosestTarget;
}
void AEnemyCharacter::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        if (Health->IsDead())
        {
                return;
        }
        AUnshackledCharacterBase* Target = FindClosestTarget();
        if (Target == nullptr)
        {
                return;
        }
        FVector VectorToTarget = Target->GetActorLocation() - GetActorLocation();
        VectorToTarget.Z = 0.0f;
        float DistanceToTarget = VectorToTarget.Size();
        FVector DirectionToTarget = VectorToTarget;
        DirectionToTarget.Normalize();
        if (DistanceToTarget > AttackTriggerDistance)
        {
                FVector2D MoveInput = FVector2D(DirectionToTarget.Y, DirectionToTarget.X);
                MoveCharacter(MoveInput);
        }
        else
        {
                FacingDirection = DirectionToTarget;
                BasicAttack();
        }
}
void AEnemyCharacter::Die()
{
        Super::Die();
        SetLifeSpan(1.0f);
}
