#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "UnshackledPlayerController.generated.h"
UCLASS()
class UNSHACKLED_API AUnshackledPlayerController : public APlayerController
{
        GENERATED_BODY()
public:
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputMappingContext *MappingContext;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *MoveAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *AttackAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *DashAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *SpecialAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *UltimateAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *AimAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        float StickAimThreshold = 0.25f;
protected:
        virtual void BeginPlay() override;
        virtual void SetupInputComponent() override;
        virtual void PlayerTick(float DeltaTime) override;
        void Move(const FInputActionValue& Value);
        void Aim(const FInputActionValue& Value);
        void Attack();
        void Dash();
        void Special();
        void Ultimate();
        FVector GetAimDirection(class AUnshackledCharacterBase* MyCharacter);
        bool bAimWithStick = false;
        FVector LastStickAimDirection = FVector(0.0f, 1.0f, 0.0f);
};
