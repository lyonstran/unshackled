#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "BattleController.generated.h"
class ATurnBattleManager;
UCLASS()
class UNSHACKLED_API ABattleController : public APlayerController
{
	GENERATED_BODY()
public:
	ABattleController();
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* MappingContext;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* NavigateAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ConfirmAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* BackAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	float PressThreshold = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Input")
	float ReleaseThreshold = 0.3f;
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
private:
	bool bNavigateLatched = false;
	ATurnBattleManager* FindManager() const;
	void Navigate(const FInputActionValue& Value);
	void NavigateReleased(const FInputActionValue& Value);
	void Confirm(const FInputActionValue& Value);
	void Back(const FInputActionValue& Value);
};
