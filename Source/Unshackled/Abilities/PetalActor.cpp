#include "PetalActor.h"
#include "Characters/PartyMemberCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
APetalActor::APetalActor()
{
        PrimaryActorTick.bCanEverTick = true;
        USceneComponent* RootPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
        SetRootComponent(RootPoint);
        Visual = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Visual"));
        Visual->SetupAttachment(RootPoint);
        Visual->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void APetalActor::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        AgeSeconds = AgeSeconds + DeltaTime;
        if (AgeSeconds >= Lifetime)
        {
                Destroy();
                return;
        }
        APartyMemberCharacter* Ally = FindAllyOnPetal();
        if (Ally != nullptr)
        {
                Absorb(Ally);
                return;
        }
        if (bDrawDebugShape)
        {
                if (Visual->GetFlipbook() == nullptr)
                {
                        DrawDebugSphere(GetWorld(), GetActorLocation(), PickupRadius, 8, FColor::Magenta, false, 0.0f);
                }
        }
}
APartyMemberCharacter* APetalActor::FindAllyOnPetal()
{
        for (TActorIterator<APartyMemberCharacter> It(GetWorld()); It; ++It)
        {
                APartyMemberCharacter* Member = *It;
                if (!Member->Health->IsDead())
                {
                        float Distance = FVector::Dist2D(GetActorLocation(), Member->GetActorLocation());
                        if (Distance <= PickupRadius)
                        {
                                return Member;
                        }
                }
        }
        return nullptr;
}
void APetalActor::Absorb(APartyMemberCharacter* Ally)
{
        float MissingHealth = Ally->Health->MaxHealth - Ally->Health->CurrentHealth;
        Ally->Health->Heal(MissingHealth * HealMissingHealthFraction);
        if (Ally->Stats->bHasMana)
        {
                float MaxMana = Ally->Stats->GetFinal(ECharacterStat::MaxMana);
                float MissingMana = MaxMana - Ally->Stats->CurrentMana;
                if (MissingMana > 0.0f)
                {
                        Ally->Stats->RestoreMana(MissingMana * RestoreMissingManaFraction);
                }
        }
        Destroy();
}
