#include "Components/GameMenuComponent.h"
#include "Game/UnshackledGameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
UGameMenuComponent::UGameMenuComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UGameMenuComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Kind == EGameMenuKind::Title)
	{
		OpenMenu();
	}
}
APlayerController* UGameMenuComponent::GetOwnerController() const
{
	APlayerController* Controller = Cast<APlayerController>(GetOwner());
	return Controller;
}
UUnshackledGameInstance* UGameMenuComponent::GetUnshackledGameInstance() const
{
	UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(UGameplayStatics::GetGameInstance(this));
	return GameInstanceRef;
}
void UGameMenuComponent::OpenMenu()
{
	if (bOpen)
	{
		return;
	}
	BuildOptions();
	bOpen = true;
	bConfirming = false;
	bConfirmYesSelected = false;
	bNavigateLatched = false;
	StatusText = TEXT("");
	SelectFirstEnabled();
	if (Kind == EGameMenuKind::Pause)
	{
		UGameplayStatics::SetGamePaused(this, true);
		AddMenuInput();
	}
}
void UGameMenuComponent::CloseMenu()
{
	if (!bOpen)
	{
		return;
	}
	bOpen = false;
	bConfirming = false;
	if (Kind == EGameMenuKind::Pause)
	{
		RemoveMenuInput();
		UGameplayStatics::SetGamePaused(this, false);
	}
}
void UGameMenuComponent::TogglePauseMenu()
{
	if (Kind != EGameMenuKind::Pause)
	{
		return;
	}
	if (bOpen)
	{
		CloseMenu();
	}
	else
	{
		OpenMenu();
	}
}
bool UGameMenuComponent::IsOpen() const
{
	return bOpen;
}
void UGameMenuComponent::BuildOptions()
{
	Options.Empty();
	if (Kind == EGameMenuKind::Title)
	{
		Options.Add(EGameMenuOption::Continue);
		Options.Add(EGameMenuOption::NewGame);
		Options.Add(EGameMenuOption::ExitGame);
		return;
	}
	Options.Add(EGameMenuOption::Resume);
	Options.Add(EGameMenuOption::Save);
	Options.Add(EGameMenuOption::QuitToTitle);
	Options.Add(EGameMenuOption::ExitGame);
}
void UGameMenuComponent::SelectFirstEnabled()
{
	SelectedIndex = 0;
	for (int32 i = 0; i < Options.Num(); i++)
	{
		if (IsOptionEnabled(i))
		{
			SelectedIndex = i;
			return;
		}
	}
}
void UGameMenuComponent::AddMenuInput()
{
	if (MenuMappingContext == nullptr)
	{
		return;
	}
	APlayerController* Controller = GetOwnerController();
	if (Controller == nullptr)
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer());
	if (InputSystem == nullptr)
	{
		return;
	}
	InputSystem->AddMappingContext(MenuMappingContext, MenuMappingPriority);
}
void UGameMenuComponent::RemoveMenuInput()
{
	if (MenuMappingContext == nullptr)
	{
		return;
	}
	APlayerController* Controller = GetOwnerController();
	if (Controller == nullptr)
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer());
	if (InputSystem == nullptr)
	{
		return;
	}
	InputSystem->RemoveMappingContext(MenuMappingContext);
}
void UGameMenuComponent::HandleNavigate(FVector2D Axis)
{
	if (!bOpen)
	{
		return;
	}
	float AbsX = FMath::Abs(Axis.X);
	float AbsY = FMath::Abs(Axis.Y);
	if (bNavigateLatched)
	{
		if (AbsX < ReleaseThreshold)
		{
			if (AbsY < ReleaseThreshold)
			{
				bNavigateLatched = false;
			}
		}
		return;
	}
	float Strongest = AbsX;
	if (AbsY > Strongest)
	{
		Strongest = AbsY;
	}
	if (Strongest < PressThreshold)
	{
		return;
	}
	bNavigateLatched = true;
	if (bConfirming)
	{
		bConfirmYesSelected = !bConfirmYesSelected;
		return;
	}
	if (AbsY < AbsX)
	{
		return;
	}
	if (Axis.Y > 0.0f)
	{
		Step(-1);
	}
	else
	{
		Step(1);
	}
}
void UGameMenuComponent::HandleNavigateReleased()
{
	bNavigateLatched = false;
}
void UGameMenuComponent::Step(int32 Direction)
{
	int32 Count = Options.Num();
	if (Count == 0)
	{
		return;
	}
	int32 Candidate = SelectedIndex;
	for (int32 Attempt = 0; Attempt < Count; Attempt++)
	{
		Candidate = Candidate + Direction;
		if (Candidate >= Count)
		{
			Candidate = 0;
		}
		if (Candidate < 0)
		{
			Candidate = Count - 1;
		}
		if (IsOptionEnabled(Candidate))
		{
			SelectedIndex = Candidate;
			return;
		}
	}
}
void UGameMenuComponent::Confirm()
{
	if (!bOpen)
	{
		return;
	}
	if (bConfirming)
	{
		bConfirming = false;
		if (bConfirmYesSelected)
		{
			RunOption(PendingOption);
		}
		return;
	}
	if (!IsOptionEnabled(SelectedIndex))
	{
		return;
	}
	EGameMenuOption Chosen = Options[SelectedIndex];
	if (NeedsConfirmation(Chosen))
	{
		PendingOption = Chosen;
		bConfirming = true;
		bConfirmYesSelected = false;
		return;
	}
	RunOption(Chosen);
}
void UGameMenuComponent::Back()
{
	if (!bOpen)
	{
		return;
	}
	if (bConfirming)
	{
		bConfirming = false;
		return;
	}
	if (Kind == EGameMenuKind::Pause)
	{
		CloseMenu();
	}
}
bool UGameMenuComponent::NeedsConfirmation(EGameMenuOption Option) const
{
	if (Option == EGameMenuOption::NewGame)
	{
		UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
		if (GameInstanceRef == nullptr)
		{
			return false;
		}
		return GameInstanceRef->HasSaveGame();
	}
	if (Option == EGameMenuOption::QuitToTitle)
	{
		return true;
	}
	if (Option == EGameMenuOption::ExitGame)
	{
		if (Kind == EGameMenuKind::Pause)
		{
			return true;
		}
	}
	return false;
}
void UGameMenuComponent::RunOption(EGameMenuOption Option)
{
	if (Option == EGameMenuOption::Resume)
	{
		CloseMenu();
		return;
	}
	if (Option == EGameMenuOption::Save)
	{
		SaveFromMenu();
		return;
	}
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		StatusText = TEXT("Set Game Instance Class in Project Settings");
		return;
	}
	if (Option == EGameMenuOption::Continue)
	{
		bool bLoaded = GameInstanceRef->ContinueGame();
		if (!bLoaded)
		{
			StatusText = TEXT("Could not load the save");
		}
		return;
	}
	if (Option == EGameMenuOption::NewGame)
	{
		GameInstanceRef->StartNewGame();
		return;
	}
	if (Option == EGameMenuOption::QuitToTitle)
	{
		CloseMenu();
		GameInstanceRef->QuitToTitle();
		return;
	}
	if (Option == EGameMenuOption::ExitGame)
	{
		GameInstanceRef->ExitGame(GetOwnerController());
	}
}
void UGameMenuComponent::SaveFromMenu()
{
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		StatusText = TEXT("Set Game Instance Class in Project Settings");
		return;
	}
	bool bSaved = GameInstanceRef->SaveCheckpoint();
	if (bSaved)
	{
		StatusText = TEXT("Saved: ") + GameInstanceRef->GetCheckpointDescription();
	}
	else
	{
		StatusText = TEXT("Save failed");
	}
}
FString UGameMenuComponent::GetMenuTitle() const
{
	if (Kind == EGameMenuKind::Pause)
	{
		return TEXT("Paused");
	}
	return TEXT("");
}
int32 UGameMenuComponent::GetOptionCount() const
{
	return Options.Num();
}
FString UGameMenuComponent::GetOptionLabel(int32 Index) const
{
	if (Index < 0)
	{
		return TEXT("");
	}
	if (Index >= Options.Num())
	{
		return TEXT("");
	}
	EGameMenuOption Option = Options[Index];
	if (Option == EGameMenuOption::Continue)
	{
		return TEXT("Continue");
	}
	if (Option == EGameMenuOption::NewGame)
	{
		return TEXT("New Game");
	}
	if (Option == EGameMenuOption::Resume)
	{
		return TEXT("Resume");
	}
	if (Option == EGameMenuOption::Save)
	{
		return TEXT("Save");
	}
	if (Option == EGameMenuOption::QuitToTitle)
	{
		return TEXT("Quit to Title");
	}
	return TEXT("Exit Game");
}
bool UGameMenuComponent::IsOptionEnabled(int32 Index) const
{
	if (Index < 0)
	{
		return false;
	}
	if (Index >= Options.Num())
	{
		return false;
	}
	return IsOptionTypeEnabled(Options[Index]);
}
bool UGameMenuComponent::IsOptionTypeEnabled(EGameMenuOption Option) const
{
	if (Option != EGameMenuOption::Continue)
	{
		return true;
	}
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		return false;
	}
	return GameInstanceRef->HasSaveGame();
}
int32 UGameMenuComponent::GetSelectedIndex() const
{
	return SelectedIndex;
}
bool UGameMenuComponent::IsConfirming() const
{
	return bConfirming;
}
FString UGameMenuComponent::GetConfirmQuestion() const
{
	if (PendingOption == EGameMenuOption::NewGame)
	{
		return TEXT("Start a new game? Your save will be deleted.");
	}
	if (PendingOption == EGameMenuOption::QuitToTitle)
	{
		return TEXT("Quit to title? Progress since your last checkpoint is lost.");
	}
	if (PendingOption == EGameMenuOption::ExitGame)
	{
		return TEXT("Exit the game? Progress since your last checkpoint is lost.");
	}
	return TEXT("Are you sure?");
}
bool UGameMenuComponent::IsConfirmYesSelected() const
{
	return bConfirmYesSelected;
}
FString UGameMenuComponent::GetStatusText() const
{
	return StatusText;
}
