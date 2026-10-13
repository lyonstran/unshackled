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
