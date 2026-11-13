#include "UI/BattleHUD.h"
#include "Battle/TurnBattleManager.h"
#include "Characters/PartyMemberCharacter.h"
#include "Characters/BossCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Engine/Canvas.h"
#include "EngineUtils.h"
#include "Engine/World.h"
void ABattleHUD::DrawExtraHUD()
{
	ATurnBattleManager* Manager = FindManager();
	if (Manager == nullptr)
	{
		return;
	}
	DrawBossPanel(Manager);
	DrawPhaseLabel(Manager);
	if (bShowIncomingAttacks)
	{
		DrawTelegraphList(Manager);
	}
	DrawPartyCards(Manager);
	DrawRetributionOrder(Manager);
	DrawHintText(Manager);
}
ATurnBattleManager* ABattleHUD::FindManager() const
{
	for (TActorIterator<ATurnBattleManager> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}
void ABattleHUD::DrawCenteredText(FString Text, FLinearColor Color, float CenterX, float Top, float Scale)
{
	float TextWidth = 0.0f;
	float TextHeight = 0.0f;
	GetTextSize(Text, TextWidth, TextHeight, nullptr, Scale);
	DrawText(Text, Color, CenterX - (TextWidth / 2.0f), Top, nullptr, Scale);
}
void ABattleHUD::DrawBossPanel(ATurnBattleManager* Manager)
{
	ABossCharacter* Boss = Manager->GetBoss();
	if (Boss == nullptr)
	{
		return;
	}
	float CenterX = Canvas->SizeX / 2.0f;
	DrawCenteredText(Boss->DisplayName, FLinearColor::White, CenterX, BossTopMargin, 1.2f);
	float BarLeft = CenterX - (BossBarWidth / 2.0f);
	float BarTop = BossTopMargin + 22.0f;
	float MaxHealth = Boss->Health->MaxHealth;
	float HealthFraction = 0.0f;
	if (MaxHealth > 0.0f)
	{
		HealthFraction = Boss->Health->CurrentHealth / MaxHealth;
	}
	FLinearColor BorderColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.9f);
	DrawRect(BorderColor, BarLeft - 3.0f, BarTop - 3.0f, BossBarWidth + 6.0f, BossBarHeight + 6.0f);
	FLinearColor BackColor = FLinearColor(0.15f, 0.05f, 0.05f, 1.0f);
	DrawRect(BackColor, BarLeft, BarTop, BossBarWidth, BossBarHeight);
	FLinearColor FrontColor = FLinearColor(0.9f, 0.15f, 0.15f, 1.0f);
	DrawRect(FrontColor, BarLeft, BarTop, BossBarWidth * HealthFraction, BossBarHeight);
	FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Boss->Health->CurrentHealth), FMath::CeilToInt(MaxHealth));
	DrawCenteredText(HealthText, FLinearColor::White, CenterX, BarTop + BossBarHeight + 4.0f, 0.9f);
	int32 BossDamage = FMath::RoundToInt(Boss->Stats->GetFinal(ECharacterStat::Damage));
	int32 BossDefense = FMath::RoundToInt(Boss->Stats->GetFinal(ECharacterStat::Defense));
	FString StatText = FString::Printf(TEXT("Damage %d   Defense %d"), BossDamage, BossDefense);
	int32 Marks = Boss->Stats->GetStacks(FName("SiriusMarks"), ECharacterStat::Damage, EModifierKind::Percent);
	if (Marks > 0)
	{
		StatText = StatText + FString::Printf(TEXT("   Marks %d"), Marks);
	}
	DrawCenteredText(StatText, FLinearColor(0.75f, 0.75f, 0.75f, 1.0f), CenterX, BarTop + BossBarHeight + 22.0f, 0.8f);
}
void ABattleHUD::DrawPhaseLabel(ATurnBattleManager* Manager)
{
	EBattleState State = Manager->GetState();
	if (State == EBattleState::Won || State == EBattleState::Lost)
	{
		return;
	}
	FString Label = FString::Printf(TEXT("Round %d  -  Your turn"), Manager->GetRound());
	FLinearColor Color = FLinearColor(0.4f, 0.9f, 1.0f, 1.0f);
	if (State == EBattleState::BossPhase)
	{
		Label = FString::Printf(TEXT("Round %d  -  Boss turn"), Manager->GetRound());
		Color = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
	}
	float LabelTop = BossTopMargin + 22.0f + BossBarHeight + 42.0f;
	DrawCenteredText(Label, Color, Canvas->SizeX / 2.0f, LabelTop, 1.1f);
}
void ABattleHUD::DrawTelegraphList(ATurnBattleManager* Manager)
{
	int32 Count = Manager->GetPendingCount();
	float Left = Canvas->SizeX - TelegraphWidth - TelegraphRightMargin;
	float Top = TelegraphTopMargin;
	DrawText(TEXT("Incoming attacks (debug)"), FLinearColor::White, Left, Top - 22.0f, nullptr, 1.0f);
	if (Count == 0)
	{
		DrawText(TEXT("None"), FLinearColor(0.6f, 0.6f, 0.6f, 1.0f), Left, Top, nullptr, 1.0f);
		return;
	}
	for (int32 i = 0; i < Count; i++)
	{
		int32 ActionsLeft = Manager->GetPendingActionsLeft(i);
		FLinearColor BorderColor = FLinearColor(1.0f, 0.75f, 0.2f, 1.0f);
		if (ActionsLeft <= 1)
		{
			BorderColor = FLinearColor(1.0f, 0.15f, 0.15f, 1.0f);
		}
		DrawRect(BorderColor, Left - 3.0f, Top - 3.0f, TelegraphWidth + 6.0f, TelegraphHeight + 6.0f);
		FLinearColor BackColor = FLinearColor(0.08f, 0.06f, 0.06f, 0.95f);
		DrawRect(BackColor, Left, Top, TelegraphWidth, TelegraphHeight);
		DrawText(Manager->GetPendingName(i), FLinearColor::White, Left + 8.0f, Top + 4.0f, nullptr, 1.0f);
		FString TimingText = FString::Printf(TEXT("Lands in %d actions"), ActionsLeft);
		if (ActionsLeft == 1)
		{
			TimingText = TEXT("Lands next action!");
		}
		DrawText(TimingText, BorderColor, Left + 8.0f, Top + 22.0f, nullptr, 0.8f);
		Top = Top + TelegraphHeight + TelegraphGap + 3.0f;
	}
}
void ABattleHUD::DrawPartyCards(ATurnBattleManager* Manager)
{
	int32 Count = Manager->GetPartyCount();
	if (Count == 0)
	{
		return;
	}
	float TotalWidth = (Count * BattleCardWidth) + ((Count - 1) * BattleCardGap);
	float Left = (Canvas->SizeX - TotalWidth) / 2.0f;
	float Top = Canvas->SizeY - BattleBottomMargin - BattleCardHeight;
	for (int32 i = 0; i < Count; i++)
	{
		DrawPartyCard(Manager, i, Left, Top);
		if (i == Manager->GetMenuMemberIndex())
		{
			if (Manager->GetState() == EBattleState::PlayerPhase)
			{
				if (Manager->GetMenuStage() != EBattleMenuStage::PickMember)
				{
					DrawActionList(Manager, Left, Top);
				}
			}
		}
		Left = Left + BattleCardWidth + BattleCardGap;
	}
}
void ABattleHUD::DrawPartyCard(ATurnBattleManager* Manager, int32 MemberIndex, float Left, float Top)
{
	APartyMemberCharacter* Member = Manager->GetPartyMember(MemberIndex);
	if (Member == nullptr)
	{
		return;
	}
	bool bHovered = false;
	if (Manager->GetState() == EBattleState::PlayerPhase)
	{
		if (MemberIndex == Manager->GetMenuMemberIndex())
		{
			bHovered = true;
		}
	}
	FLinearColor BorderColor = FLinearColor(0.2f, 0.2f, 0.2f, 0.9f);
	if (bHovered)
	{
		BorderColor = FLinearColor(1.0f, 0.85f, 0.2f, 1.0f);
	}
	if (Manager->GetState() == EBattleState::PlayerPhase)
	{
		if (Manager->GetMenuStage() == EBattleMenuStage::PickTarget)
		{
			if (MemberIndex == Manager->GetMenuTargetIndex())
			{
				BorderColor = FLinearColor(0.3f, 0.9f, 1.0f, 1.0f);
			}
		}
	}
	DrawRect(BorderColor, Left - BattleCardBorder, Top - BattleCardBorder, BattleCardWidth + (BattleCardBorder * 2.0f), BattleCardHeight + (BattleCardBorder * 2.0f));
	FLinearColor BackColor = FLinearColor(0.08f, 0.08f, 0.12f, 0.95f);
	DrawRect(BackColor, Left, Top, BattleCardWidth, BattleCardHeight);
	DrawText(Member->DisplayName, FLinearColor::White, Left + 6.0f, Top + 3.0f, nullptr, 1.0f);
	float BarWidthInCard = BattleCardWidth - 12.0f;
	float MaxHealth = Member->Health->MaxHealth;
	float HealthFraction = 0.0f;
	if (MaxHealth > 0.0f)
	{
		HealthFraction = Member->Health->CurrentHealth / MaxHealth;
	}
	FString HealthText = FString::Printf(TEXT("HP %d / %d"), FMath::CeilToInt(Member->Health->CurrentHealth), FMath::CeilToInt(MaxHealth));
	DrawText(HealthText, FLinearColor::White, Left + 6.0f, Top + 21.0f, nullptr, 0.75f);
	FLinearColor BarBack = FLinearColor(0.05f, 0.05f, 0.05f, 0.9f);
	FLinearColor HealthFront = FLinearColor(0.1f, 0.9f, 0.2f, 1.0f);
	DrawRect(BarBack, Left + 6.0f, Top + 34.0f, BarWidthInCard, 7.0f);
	DrawRect(HealthFront, Left + 6.0f, Top + 34.0f, BarWidthInCard * HealthFraction, 7.0f);
	float MaxMana = Member->Stats->GetFinal(ECharacterStat::MaxMana);
	float ManaFraction = 0.0f;
	if (MaxMana > 0.0f)
	{
		ManaFraction = Member->Stats->CurrentMana / MaxMana;
	}
	FString ManaText = FString::Printf(TEXT("MP %d / %d"), FMath::FloorToInt(Member->Stats->CurrentMana), FMath::CeilToInt(MaxMana));
	DrawText(ManaText, FLinearColor::White, Left + 6.0f, Top + 43.0f, nullptr, 0.75f);
	FLinearColor ManaFront = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);
	DrawRect(BarBack, Left + 6.0f, Top + 56.0f, BarWidthInCard, 7.0f);
	DrawRect(ManaFront, Left + 6.0f, Top + 56.0f, BarWidthInCard * ManaFraction, 7.0f);
	FString StatusText = TEXT("Ready");
	FLinearColor StatusColor = FLinearColor(0.7f, 1.0f, 0.7f, 1.0f);
	if (Member->Health->IsDead())
	{
		StatusText = TEXT("Down");
		StatusColor = FLinearColor(0.9f, 0.1f, 0.1f, 1.0f);
	}
	else if (Manager->IsMemberBlocking(MemberIndex))
	{
		StatusText = TEXT("Blocking");
		StatusColor = FLinearColor(0.4f, 0.9f, 1.0f, 1.0f);
	}
	else if (!Manager->CanMemberAct(MemberIndex))
	{
		StatusText = TEXT("Done");
		StatusColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
	}
	DrawText(StatusText, StatusColor, Left + 6.0f, Top + 69.0f, nullptr, 0.85f);
	if (Member->Health->Shield > 0.0f)
	{
		FString ShieldText = FString::Printf(TEXT("Shield %d"), FMath::CeilToInt(Member->Health->Shield));
		DrawText(ShieldText, FLinearColor(0.85f, 0.85f, 1.0f, 1.0f), Left + 62.0f, Top + 69.0f, nullptr, 0.75f);
	}
	FString NoteText = Member->GetBattleCardNote();
	if (!NoteText.IsEmpty())
	{
		DrawText(NoteText, FLinearColor(1.0f, 0.8f, 0.4f, 1.0f), Left + 6.0f, Top + 87.0f, nullptr, 0.7f);
	}
	if (Member->Health->IsDead() || !Manager->CanMemberAct(MemberIndex))
	{
		FLinearColor DimColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.45f);
		DrawRect(DimColor, Left, Top, BattleCardWidth, BattleCardHeight);
	}
}
void ABattleHUD::DrawActionList(ATurnBattleManager* Manager, float CardLeft, float CardTop)
{
	int32 Count = Manager->GetMenuActionCount();
	float PanelHeight = (Count * ActionRowHeight) + 12.0f;
	float PanelTop = CardTop - PanelHeight - ActionPanelGap;
	float PanelLeft = CardLeft;
	FLinearColor BorderColor = FLinearColor(1.0f, 0.85f, 0.2f, 1.0f);
	DrawRect(BorderColor, PanelLeft - 3.0f, PanelTop - 3.0f, ActionPanelWidth + 6.0f, PanelHeight + 6.0f);
	FLinearColor BackColor = FLinearColor(0.06f, 0.06f, 0.1f, 0.97f);
	DrawRect(BackColor, PanelLeft, PanelTop, ActionPanelWidth, PanelHeight);
	int32 MemberIndex = Manager->GetMenuMemberIndex();
	for (int32 i = 0; i < Count; i++)
	{
		float RowTop = PanelTop + 6.0f + (i * ActionRowHeight);
		FName EntryName = Manager->GetMenuEntry(i);
		bool bUsable = Manager->IsEntryUsable(MemberIndex, EntryName);
		if (i == Manager->GetMenuActionIndex())
		{
			FLinearColor HighlightColor = FLinearColor(0.25f, 0.22f, 0.05f, 1.0f);
			DrawRect(HighlightColor, PanelLeft + 4.0f, RowTop, ActionPanelWidth - 8.0f, ActionRowHeight - 2.0f);
		}
		FString RowText = Manager->GetEntryLabel(EntryName);
		FLinearColor TextColor = FLinearColor::White;
		if (!bUsable)
		{
			TextColor = FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);
		}
		DrawText(RowText, TextColor, PanelLeft + 10.0f, RowTop + 3.0f, nullptr, 0.95f);
	}
}
void ABattleHUD::DrawHintText(ATurnBattleManager* Manager)
{
	if (Manager->GetState() != EBattleState::PlayerPhase)
	{
		return;
	}
	FString Hint = TEXT("Left / Right: choose member     Space: select");
	if (Manager->GetMenuStage() == EBattleMenuStage::PickAction)
	{
		Hint = TEXT("Up / Down: choose action     Space: confirm     Esc: back");
	}
	if (Manager->GetMenuStage() == EBattleMenuStage::PickTarget)
	{
		Hint = TEXT("Left / Right: choose ally     Space: confirm     Esc: back");
	}
	if (Manager->GetMenuStage() != EBattleMenuStage::PickMember)
	{
		FName EntryName = Manager->GetMenuEntry(Manager->GetMenuActionIndex());
		FString Description = Manager->GetEntryDescription(EntryName);
		float DescriptionTop = BossTopMargin + 22.0f + BossBarHeight + 68.0f;
		DrawCenteredText(Description, FLinearColor(1.0f, 0.95f, 0.7f, 1.0f), Canvas->SizeX / 2.0f, DescriptionTop, 1.0f);
	}
	DrawCenteredText(Hint, FLinearColor(0.8f, 0.8f, 0.8f, 1.0f), Canvas->SizeX / 2.0f, Canvas->SizeY - 22.0f, 0.85f);
}
void ABattleHUD::DrawRetributionOrder(ATurnBattleManager* Manager)
{
	EBattleState State = Manager->GetState();
	if (State == EBattleState::Won || State == EBattleState::Lost)
	{
		return;
	}
	int32 StepCount = Manager->GetRetributionLength();
	if (StepCount <= 0)
	{
		return;
	}
	bool bFailed = Manager->IsRetributionFailed();
	bool bDone = Manager->IsRetributionDone();
	int32 Progress = Manager->GetRetributionProgress();
	FString Label = TEXT("Retribution: act in this order");
	if (Manager->IsUnityActive())
	{
		Label = TEXT("Retribution (Unity): act in this order");
	}
	FLinearColor LabelColor = FLinearColor::White;
	if (bFailed)
	{
		Label = TEXT("Retribution failed this round");
		LabelColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
	}
	if (bDone)
	{
		Label = TEXT("Retribution complete!");
		LabelColor = FLinearColor(0.4f, 1.0f, 0.5f, 1.0f);
	}
	DrawText(Label, LabelColor, BattleRetributionLeft, BattleRetributionTop, nullptr, 0.9f);
	float BoxTop = BattleRetributionTop + 20.0f;
	for (int32 i = 0; i < StepCount; i++)
	{
		APartyMemberCharacter* Member = Manager->GetRetributionMember(i);
		if (Member != nullptr)
		{
			float BoxLeft = BattleRetributionLeft + (i * (BattleRetributionBoxWidth + BattleRetributionBoxGap));
			FLinearColor BoxColor = FLinearColor(0.15f, 0.15f, 0.2f, 0.85f);
			if (i < Progress)
			{
				BoxColor = FLinearColor(0.1f, 0.55f, 0.2f, 0.9f);
			}
			if (i == Progress)
			{
				BoxColor = FLinearColor(0.6f, 0.5f, 0.1f, 0.95f);
			}
			if (bFailed)
			{
				BoxColor = FLinearColor(0.35f, 0.1f, 0.1f, 0.85f);
			}
			if (bDone)
			{
				BoxColor = FLinearColor(0.1f, 0.55f, 0.2f, 0.9f);
			}
			DrawRect(BoxColor, BoxLeft, BoxTop, BattleRetributionBoxWidth, BattleRetributionBoxHeight);
			FString StepText = FString::Printf(TEXT("%d. %s"), i + 1, *Member->DisplayName);
			DrawText(StepText, FLinearColor::White, BoxLeft + 5.0f, BoxTop + 4.0f, nullptr, 0.8f);
		}
	}
}
