#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputMappingContext.h"
#include "GameMenuComponent.generated.h"
class UUnshackledGameInstance;
UENUM()
enum class EGameMenuKind : uint8
{
	Title,
	Pause
};
enum class EGameMenuOption : uint8
{
	Continue,
	NewGame,
	Resume,
	Save,
	QuitToTitle,
	ExitGame
};
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UNSHACKLED_API UGameMenuComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UGameMenuComponent();
	UPROPERTY(EditAnywhere, Category = "Menu")
	EGameMenuKind Kind = EGameMenuKind::Pause;
	UPROPERTY(EditAnywhere, Category = "Menu")
	UInputMappingContext* MenuMappingContext = nullptr;
	UPROPERTY(EditAnywhere, Category = "Menu")
	int32 MenuMappingPriority = 10;
	UPROPERTY(EditAnywhere, Category = "Menu")
	float PressThreshold = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Menu")
	float ReleaseThreshold = 0.3f;
	void OpenMenu();
	void CloseMenu();
	void TogglePauseMenu();
	bool IsOpen() const;
	void HandleNavigate(FVector2D Axis);
	void HandleNavigateReleased();
	void Step(int32 Direction);
	void Confirm();
	void Back();
	FString GetMenuTitle() const;
	int32 GetOptionCount() const;
	FString GetOptionLabel(int32 Index) const;
	bool IsOptionEnabled(int32 Index) const;
	int32 GetSelectedIndex() const;
	bool IsConfirming() const;
	FString GetConfirmQuestion() const;
	bool IsConfirmYesSelected() const;
	FString GetStatusText() const;
protected:
	virtual void BeginPlay() override;
private:
	bool bOpen = false;
	bool bNavigateLatched = false;
	bool bConfirming = false;
	bool bConfirmYesSelected = false;
	EGameMenuOption PendingOption = EGameMenuOption::Resume;
	int32 SelectedIndex = 0;
	TArray<EGameMenuOption> Options;
	FString StatusText;
	void BuildOptions();
	void SelectFirstEnabled();
	bool IsOptionTypeEnabled(EGameMenuOption Option) const;
	bool NeedsConfirmation(EGameMenuOption Option) const;
	void RunOption(EGameMenuOption Option);
	void SaveFromMenu();
	void AddMenuInput();
	void RemoveMenuInput();
	APlayerController* GetOwnerController() const;
	UUnshackledGameInstance* GetUnshackledGameInstance() const;
};
