#include "UnshackledPlayerController.h"
#include "Characters/UnshackledCharacterBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
void AUnshackledPlayerController::BeginPlay()
{
        Super::BeginPlay();
        bShowMouseCursor = true;
        UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
        if (InputSystem != nullptr && MappingContext != nullptr)
        {
                InputSystem->AddMappingContext(MappingContext, 0);
        }
}
void AUnshackledPlayerController::PlayerTick(float DeltaTime)
{
        Super::PlayerTick(DeltaTime);
        float MouseDeltaX = 0.0f;
        float MouseDeltaY = 0.0f;
        GetInputMouseDelta(MouseDeltaX, MouseDeltaY);
        if (MouseDeltaX != 0.0f)
        {
                bAimWithStick = false;
        }
        if (MouseDeltaY != 0.0f)
        {
                bAimWithStick = false;
        }
}
void AUnshackledPlayerController::Aim(const FInputActionValue& Value)
{
        FVector2D StickInput = Value.Get<FVector2D>();
        float StickLength = StickInput.Size();
        if (StickLength < StickAimThreshold)
        {
                return;
        }
        FVector NewAimDirection = FVector(StickInput.Y, StickInput.X, 0.0f);
        NewAimDirection.Normalize();
        LastStickAimDirection = NewAimDirection;
        bAimWithStick = true;
}
FVector AUnshackledPlayerController::GetAimDirection(AUnshackledCharacterBase* MyCharacter)
{
        FVector FallbackDirection = MyCharacter->GetFacingDirection();
        if (bAimWithStick)
        {
                return LastStickAimDirection;
        }
        FVector MouseWorldOrigin;
        FVector MouseWorldDirection;
        bool bGotMouse = DeprojectMousePositionToWorld(MouseWorldOrigin, MouseWorldDirection);
        if (!bGotMouse)
        {
                return FallbackDirection;
        }
        FVector LineEnd = MouseWorldOrigin + (MouseWorldDirection * 100000.0f);
        FVector PlanePoint = MyCharacter->GetActorLocation();
        FVector PlaneNormal = FVector(0.0f, 0.0f, 1.0f);
        FVector CursorPoint = FMath::LinePlaneIntersection(MouseWorldOrigin, LineEnd, PlanePoint, PlaneNormal);
        FVector AimDirection = CursorPoint - MyCharacter->GetActorLocation();
        AimDirection.Z = 0.0f;
        if (AimDirection.IsNearlyZero())
        {
                return FallbackDirection;
        }
        return AimDirection;
}
void AUnshackledPlayerController::Attack()
{
        AUnshackledCharacterBase* MyCharacter = Cast<AUnshackledCharacterBase>(GetPawn());
        if (MyCharacter == nullptr)
        {
                return;
        }
        FVector AttackDirection = GetAimDirection(MyCharacter);
        MyCharacter->AttackInDirection(AttackDirection);
}
void AUnshackledPlayerController::Dash()
{
        AUnshackledCharacterBase* MyCharacter = Cast<AUnshackledCharacterBase>(GetPawn());
        if (MyCharacter != nullptr)
        {
                MyCharacter->Dash();
        }
}
void AUnshackledPlayerController::Special()
{
        AUnshackledCharacterBase* MyCharacter = Cast<AUnshackledCharacterBase>(GetPawn());
        if (MyCharacter == nullptr)
        {
                return;
        }
        FVector AimDirection = GetAimDirection(MyCharacter);
        MyCharacter->UseSpecial(AimDirection);
}
void AUnshackledPlayerController::Ultimate()
{
        AUnshackledCharacterBase* MyCharacter = Cast<AUnshackledCharacterBase>(GetPawn());
        if (MyCharacter == nullptr)
        {
                return;
        }
        FVector AimDirection = GetAimDirection(MyCharacter);
        MyCharacter->UseUltimate(AimDirection);
}
void AUnshackledPlayerController::SetupInputComponent()
{
        Super::SetupInputComponent();
        UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
        if (Input != nullptr && MoveAction != nullptr)
        {
                Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUnshackledPlayerController::Move);
                if (AttackAction != nullptr)
                {
                        Input->BindAction(AttackAction, ETriggerEvent::Started, this, &AUnshackledPlayerController::Attack);
                }
                if (DashAction != nullptr)
                {
                        Input->BindAction(DashAction, ETriggerEvent::Started, this, &AUnshackledPlayerController::Dash);
                }
                if (SpecialAction != nullptr)
                {
                        Input->BindAction(SpecialAction, ETriggerEvent::Started, this, &AUnshackledPlayerController::Special);
                }
                if (UltimateAction != nullptr)
                {
                        Input->BindAction(UltimateAction, ETriggerEvent::Started, this, &AUnshackledPlayerController::Ultimate);
                }
                if (AimAction != nullptr)
                {
                        Input->BindAction(AimAction, ETriggerEvent::Triggered, this, &AUnshackledPlayerController::Aim);
                }
        }
}
void AUnshackledPlayerController::Move(const FInputActionValue& Value)
{
        FVector2D Direction = Value.Get<FVector2D>();
        AUnshackledCharacterBase* MyCharacter = Cast<AUnshackledCharacterBase>(GetPawn());
        if (MyCharacter != nullptr)
        {
                MyCharacter->MoveCharacter(Direction);
        }
}
