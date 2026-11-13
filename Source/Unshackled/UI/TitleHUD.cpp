#include "UI/TitleHUD.h"
#include "Engine/Canvas.h"
ATitleHUD::ATitleHUD()
{
	MenuTopFraction = 0.38f;
}
void ATitleHUD::DrawExtraHUD()
{
	DrawRect(BackgroundColor, 0.0f, 0.0f, Canvas->SizeX, Canvas->SizeY);
	float CenterX = Canvas->SizeX / 2.0f;
	float TitleTop = Canvas->SizeY * TitleTopFraction;
	FLinearColor TitleColor = FLinearColor(1.0f, 0.85f, 0.2f, 1.0f);
	DrawMenuText(GameTitle, TitleColor, CenterX, TitleTop, TitleScale);
	float SubtitleTop = TitleTop + SubtitleGap;
	FLinearColor SubtitleColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f);
	DrawMenuText(Subtitle, SubtitleColor, CenterX, SubtitleTop, SubtitleScale);
}
