#pragma once
#include "CoreMinimal.h"
#include "UI/UnshackledHUD.h"
#include "BattleHUD.generated.h"
class ATurnBattleManager;
UCLASS()
class UNSHACKLED_API ABattleHUD : public AUnshackledHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
	UPROPERTY(EditAnywhere, Category = "Battle Cards")
	float BattleCardWidth = 120.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Cards")
	float BattleCardHeight = 92.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Cards")
	float BattleCardGap = 10.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Cards")
	float BattleCardBorder = 3.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Cards")
	float BattleBottomMargin = 30.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Actions")
	float ActionPanelWidth = 170.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Actions")
	float ActionRowHeight = 24.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Actions")
	float ActionPanelGap = 10.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Telegraphs")
	float TelegraphWidth = 220.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Telegraphs")
	float TelegraphHeight = 40.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Telegraphs")
	float TelegraphGap = 6.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Telegraphs")
	float TelegraphRightMargin = 16.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Telegraphs")
	float TelegraphTopMargin = 120.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Debug")
	bool bShowIncomingAttacks = true;
	UPROPERTY(EditAnywhere, Category = "Battle Boss")
	float BossBarWidth = 400.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Boss")
	float BossBarHeight = 14.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Boss")
	float BossTopMargin = 14.0f;
protected:
	ATurnBattleManager* FindManager() const;
	void DrawBossPanel(ATurnBattleManager* Manager);
	void DrawPhaseLabel(ATurnBattleManager* Manager);
	void DrawTelegraphList(ATurnBattleManager* Manager);
	void DrawPartyCards(ATurnBattleManager* Manager);
	void DrawPartyCard(ATurnBattleManager* Manager, int32 MemberIndex, float Left, float Top);
	void DrawActionList(ATurnBattleManager* Manager, float CardLeft, float CardTop);
	void DrawHintText(ATurnBattleManager* Manager);
	void DrawCenteredText(FString Text, FLinearColor Color, float CenterX, float Top, float Scale);
};
