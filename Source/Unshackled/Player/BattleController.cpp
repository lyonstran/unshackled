#include "Player/BattleController.h"
#include "Battle/TurnBattleManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
ABattleController::ABattleController()
{
	bShowMouseCursor = false;
	GameMenu = CreateDefaultSubobject<UGameMenuComponent>(TEXT("GameMenu"));
}
void ABattleController::BeginPlay()
{
	Super::BeginPlay();
	UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (InputSystem != nullptr && MappingContext != nullptr)
	{
		InputSystem->AddMappingContext(MappingContext, 0);
	}
}
void ABattleController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (Input == nullptr)
	{
		return;
	}
	if (NavigateAction != nullptr)
	{
		Input->BindAction(NavigateAction, ETriggerEvent::Triggered, this, &ABattleController::Navigate);
		Input->BindAction(NavigateAction, ETriggerEvent::Completed, this, &ABattleController::NavigateReleased);
	}
	if (ConfirmAction != nullptr)
	{
		Input->BindAction(ConfirmAction, ETriggerEvent::Started, this, &ABattleController::Confirm);
	}
	if (BackAction != nullptr)
	{
		Input->BindAction(BackAction, ETriggerEvent::Started, this, &ABattleController::Back);
	}
	if (PauseAction != nullptr)
	{
		Input->BindAction(PauseAction, ETriggerEvent::Started, this, &ABattleController::TogglePause);
	}
}
ATurnBattleManager* ABattleController::FindManager() const
{
	for (TActorIterator<ATurnBattleManager> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}
bool ABattleController::IsGameMenuOpen() const
{
	if (GameMenu == nullptr)
	{
		return false;
	}
	return GameMenu->IsOpen();
}
bool ABattleController::CanManagerGoBack(ATurnBattleManager* Manager) const
{
	if (Manager == nullptr)
	{
		return false;
	}
	if (Manager->GetState() != EBattleState::PlayerPhase)
	{
		return false;
	}
	if (Manager->GetMenuStage() == EBattleMenuStage::PickMember)
	{
		return false;
	}
	return true;
}
void ABattleController::Navigate(const FInputActionValue& Value)
{
	FVector2D Axis = Value.Get<FVector2D>();
	if (IsGameMenuOpen())
	{
		GameMenu->HandleNavigate(Axis);
		return;
	}
	float AbsX = FMath::Abs(Axis.X);
	float AbsY = FMath::Abs(Axis.Y);
	if (bNavigateLatched)
	{
		if (AbsX < ReleaseThreshold && AbsY < ReleaseThreshold)
		{
			bNavigateLatched = false;
		}
		return;
	}
	if (AbsX < PressThreshold && AbsY < PressThreshold)
	{
		return;
	}
	int32 Horizontal = 0;
	int32 Vertical = 0;
	if (AbsX >= AbsY)
	{
		if (Axis.X > 0.0f)
		{
			Horizontal = 1;
		}
		else
		{
			Horizontal = -1;
		}
	}
	else
	{
		if (Axis.Y > 0.0f)
		{
			Vertical = 1;
		}
		else
		{
			Vertical = -1;
		}
	}
	bNavigateLatched = true;
	ATurnBattleManager* Manager = FindManager();
	if (Manager != nullptr)
	{
		Manager->MenuStep(Horizontal, Vertical);
	}
}
void ABattleController::NavigateReleased(const FInputActionValue& Value)
{
	bNavigateLatched = false;
	if (GameMenu != nullptr)
	{
		GameMenu->HandleNavigateReleased();
	}
}
void ABattleController::Confirm(const FInputActionValue& Value)
{
	if (IsGameMenuOpen())
	{
		GameMenu->Confirm();
		return;
	}
	ATurnBattleManager* Manager = FindManager();
	if (Manager != nullptr)
	{
		Manager->MenuConfirm();
	}
}
void ABattleController::Back(const FInputActionValue& Value)
{
	if (IsGameMenuOpen())
	{
		GameMenu->Back();
		return;
	}
	ATurnBattleManager* Manager = FindManager();
	if (CanManagerGoBack(Manager))
	{
		Manager->MenuBack();
		return;
	}
	if (GameMenu != nullptr)
	{
		GameMenu->OpenMenu();
	}
}
void ABattleController::TogglePause(const FInputActionValue& Value)
{
	if (GameMenu != nullptr)
	{
		GameMenu->TogglePauseMenu();
	}
}
