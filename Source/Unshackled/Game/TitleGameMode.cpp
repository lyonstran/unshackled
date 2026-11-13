#include "Game/TitleGameMode.h"
#include "Player/TitleController.h"
#include "UI/TitleHUD.h"
ATitleGameMode::ATitleGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ATitleController::StaticClass();
	HUDClass = ATitleHUD::StaticClass();
}
