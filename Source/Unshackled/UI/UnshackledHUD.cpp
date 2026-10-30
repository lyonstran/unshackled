#include "UI/UnshackledHUD.h"
#include "Characters/UnshackledCharacterBase.h"
#include "Characters/PartyMemberCharacter.h"
#include "Player/UnshackledPlayerController.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Components/RetributionComponent.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "Engine/World.h"
void AUnshackledHUD::DrawHUD()
{
        Super::DrawHUD();
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Character = *It;
                if (Character != nullptr)
                {
                        DrawHealthBar(Character);
                        DrawManaBar(Character);
                        APartyMemberCharacter* PartyMember = Cast<APartyMemberCharacter>(Character);
                        if (PartyMember != nullptr)
                        {
                                DrawGroundedTag(PartyMember);
                        }
                }
        }
        DrawRetributionTracker();
        DrawSwitchMenu();
        DrawBanner();
}
void AUnshackledHUD::ShowBanner(FString Text, FLinearColor Color, float Duration)
{
        BannerText = Text;
        BannerColor = Color;
        BannerEndTime = GetWorld()->GetRealTimeSeconds() + Duration;
}
void AUnshackledHUD::DrawBanner()
{
        float TimeLeft = BannerEndTime - GetWorld()->GetRealTimeSeconds();
        if (TimeLeft <= 0.0f)
        {
                return;
        }
        float Alpha = 1.0f;
        if (TimeLeft < BannerFadeTime)
        {
                Alpha = TimeLeft / BannerFadeTime;
        }
        float TextWidth = 0.0f;
        float TextHeight = 0.0f;
        GetTextSize(BannerText, TextWidth, TextHeight, nullptr, BannerScale);
        float TextLeft = (Canvas->SizeX - TextWidth) / 2.0f;
        float TextTop = Canvas->SizeY * BannerTopFraction;
        FLinearColor ShadowColor = FLinearColor(0.0f, 0.0f, 0.0f, Alpha);
        FLinearColor MainColor = BannerColor;
        MainColor.A = Alpha;
        DrawText(BannerText, ShadowColor, TextLeft + 2.0f, TextTop + 2.0f, nullptr, BannerScale);
        DrawText(BannerText, MainColor, TextLeft, TextTop, nullptr, BannerScale);
}
void AUnshackledHUD::DrawRetributionTracker()
{
        AUnshackledPlayerController* MyController = Cast<AUnshackledPlayerController>(GetOwningPlayerController());
        if (MyController == nullptr)
        {
                return;
        }
        URetributionComponent* Retribution = MyController->Retribution;
        if (Retribution == nullptr)
        {
                return;
        }
        float CooldownLeft = Retribution->GetCooldownRemaining();
        if (CooldownLeft > 0.0f)
        {
                int32 SecondsLeft = FMath::CeilToInt(CooldownLeft);
                FString CooldownText = FString::Printf(TEXT("Retribution recharging %d"), SecondsLeft);
                FLinearColor CooldownColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
                float CooldownLeftEdge = Canvas->SizeX - RetributionRightMargin - 190.0f;
                float CooldownTop = Canvas->SizeY - RetributionBottomMargin - RetributionStepHeight + 6.0f;
                DrawText(CooldownText, CooldownColor, CooldownLeftEdge, CooldownTop);
                return;
        }
        int32 StepCount = Retribution->GetSequenceLength();
        if (StepCount <= 0)
        {
                return;
        }
        float TotalWidth = (StepCount * RetributionStepWidth) + ((StepCount - 1) * RetributionStepGap);
        float StartLeft = Canvas->SizeX - RetributionRightMargin - TotalWidth;
        float StepTop = Canvas->SizeY - RetributionBottomMargin - RetributionStepHeight;
        float LabelTop = StepTop - 20.0f;
        FLinearColor TextColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
        DrawText(TEXT("Retribution: switch in this order"), TextColor, StartLeft, LabelTop);
        int32 Progress = Retribution->GetProgress();
        for (int32 i = 0; i < StepCount; i++)
        {
                APartyMemberCharacter* Member = Retribution->GetSequenceMember(i);
                if (Member != nullptr)
                {
                        float StepLeft = StartLeft + (i * (RetributionStepWidth + RetributionStepGap));
                        FLinearColor BoxColor = FLinearColor(0.15f, 0.15f, 0.2f, 0.85f);
                        if (i < Progress)
                        {
                                BoxColor = FLinearColor(0.1f, 0.55f, 0.2f, 0.9f);
                        }
                        if (i == Progress)
                        {
                                BoxColor = FLinearColor(0.6f, 0.5f, 0.1f, 0.95f);
                        }
                        DrawRect(BoxColor, StepLeft, StepTop, RetributionStepWidth, RetributionStepHeight);
                        FString StepText = FString::Printf(TEXT("%d. %s"), i + 1, *Member->DisplayName);
                        DrawText(StepText, TextColor, StepLeft + 8.0f, StepTop + 6.0f);
                }
        }
}
void AUnshackledHUD::DrawHealthBar(AUnshackledCharacterBase* Character)
{
        if (Character->Health->IsDead())
        {
                return;
        }
        float MaxHealth = Character->Health->MaxHealth;
        if (MaxHealth <= 0.0f)
        {
                return;
        }
        float HealthFraction = Character->Health->CurrentHealth / MaxHealth;
        FVector WorldPosition = Character->GetActorLocation() + FVector(BarWorldOffset, 0.0f, 0.0f);
        FVector ScreenPosition = Project(WorldPosition);
        float BarLeft = ScreenPosition.X - (BarWidth / 2.0f);
        float BarTop = ScreenPosition.Y;
        FLinearColor BackColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.8f);
        DrawRect(BackColor, BarLeft, BarTop, BarWidth, BarHeight);
        FLinearColor FrontColor = FLinearColor(0.1f, 0.9f, 0.2f, 1.0f);
        if (Character->bIsEnemy)
        {
                FrontColor = FLinearColor(0.9f, 0.1f, 0.1f, 1.0f);
        }
        DrawRect(FrontColor, BarLeft, BarTop, BarWidth * HealthFraction, BarHeight);
}
void AUnshackledHUD::DrawManaBar(AUnshackledCharacterBase* Character)
{
        if (!Character->Stats->bHasMana)
        {
                return;
        }
        if (Character->Health->IsDead())
        {
                return;
        }
        float MaxMana = Character->Stats->GetFinal(ECharacterStat::MaxMana);
        if (MaxMana <= 0.0f)
        {
                return;
        }
        float ManaFraction = Character->Stats->CurrentMana / MaxMana;
        FVector WorldPosition = Character->GetActorLocation() + FVector(BarWorldOffset, 0.0f, 0.0f);
        FVector ScreenPosition = Project(WorldPosition);
        float BarLeft = ScreenPosition.X - (BarWidth / 2.0f);
        float BarTop = ScreenPosition.Y + BarHeight + ManaBarGap;
        FLinearColor BackColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.8f);
        DrawRect(BackColor, BarLeft, BarTop, BarWidth, ManaBarHeight);
        FLinearColor FrontColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);
        DrawRect(FrontColor, BarLeft, BarTop, BarWidth * ManaFraction, ManaBarHeight);
}
void AUnshackledHUD::DrawGroundedTag(APartyMemberCharacter* Member)
{
        if (!Member->IsGrounded())
        {
                return;
        }
        if (Member->Health->IsDead())
        {
                return;
        }
        FVector WorldPosition = Member->GetActorLocation() + FVector(BarWorldOffset, 0.0f, 0.0f);
        FVector ScreenPosition = Project(WorldPosition);
        FLinearColor GroundedColor = FLinearColor(0.4f, 1.0f, 0.5f, 1.0f);
        DrawText(TEXT("Grounded"), GroundedColor, ScreenPosition.X - 28.0f, ScreenPosition.Y - 18.0f);
}
void AUnshackledHUD::DrawSwitchMenu()
{
        AUnshackledPlayerController* MyController = Cast<AUnshackledPlayerController>(GetOwningPlayerController());
        if (MyController == nullptr)
        {
                return;
        }
        if (!MyController->IsSwitchMenuOpen())
        {
                return;
        }
        int32 PartyCount = MyController->GetPartyCount();
        if (PartyCount <= 0)
        {
                return;
        }
        float TotalWidth = (PartyCount * CardWidth) + ((PartyCount - 1) * CardGap);
        float StartLeft = (Canvas->SizeX - TotalWidth) / 2.0f;
        float CardTop = Canvas->SizeY - CardHeight - MenuBottomMargin;
        FLinearColor TitleColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
        FString MenuTitle = MyController->GetMenuTitle();
        DrawText(MenuTitle, TitleColor, StartLeft, CardTop - 32.0f, nullptr, 1.4f);
        int32 SelectedIndex = MyController->GetSwitchMenuIndex();
        for (int32 i = 0; i < PartyCount; i++)
        {
                APartyMemberCharacter* Member = MyController->GetPartyMember(i);
                float CardLeft = StartLeft + (i * (CardWidth + CardGap));
                bool bSelected = (i == SelectedIndex);
                bool bChoosable = MyController->CanChooseMember(Member);
                DrawSwitchCard(Member, CardLeft, CardTop, bSelected, bChoosable);
        }
}
void AUnshackledHUD::DrawSwitchCard(APartyMemberCharacter* Member, float Left, float Top, bool bSelected, bool bChoosable)
{
        if (Member == nullptr)
        {
                return;
        }
        FLinearColor BorderColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.9f);
        if (bSelected)
        {
                BorderColor = FLinearColor(1.0f, 0.85f, 0.2f, 1.0f);
        }
        DrawRect(BorderColor, Left - CardBorder, Top - CardBorder, CardWidth + (CardBorder * 2.0f), CardHeight + (CardBorder * 2.0f));
        FLinearColor BackColor = FLinearColor(0.08f, 0.08f, 0.12f, 0.95f);
        DrawRect(BackColor, Left, Top, CardWidth, CardHeight);
        float PortraitSize = CardWidth - 16.0f;
        float PortraitLeft = Left + 8.0f;
        float PortraitTop = Top + 8.0f;
        if (Member->Portrait != nullptr)
        {
                DrawTexture(Member->Portrait, PortraitLeft, PortraitTop, PortraitSize, PortraitSize, 0.0f, 0.0f, 1.0f, 1.0f);
        }
        else
        {
                FLinearColor PlaceholderColor = FLinearColor(0.3f, 0.35f, 0.5f, 1.0f);
                DrawRect(PlaceholderColor, PortraitLeft, PortraitTop, PortraitSize, PortraitSize);
        }
        FLinearColor TextColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
        DrawText(Member->DisplayName, TextColor, Left + 8.0f, Top + PortraitSize + 12.0f);
        DrawCardStatusText(Member, Left, Top + PortraitSize + 30.0f);
        float MaxHealth = Member->Health->MaxHealth;
        float HealthFraction = 0.0f;
        if (MaxHealth > 0.0f)
        {
                HealthFraction = Member->Health->CurrentHealth / MaxHealth;
        }
        float BarLeft = Left + 8.0f;
        float BarWidthInCard = CardWidth - 16.0f;
        float ManaBarTop = Top + CardHeight - 28.0f;
        float HealthBarTop = Top + CardHeight - 16.0f;
        float MaxMana = Member->Stats->GetFinal(ECharacterStat::MaxMana);
        float ManaFraction = 0.0f;
        if (MaxMana > 0.0f)
        {
                ManaFraction = Member->Stats->CurrentMana / MaxMana;
        }
        FLinearColor BarBack = FLinearColor(0.05f, 0.05f, 0.05f, 0.9f);
        FLinearColor ManaFront = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);
        DrawRect(BarBack, BarLeft, ManaBarTop, BarWidthInCard, 8.0f);
        DrawRect(ManaFront, BarLeft, ManaBarTop, BarWidthInCard * ManaFraction, 8.0f);
        FLinearColor HealthFront = FLinearColor(0.1f, 0.9f, 0.2f, 1.0f);
        DrawRect(BarBack, BarLeft, HealthBarTop, BarWidthInCard, 8.0f);
        DrawRect(HealthFront, BarLeft, HealthBarTop, BarWidthInCard * HealthFraction, 8.0f);
        if (Member->Health->IsDead() || !bChoosable)
        {
                FLinearColor DimColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.6f);
                DrawRect(DimColor, Left, Top, CardWidth, CardHeight);
        }
}
void AUnshackledHUD::DrawCardStatusText(APartyMemberCharacter* Member, float Left, float TextTop)
{
        float TextLeft = Left + 8.0f;
        if (Member->Health->IsDead())
        {
                FLinearColor DownColor = FLinearColor(0.9f, 0.1f, 0.1f, 1.0f);
                DrawText(TEXT("Down"), DownColor, TextLeft, TextTop);
                return;
        }
        if (Member->IsPlayerControlled())
        {
                FLinearColor LeaderColor = FLinearColor(1.0f, 0.85f, 0.2f, 1.0f);
                DrawText(TEXT("Leading"), LeaderColor, TextLeft, TextTop);
                return;
        }
        if (Member->IsGrounded())
        {
                FLinearColor GroundedColor = FLinearColor(0.4f, 1.0f, 0.5f, 1.0f);
                DrawText(TEXT("Grounded"), GroundedColor, TextLeft, TextTop);
                return;
        }
        float CooldownLeft = Member->GetGroundCooldownRemaining();
        if (CooldownLeft > 0.0f)
        {
                int32 SecondsLeft = FMath::CeilToInt(CooldownLeft);
                FString CooldownText = FString::Printf(TEXT("Cooldown %d"), SecondsLeft);
                FLinearColor CooldownColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
                DrawText(CooldownText, CooldownColor, TextLeft, TextTop);
        }
}
