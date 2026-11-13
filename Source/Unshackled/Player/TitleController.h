#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Components/GameMenuComponent.h"
#include "TitleController.generated.h"
UCLASS()
class UNSHACKLED_API ATitleController : public APlayerController
{
	GENERATED_BODY()
public:
	ATitleController();
	UPROPERTY(VisibleAnywhere, Category = "Menu")
	UGameMenuComponent* GameMenu;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* MappingContext;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* NavigateAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ConfirmAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* BackAction;
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
private:
	void Navigate(const FInputActionValue& Value);
	void NavigateReleased(const FInputActionValue& Value);
	void Confirm(const FInputActionValue& Value);
	void Back(const FInputActionValue& Value);
};
