#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UnshackledHUD.generated.h"

UCLASS()
class UNSHACKLED_API AUnshackledHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Health Bars")
	float BarWidth = 70.0f;

	UPROPERTY(EditAnywhere, Category = "Health Bars")
	float BarHeight = 8.0f;

	UPROPERTY(EditAnywhere, Category = "Health Bars")
	float BarWorldOffset = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Health Bars")
	float ManaBarHeight = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Health Bars")
	float ManaBarGap = 2.0f;

protected:
	void DrawHealthBar(class AUnshackledCharacterBase* Character);
	void DrawManaBar(class AUnshackledCharacterBase* Character);
};

// NOTES
// - Sizes are screen pixels. ManaBarGap is the empty space between the health and mana bars.