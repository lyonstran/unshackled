#include "UnshackledCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"

AUnshackledCharacterBase::AUnshackledCharacterBase()
{
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	GetCharacterMovement()->BrakingDecelerationFlying = 1000.0f;
	GetCharacterMovement()->MaxAcceleration = 3500.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 3000.0f;
	GetSprite()->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
}

void AUnshackledCharacterBase::MoveCharacter(FVector2D Direction)
{
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Direction.Y);
	AddMovementInput(FVector(0.0f, 1.0f, 0.0f), Direction.X);
}


// AUnshackledCharacterBase -> stops char from turning when mouse or controller turns, 
// stops sprite from rotating toward direction of walking, 
// set speed of character walk, 
// set speed of character stop walking (making it so that it stops quickly), 
// lays flatness of sprite

// MoveCharacter -> camera is setup to look straight down at the ground
// w to move forward on x, d to move along y 