#include "Player/TitleController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
ATitleController::ATitleController()
{
	bShowMouseCursor = false;
	GameMenu = CreateDefaultSubobject<UGameMenuComponent>(TEXT("GameMenu"));
	GameMenu->Kind = EGameMenuKind::Title;
}
void ATitleController::BeginPlay()
{
	Super::BeginPlay();
	FInputModeGameOnly GameOnlyMode;
	SetInputMode(GameOnlyMode);
	UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (InputSystem == nullptr)
	{
		return;
	}
	if (MappingContext != nullptr)
	{
		InputSystem->AddMappingContext(MappingContext, 0);
	}
}
void ATitleController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (Input == nullptr)
	{
		return;
	}
	if (NavigateAction != nullptr)
	{
		Input->BindAction(NavigateAction, ETriggerEvent::Triggered, this, &ATitleController::Navigate);
		Input->BindAction(NavigateAction, ETriggerEvent::Completed, this, &ATitleController::NavigateReleased);
	}
	if (ConfirmAction != nullptr)
	{
		Input->BindAction(ConfirmAction, ETriggerEvent::Started, this, &ATitleController::Confirm);
	}
	if (BackAction != nullptr)
	{
		Input->BindAction(BackAction, ETriggerEvent::Started, this, &ATitleController::Back);
	}
}
void ATitleController::Navigate(const FInputActionValue& Value)
{
	GameMenu->HandleNavigate(Value.Get<FVector2D>());
}
void ATitleController::NavigateReleased(const FInputActionValue& Value)
{
	GameMenu->HandleNavigateReleased();
}
void ATitleController::Confirm(const FInputActionValue& Value)
{
	GameMenu->Confirm();
}
void ATitleController::Back(const FInputActionValue& Value)
{
	GameMenu->Back();
}
