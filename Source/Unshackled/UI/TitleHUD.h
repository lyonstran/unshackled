#pragma once
#include "CoreMinimal.h"
#include "UI/UnshackledHUD.h"
#include "TitleHUD.generated.h"
UCLASS()
class UNSHACKLED_API ATitleHUD : public AUnshackledHUD
{
	GENERATED_BODY()
public:
	ATitleHUD();
	UPROPERTY(EditAnywhere, Category = "Title")
	FString GameTitle = TEXT("UNSHACKLED");
	UPROPERTY(EditAnywhere, Category = "Title")
	FString Subtitle = TEXT("placeholder title screen");
	UPROPERTY(EditAnywhere, Category = "Title")
	float TitleScale = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Title")
	float SubtitleScale = 1.3f;
	UPROPERTY(EditAnywhere, Category = "Title")
	float TitleTopFraction = 0.12f;
	UPROPERTY(EditAnywhere, Category = "Title")
	float SubtitleGap = 90.0f;
	UPROPERTY(EditAnywhere, Category = "Title")
	FLinearColor BackgroundColor = FLinearColor(0.04f, 0.03f, 0.07f, 1.0f);
protected:
	virtual void DrawExtraHUD() override;
};
