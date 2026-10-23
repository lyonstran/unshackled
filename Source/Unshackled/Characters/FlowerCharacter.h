#pragma once
#include "CoreMinimal.h"
#include "UnshackledCharacterBase.h"
#include "FlowerCharacter.generated.h"
UCLASS()
class UNSHACKLED_API AFlowerCharacter : public AUnshackledCharacterBase
{
        GENERATED_BODY()
public:
        AFlowerCharacter();
        virtual void Tick(float DeltaTime) override;
        UPROPERTY(EditAnywhere, Category = "Flower")
        float Lifetime = 15.0f;
        UPROPERTY(EditAnywhere, Category = "Flower")
        float ZoneRadius = 250.0f;
        UPROPERTY(EditAnywhere, Category = "Flower")
        float HealFractionPerSecond = 0.025f;
        UPROPERTY(EditAnywhere, Category = "Flower")
        float ManaFractionPerSecond = 0.025f;
        UPROPERTY(EditAnywhere, Category = "Flower")
        bool bDrawDebugShape = true;
protected:
        virtual void Die() override;
        void HealAlliesInZone(float DeltaTime);
        void DrawZone();
        float AgeSeconds = 0.0f;
};
