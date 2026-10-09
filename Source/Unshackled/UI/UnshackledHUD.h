#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UnshackledHUD.generated.h"

UCLASS()
class UNSHACKLED_API AUnshackledHUD : public AHUD
{
	GENERATED_BODY()

public:
	// Unreal calls this every frame. Everything drawn on screen starts here.
	virtual void DrawHUD() override;

	// Size of the health bar in screen pixels.
	UPROPERTY(EditAnywhere, Category = "Health Bars")
	float BarWidth = 70.0f;

	UPROPERTY(EditAnywhere, Category = "Health Bars")
	float BarHeight = 8.0f;

	
	UPROPERTY(EditAnywhere, Category = "Health Bars")
	float BarWorldOffset = 90.0f;

protected:
	// Draws one health bar above one character.
	void DrawHealthBar(class AUnshackledCharacterBase* Character);
};