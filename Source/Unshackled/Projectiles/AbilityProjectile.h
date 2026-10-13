#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperFlipbookComponent.h"
#include "Characters/UnshackledCharacterBase.h"
#include "AbilityProjectile.generated.h"

UCLASS()
class UNSHACKLED_API AAbilityProjectile : public AActor
{
        GENERATED_BODY()

public:
        AAbilityProjectile();

        virtual void Tick(float DeltaTime) override;

        void Launch(AUnshackledCharacterBase* InOwner, FVector Direction, float InDamageScale, float InSizeScale);

        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
        UPaperFlipbookComponent* Visual;

        UPROPERTY(EditAnywhere, Category = "Projectile")
        float Speed = 700.0f;

        UPROPERTY(EditAnywhere, Category = "Projectile")
        float MaxRange = 220.0f;

        UPROPERTY(EditAnywhere, Category = "Projectile")
        float Radius = 100.0f;

        UPROPERTY(EditAnywhere, Category = "Projectile")
        bool bPierces = true;

        UPROPERTY(EditAnywhere, Category = "Projectile")
        bool bDrawDebugShape = true;

protected:
        void MoveForward(float DeltaTime);
        bool HitOverlappingCharacters();
        void DrawDebugShapeIfNoSprite();

        UPROPERTY()
        AUnshackledCharacterBase* OwnerCharacter;

        FVector TravelDirection = FVector(0.0f, 1.0f, 0.0f);
        float DamageScale = 1.0f;
        float SizeScale = 1.0f;
        float DistanceTraveled = 0.0f;

        TArray<AActor*> ActorsAlreadyHit;
};

// NOTES
// Make a Blueprint child (BP_CrescentProjectile) and set the flipbook on its Visual component. That is all the art needs.
// With no flipbook assigned, a debug sphere is drawn. Assigning one hides it automatically.
// Launch sets the owner (for damage and which side to hurt), direction, damage scale, and size scale.
// Radius and MaxRange are tuning numbers. The projectile hits characters whose bIsEnemy differs from the owner's.
// SizeScale is for empowered versions (Sirius's C). It scales the hit radius and the sprite.