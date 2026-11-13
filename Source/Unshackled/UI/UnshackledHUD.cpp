#include "UI/UnshackledHUD.h"
#include "Characters/UnshackledCharacterBase.h"
#include "Characters/PartyMemberCharacter.h"
#include "Player/UnshackledPlayerController.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Components/RetributionComponent.h"
#include "Components/GameMenuComponent.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
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
        DrawMessages();
        DrawPrompt();
        DrawBanner();
        DrawExtraHUD();
        DrawGameMenu();
}
AUnshackledHUD* AUnshackledHUD::FindPlayerHUD(const UObject* WorldContextObject)
{
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
        if (PlayerController == nullptr)
        {
                return nullptr;
        }
        AUnshackledHUD* Hud = Cast<AUnshackledHUD>(PlayerController->GetHUD());
        return Hud;
}
void AUnshackledHUD::ShowPrompt(FString Text, FLinearColor Color)
{
        PromptText = Text;
        PromptColor = Color;
        PromptEndTime = GetWorld()->GetRealTimeSeconds() + PromptHoldTime;
}
void AUnshackledHUD::DrawPrompt()
{
        float TimeLeft = PromptEndTime - GetWorld()->GetRealTimeSeconds();
        if (TimeLeft <= 0.0f)
        {
                return;
        }
        float TextWidth = 0.0f;
        float TextHeight = 0.0f;
        GetTextSize(PromptText, TextWidth, TextHeight, nullptr, PromptScale);
        float TextLeft = (Canvas->SizeX - TextWidth) / 2.0f;
        float TextTop = Canvas->SizeY * PromptTopFraction;
        FLinearColor ShadowColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
        DrawText(PromptText, ShadowColor, TextLeft + 2.0f, TextTop + 2.0f, nullptr, PromptScale);
        DrawText(PromptText, PromptColor, TextLeft, TextTop, nullptr, PromptScale);
}
void AUnshackledHUD::AddMessage(FString Text, FLinearColor Color, float Duration)
{
        float EndTime = GetWorld()->GetRealTimeSeconds() + Duration;
        MessageTexts.Add(Text);
        MessageColors.Add(Color);
        MessageEndTimes.Add(EndTime);
        while (MessageTexts.Num() > MaxMessages)
        {
                MessageTexts.RemoveAt(0);
                MessageColors.RemoveAt(0);
                MessageEndTimes.RemoveAt(0);
        }
}
void AUnshackledHUD::RemoveExpiredMessages()
{
        float Now = GetWorld()->GetRealTimeSeconds();
        TArray<FString> KeptTexts;
        TArray<FLinearColor> KeptColors;
        TArray<float> KeptEndTimes;
        for (int32 i = 0; i < MessageTexts.Num(); i++)
        {
                if (MessageEndTimes[i] > Now)
                {
                        KeptTexts.Add(MessageTexts[i]);
                        KeptColors.Add(MessageColors[i]);
                        KeptEndTimes.Add(MessageEndTimes[i]);
                }
        }
        MessageTexts = KeptTexts;
        MessageColors = KeptColors;
        MessageEndTimes = KeptEndTimes;
}
void AUnshackledHUD::DrawMessages()
{
        RemoveExpiredMessages();
        float Now = GetWorld()->GetRealTimeSeconds();
        for (int32 i = 0; i < MessageTexts.Num(); i++)
        {
                float TimeLeft = MessageEndTimes[i] - Now;
                float Alpha = 1.0f;
                if (TimeLeft < MessageFadeTime)
                {
                        Alpha = TimeLeft / MessageFadeTime;
                }
                FLinearColor ShadowColor = FLinearColor(0.0f, 0.0f, 0.0f, Alpha);
                FLinearColor MainColor = MessageColors[i];
                MainColor.A = Alpha;
                float LineTop = MessageTopMargin + (i * MessageLineHeight);
                DrawText(MessageTexts[i], ShadowColor, MessageLeftMargin + 1.0f, LineTop + 1.0f, nullptr, MessageScale);
                DrawText(MessageTexts[i], MainColor, MessageLeftMargin, LineTop, nullptr, MessageScale);
        }
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
void AUnshackledHUD::DrawExtraHUD()
{
}
void AUnshackledHUD::DrawMenuText(FString Text, FLinearColor Color, float CenterX, float Top, float Scale)
{
        if (Text.IsEmpty())
        {
                return;
        }
        float TextWidth = 0.0f;
        float TextHeight = 0.0f;
        GetTextSize(Text, TextWidth, TextHeight, nullptr, Scale);
        float TextLeft = CenterX - (TextWidth / 2.0f);
        FLinearColor ShadowColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
        DrawText(Text, ShadowColor, TextLeft + 2.0f, Top + 2.0f, nullptr, Scale);
        DrawText(Text, Color, TextLeft, Top, nullptr, Scale);
}
void AUnshackledHUD::DrawGameMenu()
{
        APlayerController* OwningController = GetOwningPlayerController();
        if (OwningController == nullptr)
        {
                return;
        }
        UGameMenuComponent* GameMenu = OwningController->FindComponentByClass<UGameMenuComponent>();
        if (GameMenu == nullptr)
        {
                return;
        }
        if (!GameMenu->IsOpen())
        {
                return;
        }
        if (GameMenu->Kind == EGameMenuKind::Pause)
        {
                FLinearColor BackdropColor = FLinearColor(0.0f, 0.0f, 0.0f, MenuBackdropAlpha);
                DrawRect(BackdropColor, 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);
        }
        float CenterX = Canvas->SizeX / 2.0f;
        float Top = Canvas->SizeY * MenuTopFraction;
        DrawMenuText(GameMenu->GetMenuTitle(), FLinearColor::White, CenterX, Top, MenuTitleScale);
        float RowsTop = Top + (MenuRowHeight * 2.0f);
        if (GameMenu->IsConfirming())
        {
                DrawConfirmRows(GameMenu, CenterX, RowsTop);
        }
        else
        {
                DrawMenuRows(GameMenu, CenterX, RowsTop);
        }
        float StatusTop = RowsTop + (MenuRowHeight * 5.0f);
        FLinearColor StatusColor = FLinearColor(0.5f, 1.0f, 0.5f, 1.0f);
        DrawMenuText(GameMenu->GetStatusText(), StatusColor, CenterX, StatusTop, MenuOptionScale);
        float HintTop = Canvas->SizeY - 50.0f;
        FLinearColor HintColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
        DrawMenuText(TEXT("Arrows / D-pad: move     Space / A: choose     Esc / B: back"), HintColor, CenterX, HintTop, MenuHintScale);
}
void AUnshackledHUD::DrawMenuRows(UGameMenuComponent* GameMenu, float CenterX, float Top)
{
        int32 SelectedIndex = GameMenu->GetSelectedIndex();
        for (int32 i = 0; i < GameMenu->GetOptionCount(); i++)
        {
                float RowTop = Top + (MenuRowHeight * i);
                bool bSelected = false;
                if (i == SelectedIndex)
                {
                        bSelected = true;
                }
                DrawMenuRow(GameMenu->GetOptionLabel(i), CenterX, RowTop, bSelected, GameMenu->IsOptionEnabled(i));
        }
}
void AUnshackledHUD::DrawConfirmRows(UGameMenuComponent* GameMenu, float CenterX, float Top)
{
        FLinearColor QuestionColor = FLinearColor(1.0f, 0.85f, 0.4f, 1.0f);
        DrawMenuText(GameMenu->GetConfirmQuestion(), QuestionColor, CenterX, Top, MenuOptionScale);
        bool bYesSelected = GameMenu->IsConfirmYesSelected();
        float YesTop = Top + (MenuRowHeight * 1.5f);
        float NoTop = Top + (MenuRowHeight * 2.5f);
        DrawMenuRow(TEXT("Yes"), CenterX, YesTop, bYesSelected, true);
        DrawMenuRow(TEXT("No"), CenterX, NoTop, !bYesSelected, true);
}
void AUnshackledHUD::DrawMenuRow(FString Label, float CenterX, float Top, bool bSelected, bool bEnabled)
{
        FLinearColor TextColor = FLinearColor::White;
        if (!bEnabled)
        {
                TextColor = FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
        }
        if (bSelected)
        {
                FLinearColor HighlightColor = FLinearColor(1.0f, 0.85f, 0.2f, 0.25f);
                float HighlightLeft = CenterX - (MenuPanelWidth / 2.0f);
                DrawRect(HighlightColor, HighlightLeft, Top - 4.0f, MenuPanelWidth, MenuRowHeight - 4.0f);
                TextColor = FLinearColor(1.0f, 0.85f, 0.2f, 1.0f);
        }
        DrawMenuText(Label, TextColor, CenterX, Top, MenuOptionScale);
}
