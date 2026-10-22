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
        UPROPERTY(EditAnywhere, Category = "Switch Menu")
        float CardWidth = 110.0f;
        UPROPERTY(EditAnywhere, Category = "Switch Menu")
        float CardHeight = 176.0f;
        UPROPERTY(EditAnywhere, Category = "Switch Menu")
        float CardGap = 14.0f;
        UPROPERTY(EditAnywhere, Category = "Switch Menu")
        float MenuBottomMargin = 40.0f;
        UPROPERTY(EditAnywhere, Category = "Switch Menu")
        float CardBorder = 4.0f;
        UPROPERTY(EditAnywhere, Category = "Retribution")
        float RetributionStepWidth = 110.0f;
        UPROPERTY(EditAnywhere, Category = "Retribution")
        float RetributionStepHeight = 28.0f;
        UPROPERTY(EditAnywhere, Category = "Retribution")
        float RetributionStepGap = 8.0f;
        UPROPERTY(EditAnywhere, Category = "Retribution")
        float RetributionRightMargin = 24.0f;
        UPROPERTY(EditAnywhere, Category = "Retribution")
        float RetributionBottomMargin = 24.0f;
protected:
        void DrawHealthBar(class AUnshackledCharacterBase* Character);
        void DrawManaBar(class AUnshackledCharacterBase* Character);
        void DrawGroundedTag(class APartyMemberCharacter* Member);
        void DrawSwitchMenu();
        void DrawRetributionTracker();
        void DrawSwitchCard(class APartyMemberCharacter* Member, float Left, float Top, bool bSelected, bool bChoosable);
        void DrawCardStatusText(class APartyMemberCharacter* Member, float Left, float TextTop);
};
