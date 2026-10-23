#include "FlowerCharacter.h"
#include "Characters/PartyMemberCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
AFlowerCharacter::AFlowerCharacter()
{
        PrimaryActorTick.bCanEverTick = true;
        bIsEnemy = false;
        Health->MaxHealth = 80.0f;
        Stats->BaseDefense = 10.0f;
        Stats->BaseSpeed = 0.0f;
        GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying;
        GetCharacterMovement()->GravityScale = 0.0f;
        GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}
void AFlowerCharacter::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        if (Health->IsDead())
        {
                return;
        }
        AgeSeconds = AgeSeconds + DeltaTime;
        if (AgeSeconds >= Lifetime)
        {
                Destroy();
                return;
        }
        HealAlliesInZone(DeltaTime);
        DrawZone();
}
void AFlowerCharacter::HealAlliesInZone(float DeltaTime)
{
        for (TActorIterator<APartyMemberCharacter> It(GetWorld()); It; ++It)
        {
                APartyMemberCharacter* Member = *It;
                if (!Member->Health->IsDead())
                {
                        float Distance = FVector::Dist2D(GetActorLocation(), Member->GetActorLocation());
                        if (Distance <= ZoneRadius)
                        {
                                float HealAmount = Member->Health->MaxHealth * HealFractionPerSecond * DeltaTime;
                                Member->Health->Heal(HealAmount);
                                if (Member->Stats->bHasMana)
                                {
                                        float MaxMana = Member->Stats->GetFinal(ECharacterStat::MaxMana);
                                        float ManaAmount = MaxMana * ManaFractionPerSecond * DeltaTime;
                                        Member->Stats->RestoreMana(ManaAmount);
                                }
                        }
                }
        }
}
void AFlowerCharacter::DrawZone()
{
        if (!bDrawDebugShape)
        {
                return;
        }
        DrawDebugSphere(GetWorld(), GetActorLocation(), ZoneRadius, 24, FColor::Green, false, 0.05f);
        DrawDebugSphere(GetWorld(), GetActorLocation(), 40.0f, 12, FColor::Magenta, false, 0.05f);
}
void AFlowerCharacter::Die()
{
        Super::Die();
        SetLifeSpan(1.0f);
}
