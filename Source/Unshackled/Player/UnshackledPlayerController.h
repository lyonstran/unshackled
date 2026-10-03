#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "UnshackledPlayerController.generated.h"

UCLASS()
class UNSHACKLED_API AUnshackledPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext *MappingContext;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *MoveAction;
	
protected:
	virtual void BeginPlay() override;	
	virtual void SetupInputComponent() override;
	void Move(const FInputActionValue& Value);
};