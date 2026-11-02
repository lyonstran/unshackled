#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Battle/TelegraphRow.h"
#include "TurnBattleManager.generated.h"
class APartyMemberCharacter;
class ABossCharacter;
USTRUCT()
struct FBattlePartySlot
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Battle")
	FName CharacterId;
	UPROPERTY(EditAnywhere, Category = "Battle")
	TSubclassOf<APartyMemberCharacter> MemberClass;
};
USTRUCT()
struct FBattleBossSlot
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Battle")
	int32 Floor = 1;
	UPROPERTY(EditAnywhere, Category = "Battle")
	TSubclassOf<ABossCharacter> BossClass;
};
enum class EBattleState
{
	PlayerPhase,
	BossPhase,
	Won,
	Lost
};
enum class EBattleMenuStage
{
	PickMember,
	PickAction
};
struct FPendingTelegraph
{
	FName RowName;
	int32 ActionsLeft = 0;
	int32 TargetIndex = -1;
};
UCLASS()
class UNSHACKLED_API ATurnBattleManager : public AActor
{
	GENERATED_BODY()
public:
	ATurnBattleManager();
	virtual void Tick(float DeltaTime) override;
	bool TryPlayerAction(int32 MemberIndex, EBattleAction Action);
	bool CanMemberAct(int32 MemberIndex) const;
	void MenuStep(int32 Horizontal, int32 Vertical);
	void MenuConfirm();
	void MenuBack();
	EBattleMenuStage GetMenuStage() const;
	int32 GetMenuMemberIndex() const;
	int32 GetMenuActionIndex() const;
	int32 GetMenuActionCount() const;
	EBattleAction GetMenuAction(int32 Index) const;
	FString GetActionLabel(EBattleAction Action) const;
	bool IsActionUsable(int32 MemberIndex, EBattleAction Action) const;
	EBattleState GetState() const;
	int32 GetRound() const;
	bool IsMemberBlocking(int32 MemberIndex) const;
	int32 GetPartyCount() const;
	APartyMemberCharacter* GetPartyMember(int32 Index) const;
	ABossCharacter* GetBoss() const;
	int32 GetPendingCount() const;
	FString GetPendingName(int32 Index) const;
	int32 GetPendingActionsLeft(int32 Index) const;
	UPROPERTY(EditAnywhere, Category = "Battle")
	UDataTable* TelegraphTable = nullptr;
	UPROPERTY(EditAnywhere, Category = "Battle")
	float BossTurnDelay = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Battle")
	float ExtraTurnHealthFraction = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Battle")
	float AttackScale = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Battle")
	float SpecialScale = 2.0f;
	UPROPERTY(EditAnywhere, Category = "Battle")
	float UltimateScale = 4.0f;
	UPROPERTY(EditAnywhere, Category = "Battle")
	float ManaRestoreFraction = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Battle Setup")
	TArray<FBattlePartySlot> PartySlots;
	UPROPERTY(EditAnywhere, Category = "Battle Setup")
	TArray<FBattleBossSlot> BossSlots;
	UPROPERTY(EditAnywhere, Category = "Battle Setup")
	float PartySpacing = 160.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Setup")
	FVector BossOffset = FVector(700.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, Category = "Battle Setup")
	FName OverworldLevelName = FName("Level_Prototype");
	UPROPERTY(EditAnywhere, Category = "Battle Setup")
	float EndDelay = 4.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Setup")
	float WinInputDelay = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Debug")
	bool bDebugLoseKey = true;
	UPROPERTY(EditAnywhere, Category = "Battle Debug")
	bool bSpawnWholePartyForTesting = false;
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	TArray<APartyMemberCharacter*> Party;
	UPROPERTY()
	ABossCharacter* Boss = nullptr;
	TArray<bool> HasActed;
	TArray<bool> IsBlocking;
	TArray<FPendingTelegraph> Pending;
	TArray<FName> BossRowNames;
	EBattleState State = EBattleState::PlayerPhase;
	EBattleMenuStage MenuStage = EBattleMenuStage::PickMember;
	int32 MenuMemberIndex = 0;
	int32 MenuActionIndex = 0;
	int32 Round = 1;
	int32 BossTurnsLeft = 0;
	float BossTurnTimer = 0.0f;
	float EndTimer = 0.0f;
	bool bEndHandled = false;
	bool bWaitingForContinue = false;
	void CheckDebugKeys();
	void SpawnMissingParty();
	void SpawnBossIfMissing();
	FVector GetBattleOrigin() const;
	APartyMemberCharacter* FindMemberByRow(FName CharacterId) const;
	void TickEnd(float DeltaTime);
	void LeaveAfterWin();
	void LeaveAfterLoss();
	void ReviveFallenMembers();
	void FindParty();
	void FindBoss();
	void BuildBossRows();
	void StartPlayerPhase();
	void StartBossPhase();
	void TickBossPhase(float DeltaTime);
	void DoBossTurn();
	void MoveMemberSelection(int32 Step);
	void ResetMenuToNextMember();
	void HoldPartyInPlace();
	void StopRealTimeManaRegen();
	void RestoreManaForNewRound();
	void ShowMessage(FString Text, FColor Color);
	void ShowBanner(FString Text, FLinearColor Color);
	void ApplyActionEffect(int32 MemberIndex, EBattleAction Action);
	void AdvanceTelegraphs();
	void ResolveTelegraph(FPendingTelegraph Telegraph, bool bCountered);
	void HitMember(int32 MemberIndex, const FTelegraphRow* Row, float Multiplier);
	void RemovePending(int32 Index);
	void CheckForEnd();
	bool AllLivingMembersActed() const;
	int32 CountLivingMembers() const;
	int32 FindNextActingMember() const;
	int32 FindCounterIndex(APartyMemberCharacter* Member, EBattleAction Action) const;
	int32 PickRandomLivingMember() const;
	int32 PickTargetIndex(EBossTargetMode Mode) const;
	int32 FindLeaderIndex() const;
	const FTelegraphRow* GetRow(FName RowName) const;
};
