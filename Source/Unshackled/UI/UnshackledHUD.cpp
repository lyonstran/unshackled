#include "UI/UnshackledHUD.h"
#include "Characters/UnshackledCharacterBase.h"
#include "Characters/PartyMemberCharacter.h"
#include "Player/UnshackledPlayerController.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
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
                }
        }
        DrawSwitchMenu();
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
        int32 SelectedIndex = MyController->GetSwitchMenuIndex();
        for (int32 i = 0; i < PartyCount; i++)
        {
                APartyMemberCharacter* Member = MyController->GetPartyMember(i);
                float CardLeft = StartLeft + (i * (CardWidth + CardGap));
                bool bSelected = (i == SelectedIndex);
                DrawSwitchCard(Member, CardLeft, CardTop, bSelected);
        }
}
void AUnshackledHUD::DrawSwitchCard(APartyMemberCharacter* Member, float Left, float Top, bool bSelected)
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
        bool bDead = Member->Health->IsDead();
        if (bDead)
        {
                FLinearColor DimColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.65f);
                DrawRect(DimColor, PortraitLeft, PortraitTop, PortraitSize, PortraitSize);
                FLinearColor DownColor = FLinearColor(0.9f, 0.1f, 0.1f, 1.0f);
                DrawText(TEXT("Down"), DownColor, Left + 8.0f, Top + PortraitSize + 30.0f);
        }
        else
        {
                if (!Member->IsFollowing())
                {
                        FLinearColor LeaderColor = FLinearColor(1.0f, 0.85f, 0.2f, 1.0f);
                        DrawText(TEXT("Leading"), LeaderColor, Left + 8.0f, Top + PortraitSize + 30.0f);
                }
        }
        float MaxHealth = Member->Health->MaxHealth;
        float HealthFraction = 0.0f;
        if (MaxHealth > 0.0f)
        {
                HealthFraction = Member->Health->CurrentHealth / MaxHealth;
        }
        float BarLeft = Left + 8.0f;
        float BarTop = Top + CardHeight - 16.0f;
        float BarWidthInCard = CardWidth - 16.0f;
        FLinearColor BarBack = FLinearColor(0.05f, 0.05f, 0.05f, 0.9f);
        DrawRect(BarBack, BarLeft, BarTop, BarWidthInCard, 8.0f);
        FLinearColor BarFront = FLinearColor(0.1f, 0.9f, 0.2f, 1.0f);
        DrawRect(BarFront, BarLeft, BarTop, BarWidthInCard * HealthFraction, 8.0f);
}