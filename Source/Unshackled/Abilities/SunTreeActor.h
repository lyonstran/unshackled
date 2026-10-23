#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperFlipbookComponent.h"
#include "Characters/UnshackledCharacterBase.h"
#include "Characters/FlowerCharacter.h"
#include "SunTreeActor.generated.h"
UCLASS()
class UNSHACKLED_API ASunTreeActor : public AActor
{
        GENERATED_BODY()
public:
        ASunTreeActor();
        virtual void Tick(float DeltaTime) override;
        void Setup(AUnshackledCharacterBase* InOwner, TSubclassOf<AFlowerCharacter> InFlowerClass);
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sun Tree")
        UPaperFlipbookComponent* Visual;
        UPROPERTY(EditAnywhere, Category = "Sun Tree")
        float Lifetime = 30.0f;
        UPROPERTY(EditAnywhere, Category = "Sun Tree")
        float MissingHealthFractionPerSecond = 0.05f;
        UPROPERTY(EditAnywhere, Category = "Sun Tree")
        float MinEnemyDamageFractionPerSecond = 0.01f;
        UPROPERTY(EditAnywhere, Category = "Sun Tree")
        float FlowerWindow = 3.0f;
        UPROPERTY(EditAnywhere, Category = "Sun Tree")
        bool bDrawDebugShape = true;
protected:
        void TickBeam(float DeltaTime);
        void HitEnemy(AUnshackledCharacterBase* Enemy, float DeltaTime);
        void HealAlly(AUnshackledCharacterBase* Ally, float DeltaTime);
        void RecordEnemyHit(AUnshackledCharacterBase* Enemy);
        void TickFlowerConversion(float DeltaTime);
        void SpawnFlowerAt(FVector Location);
        UPROPERTY()
        AUnshackledCharacterBase* OwnerCharacter = nullptr;
        UPROPERTY()
        TSubclassOf<AFlowerCharacter> FlowerClass;
        UPROPERTY()
        TArray<AUnshackledCharacterBase*> TrackedEnemies;
        TArray<float> TrackedAges;
        bool bOwnerIsEnemy = false;
        float AgeSeconds = 0.0f;
};
