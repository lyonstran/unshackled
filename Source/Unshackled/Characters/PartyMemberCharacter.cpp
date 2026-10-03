#include "PartyMemberCharacter.h"

APartyMemberCharacter::APartyMemberCharacter()
{
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->SetUsingAbsoluteRotation(true);
	CameraArm->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	CameraArm->TargetArmLength = 1000.0f;
	CameraArm->bDoCollisionTest = false; 
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm);
	Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
	Camera->SetOrthoWidth(1300.0f);
}


// create camera arm and attach it to the character, points arm straight into ground,
// doesnt let walls push camera in, the orthographic gets the flat 2d look