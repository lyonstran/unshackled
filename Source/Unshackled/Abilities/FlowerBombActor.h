#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperFlipbookComponent.h"
#include "Characters/UnshackledCharacterBase.h"
#include "FlowerBombActor.generated.h"
UCLASS()
class UNSHACKLED_API AFlowerBombActor : public AActor
{
        GENERATED_BODY()
public:
        AFlowerBombActor();
        virtual void Tick(float DeltaTime) override;
        void Setup(AUnshackledCharacterBase* InOwner);
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flower Bomb")
        UPaperFlipbookComponent* Visual;
        UPROPERTY(EditAnywhere, Category = "Flower Bomb")
        float Lifetime = 20.0f;
        UPROPERTY(EditAnywhere, Category = "Flower Bomb")
        float TriggerRadius = 70.0f;
        UPROPERTY(EditAnywhere, Category = "Flower Bomb")
        float BlastRadius = 160.0f;
        UPROPERTY(EditAnywhere, Category = "Flower Bomb")
        float DefenseReduction = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Flower Bomb")
        float DamageReduction = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Flower Bomb")
        float SlowFraction = 0.5f;
        UPROPERTY(EditAnywhere, Category = "Flower Bomb")
        float DebuffDuration = 5.0f;
        UPROPERTY(EditAnywhere, Category = "Flower Bomb")
        bool bDrawDebugShape = true;
protected:
        bool IsEnemyOnBomb();
        void Explode();
        void ApplyDebuffs(AUnshackledCharacterBase* Target);
        void DrawDebugShapeIfNoSprite();
        bool bOwnerIsEnemy = false;
        float AgeSeconds = 0.0f;
};
