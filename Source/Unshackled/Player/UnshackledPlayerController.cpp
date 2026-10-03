#include "UnshackledPlayerController.h"
#include "Characters/UnshackledCharacterBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void AUnshackledPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (InputSystem != nullptr && MappingContext != nullptr)
	{
		InputSystem->AddMappingContext(MappingContext, 0);
	}
}

void AUnshackledPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (Input != nullptr && MoveAction != nullptr)
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUnshackledPlayerController::Move);
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