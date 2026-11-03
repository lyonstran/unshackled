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
        void ShowBanner(FString Text, FLinearColor Color, float Duration);
        void ShowPrompt(FString Text, FLinearColor Color);
        void AddMessage(FString Text, FLinearColor Color, float Duration);
        static AUnshackledHUD* FindPlayerHUD(const UObject* WorldContextObject);
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
        UPROPERTY(EditAnywhere, Category = "Banner")
        float BannerScale = 3.0f;
        UPROPERTY(EditAnywhere, Category = "Banner")
        float BannerTopFraction = 0.2f;
        UPROPERTY(EditAnywhere, Category = "Banner")
        float BannerFadeTime = 1.0f;
        UPROPERTY(EditAnywhere, Category = "Prompt")
        float PromptScale = 1.4f;
        UPROPERTY(EditAnywhere, Category = "Prompt")
        float PromptTopFraction = 0.72f;
        UPROPERTY(EditAnywhere, Category = "Prompt")
        float PromptHoldTime = 0.15f;
        UPROPERTY(EditAnywhere, Category = "Messages")
        int32 MaxMessages = 5;
        UPROPERTY(EditAnywhere, Category = "Messages")
        float MessageScale = 1.1f;
        UPROPERTY(EditAnywhere, Category = "Messages")
        float MessageLeftMargin = 24.0f;
        UPROPERTY(EditAnywhere, Category = "Messages")
        float MessageTopMargin = 120.0f;
        UPROPERTY(EditAnywhere, Category = "Messages")
        float MessageLineHeight = 22.0f;
        UPROPERTY(EditAnywhere, Category = "Messages")
        float MessageFadeTime = 0.5f;
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
        void DrawBanner();
        void DrawPrompt();
        void DrawMessages();
        void RemoveExpiredMessages();
        void DrawSwitchCard(class APartyMemberCharacter* Member, float Left, float Top, bool bSelected, bool bChoosable);
        void DrawCardStatusText(class APartyMemberCharacter* Member, float Left, float TextTop);
private:
        FString BannerText;
        FLinearColor BannerColor = FLinearColor::White;
        float BannerEndTime = 0.0f;
        FString PromptText;
        FLinearColor PromptColor = FLinearColor::White;
        float PromptEndTime = 0.0f;
        TArray<FString> MessageTexts;
        TArray<FLinearColor> MessageColors;
        TArray<float> MessageEndTimes;
};
