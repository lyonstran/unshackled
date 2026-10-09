#include "UnshackledPlayerController.h"
#include "Characters/UnshackledCharacterBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void AUnshackledPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// The player aims with the mouse, so the cursor has to be visible.
	bShowMouseCursor = true;

	UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (InputSystem != nullptr && MappingContext != nullptr)
	{
		InputSystem->AddMappingContext(MappingContext, 0);
	}
}

void AUnshackledPlayerController::Attack()
{
	AUnshackledCharacterBase* MyCharacter = Cast<AUnshackledCharacterBase>(GetPawn());
	if (MyCharacter == nullptr)
	{
		return;
	}

	FVector MouseWorldOrigin;
	FVector MouseWorldDirection;
	bool bGotMouse = DeprojectMousePositionToWorld(MouseWorldOrigin, MouseWorldDirection);
	if (!bGotMouse)
	{
		MyCharacter->BasicAttack();
		return;
	}

	FVector LineEnd = MouseWorldOrigin + (MouseWorldDirection * 100000.0f);
	FVector PlanePoint = MyCharacter->GetActorLocation();
	FVector PlaneNormal = FVector(0.0f, 0.0f, 1.0f);
	FVector CursorPoint = FMath::LinePlaneIntersection(MouseWorldOrigin, LineEnd, PlanePoint, PlaneNormal);

	FVector AttackDirection = CursorPoint - MyCharacter->GetActorLocation();
	AttackDirection.Z = 0.0f;
	if (AttackDirection.IsNearlyZero())
	{
		
		MyCharacter->BasicAttack();
		return;
	}

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

