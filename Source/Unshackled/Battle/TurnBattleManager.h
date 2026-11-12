#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Battle/TelegraphRow.h"
#include "Battle/BattleMoveRow.h"
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
	PickAction,
	PickTarget
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
	bool TryPlayerMove(int32 MemberIndex, FName EntryName, int32 TargetIndex);
	bool CanMemberAct(int32 MemberIndex) const;
	void MenuStep(int32 Horizontal, int32 Vertical);
	void MenuConfirm();
	void MenuBack();
	EBattleMenuStage GetMenuStage() const;
	int32 GetMenuMemberIndex() const;
	int32 GetMenuActionIndex() const;
	int32 GetMenuActionCount() const;
	FName GetMenuEntry(int32 Index) const;
	int32 GetMenuTargetIndex() const;
	FString GetEntryLabel(FName EntryName) const;
	FString GetEntryDescription(FName EntryName) const;
	bool IsEntryUsable(int32 MemberIndex, FName EntryName) const;
	const FBattleMoveRow* GetMove(FName MoveId) const;
	int32 FindPartyIndex(APartyMemberCharacter* Member) const;
	void ShowMessage(FString Text, FColor Color);
	void GrantExtraActionNextRound(APartyMemberCharacter* Member);
	void SkipBossTurns(int32 Count);
	void NegateBossTurns(int32 Count);
	int32 GetNegatedBossTurnsLeft() const;
	void GrantFreeAction();
	void RestartRetributionOn(APartyMemberCharacter* Starter, float ShieldFraction, float DamageCut, int32 Rounds);
	bool HasOtherMemberToAct(const APartyMemberCharacter* Member) const;
	int32 GetRetributionLength() const;
	APartyMemberCharacter* GetRetributionMember(int32 Index) const;
	int32 GetRetributionProgress() const;
	bool IsRetributionFailed() const;
	bool IsRetributionDone() const;
	bool IsUnityActive() const;
	int32 PickRandomLivingMember() const;
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
	UDataTable* MoveTable = nullptr;
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
	UPROPERTY(EditAnywhere, Category = "Battle Setup")
	float MessageDuration = 4.0f;
	UPROPERTY(EditAnywhere, Category = "Battle Debug")
	bool bDebugLoseKey = false;
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
	bool bPhaseStartPending = false;
	TArray<int32> BonusActionsNow;
	TArray<int32> BonusActionsNext;
	TArray<FPendingTelegraph> Pending;
	TArray<FName> BossRowNames;
	EBattleState State = EBattleState::PlayerPhase;
	EBattleMenuStage MenuStage = EBattleMenuStage::PickMember;
	int32 MenuMemberIndex = 0;
	int32 MenuActionIndex = 0;
	int32 MenuTargetIndex = 0;
	TArray<FName> MenuEntries;
	int32 Round = 1;
	int32 BossTurnsLeft = 0;
	int32 BossTurnsToSkip = 0;
	int32 NegatedBossTurnsLeft = 0;
	int32 FreeActions = 0;
	TArray<int32> RetributionOrder;
	int32 RetributionProgress = 0;
	bool bRetributionFailed = false;
	bool bRetributionDone = false;
	bool bUnityActive = false;
	float UnityShieldFraction = 0.0f;
	float UnityDamageCut = 0.0f;
	int32 UnityRounds = 0;
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
	void NotifyBossTurn();
	void RollRetributionOrder();
	TArray<int32> ShuffleIndices(TArray<int32> Indices) const;
	void AdvanceRetribution(int32 MemberIndex);
	void SkipDeadRetributionSteps();
	void CompleteRetribution(int32 MemberIndex);
	void GiveUnityShields();
	void MoveMemberSelection(int32 Step);
	void ResetMenuToNextMember();
	void HoldPartyInPlace();
	void StopRealTimeManaRegen();
	void RestoreManaForNewRound();
	void FlushPhaseStart();
	void NotifyPlayerPhaseStart();
	void NotifyPlayerPhaseEnd();
	void UseBonusActionIfAny(int32 MemberIndex);
	void ShowBanner(FString Text, FLinearColor Color);
	void BuildMenuEntries(int32 MemberIndex);
	void StartTargetPick();
	void MoveTargetSelection(int32 Step);
	bool NeedsAllyTarget(FName EntryName) const;
	EBattleAction GetEntryCategory(FName EntryName) const;
	bool IsBlockEntry(FName EntryName) const;
	bool IsItemEntry(FName EntryName) const;
	void EndRound();
	void AdvanceTelegraphs();
	void ResolveTelegraph(FPendingTelegraph Telegraph, bool bCountered);
	void HitMember(int32 MemberIndex, const FTelegraphRow* Row, float Multiplier);
	void RemovePending(int32 Index);
	void CheckForEnd();
	bool AllLivingMembersActed() const;
	int32 CountLivingMembers() const;
	int32 FindNextActingMember() const;
	int32 FindCounterIndex(APartyMemberCharacter* Member, EBattleAction Action) const;
	int32 PickTargetIndex(EBossTargetMode Mode) const;
	int32 FindLeaderIndex() const;
	const FTelegraphRow* GetRow(FName RowName) const;
};
