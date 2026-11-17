#include "EnemyCharacter.h"
#include "Characters/PartyMemberCharacter.h"
#include "Characters/FlowerCharacter.h"
#include "EngineUtils.h"
#include "Game/UnshackledGameInstance.h"
#include "UI/UnshackledHUD.h"
#include "GameFramework/CharacterMovementComponent.h"
AEnemyCharacter::AEnemyCharacter()
{
        PrimaryActorTick.bCanEverTick = true;
        bIsEnemy = true;
        AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
        ChestClass = AItemChestActor::StaticClass();
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
        DropLoot();
        SetLifeSpan(1.0f);
}
void AEnemyCharacter::DropLoot()
{
        if (bIsMiniboss)
        {
                SpawnMinibossChest();
                return;
        }
        UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
        if (GameInstanceRef == nullptr)
        {
                return;
        }
        TArray<FName> Dropped = GameInstanceRef->RollEnemyDrops();
        if (Dropped.Num() == 0)
        {
                return;
        }
        FString Text = TEXT("Loot:");
        for (int32 i = 0; i < Dropped.Num(); i++)
        {
                Text = Text + TEXT(" ") + GameInstanceRef->GetItemName(Dropped[i]);
                if (i < Dropped.Num() - 1)
                {
                        Text = Text + TEXT(",");
                }
        }
        AUnshackledHUD* Hud = AUnshackledHUD::FindPlayerHUD(this);
        if (Hud != nullptr)
        {
                Hud->AddMessage(Text, FLinearColor(0.6f, 1.0f, 0.6f, 1.0f), 3.0f);
        }
}
bool AEnemyCharacter::HasMinibossChest() const
{
        for (TActorIterator<AItemChestActor> It(GetWorld()); It; ++It)
        {
                AItemChestActor* Chest = *It;
                if (Chest->bFromMiniboss)
                {
                        return true;
                }
        }
        return false;
}
void AEnemyCharacter::SpawnMinibossChest()
{
        if (ChestClass == nullptr)
        {
                return;
        }
        if (HasMinibossChest())
        {
                return;
        }
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AItemChestActor* Chest = GetWorld()->SpawnActor<AItemChestActor>(ChestClass, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
        if (Chest == nullptr)
        {
                return;
        }
        Chest->bFromMiniboss = true;
        AUnshackledHUD* Hud = AUnshackledHUD::FindPlayerHUD(this);
        if (Hud != nullptr)
        {
                Hud->AddMessage(TEXT("The director dropped a chest"), FLinearColor(1.0f, 0.85f, 0.2f, 1.0f), 4.0f);
        }
}
float AEnemyCharacter::GetFloorStatScale() const
{
        UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
        if (GameInstanceRef == nullptr)
        {
                return 1.0f;
        }
        return GameInstanceRef->GetEnemyStatScale();
}
