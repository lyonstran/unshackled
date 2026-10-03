#include "Game/UnshackledGameMode.h"
#include "Player/UnshackledPlayerController.h"

AUnshackledGameMode::AUnshackledGameMode()
{
	PlayerControllerClass = AUnshackledPlayerController::StaticClass();
}