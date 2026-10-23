#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperFlipbookComponent.h"
#include "PetalActor.generated.h"
UCLASS()
class UNSHACKLED_API APetalActor : public AActor
{
        GENERATED_BODY()
public:
        APetalActor();
        virtual void Tick(float DeltaTime) override;
        UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Petal")
        UPaperFlipbookComponent* Visual;
        UPROPERTY(EditAnywhere, Category = "Petal")
        float Lifetime = 10.0f;
        UPROPERTY(EditAnywhere, Category = "Petal")
        float PickupRadius = 80.0f;
        UPROPERTY(EditAnywhere, Category = "Petal")
        float HealMissingHealthFraction = 0.025f;
        UPROPERTY(EditAnywhere, Category = "Petal")
        float RestoreMissingManaFraction = 0.025f;
        UPROPERTY(EditAnywhere, Category = "Petal")
        bool bDrawDebugShape = true;
protected:
        class APartyMemberCharacter* FindAllyOnPetal();
        void Absorb(class APartyMemberCharacter* Ally);
        float AgeSeconds = 0.0f;
};
