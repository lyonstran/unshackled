#include "Game/UnshackledGameMode.h"
#include "Player/UnshackledPlayerController.h"
#include "UI/UnshackledHUD.h"

AUnshackledGameMode::AUnshackledGameMode()
{
	PlayerControllerClass = AUnshackledPlayerController::StaticClass();
	HUDClass = AUnshackledHUD::StaticClass();
}