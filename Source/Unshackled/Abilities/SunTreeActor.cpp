#include "SunTreeActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Components/HealthComponent.h"
ASunTreeActor::ASunTreeActor()
{
        PrimaryActorTick.bCanEverTick = true;
        USceneComponent* RootPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
        SetRootComponent(RootPoint);
        Visual = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Visual"));
        Visual->SetupAttachment(RootPoint);
        Visual->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
        Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void ASunTreeActor::Setup(AUnshackledCharacterBase* InOwner, TSubclassOf<AFlowerCharacter> InFlowerClass)
{
        OwnerCharacter = InOwner;
        FlowerClass = InFlowerClass;
        if (InOwner != nullptr)
        {
                bOwnerIsEnemy = InOwner->bIsEnemy;
        }
}
void ASunTreeActor::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        AgeSeconds = AgeSeconds + DeltaTime;
        if (AgeSeconds >= Lifetime)
        {
                Destroy();
                return;
        }
        TickBeam(DeltaTime);
        TickFlowerConversion(DeltaTime);
        if (bDrawDebugShape)
        {
                if (Visual->GetFlipbook() == nullptr)
                {
                        DrawDebugSphere(GetWorld(), GetActorLocation(), 50.0f, 12, FColor::Yellow, false, 0.0f);
                }
        }
}
void ASunTreeActor::TickBeam(float DeltaTime)
{
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (!Candidate->Health->IsDead())
                {
                        if (Candidate->bIsEnemy != bOwnerIsEnemy)
                        {
                                HitEnemy(Candidate, DeltaTime);
                        }
                        else
                        {
                                HealAlly(Candidate, DeltaTime);
                        }
                }
        }
}
void ASunTreeActor::HitEnemy(AUnshackledCharacterBase* Enemy, float DeltaTime)
{
        float MissingHealth = Enemy->Health->MaxHealth - Enemy->Health->CurrentHealth;
        float MissingDamage = MissingHealth * MissingHealthFractionPerSecond;
        float MinimumDamage = Enemy->Health->MaxHealth * MinEnemyDamageFractionPerSecond;
        float DamagePerSecond = MissingDamage;
        if (MinimumDamage > DamagePerSecond)
        {
                DamagePerSecond = MinimumDamage;
        }
        RecordEnemyHit(Enemy);
        if (bDrawDebugShape)
        {
                DrawDebugLine(GetWorld(), GetActorLocation(), Enemy->GetActorLocation(), FColor::Yellow, false, 0.0f, 0, 2.0f);
        }
        Enemy->ApplyHit(DamagePerSecond * DeltaTime, true, OwnerCharacter);
}
void ASunTreeActor::HealAlly(AUnshackledCharacterBase* Ally, float DeltaTime)
{
        float MissingHealth = Ally->Health->MaxHealth - Ally->Health->CurrentHealth;
        float HealAmount = MissingHealth * MissingHealthFractionPerSecond * DeltaTime;
        Ally->Health->Heal(HealAmount);
        if (bDrawDebugShape)
        {
                DrawDebugLine(GetWorld(), GetActorLocation(), Ally->GetActorLocation(), FColor::Green, false, 0.0f, 0, 2.0f);
        }
}
void ASunTreeActor::RecordEnemyHit(AUnshackledCharacterBase* Enemy)
{
        int32 FoundIndex = TrackedEnemies.Find(Enemy);
        if (FoundIndex != INDEX_NONE)
        {
                TrackedAges[FoundIndex] = 0.0f;
                return;
        }
        TrackedEnemies.Add(Enemy);
        TrackedAges.Add(0.0f);
}
void ASunTreeActor::TickFlowerConversion(float DeltaTime)
{
        TArray<AUnshackledCharacterBase*> StillTracked;
        TArray<float> StillAges;
        for (int32 i = 0; i < TrackedEnemies.Num(); i++)
        {
                AUnshackledCharacterBase* Enemy = TrackedEnemies[i];
                float Age = TrackedAges[i] + DeltaTime;
                if (IsValid(Enemy))
                {
                        if (Enemy->Health->IsDead())
                        {
                                SpawnFlowerAt(Enemy->GetActorLocation());
                        }
                        else
                        {
                                if (Age < FlowerWindow)
                                {
                                        StillTracked.Add(Enemy);
                                        StillAges.Add(Age);
                                }
                        }
                }
        }
        TrackedEnemies = StillTracked;
        TrackedAges = StillAges;
}
void ASunTreeActor::SpawnFlowerAt(FVector Location)
{
        TSubclassOf<AFlowerCharacter> ClassToSpawn = FlowerClass;
        if (ClassToSpawn == nullptr)
        {
                ClassToSpawn = AFlowerCharacter::StaticClass();
        }
        FRotator SpawnRotation = FRotator::ZeroRotator;
        FActorSpawnParameters SpawnParameters;
        SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        GetWorld()->SpawnActor<AFlowerCharacter>(ClassToSpawn, Location, SpawnRotation, SpawnParameters);
}
