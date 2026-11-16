#pragma once
#include "CoreMinimal.h"
#include "BossCharacter.h"
#include "LionCharacter.generated.h"
class APartyMemberCharacter;
UCLASS()
class UNSHACKLED_API ALionCharacter : public ABossCharacter
{
	GENERATED_BODY()
public:
	ALionCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void OnBattleRoundStart(ATurnBattleManager* Manager) override;
	virtual bool DoesMemberMoveFail(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName, EBattleAction Category) override;
	virtual void BeforeMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName) override;
	virtual void AfterMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName) override;
	virtual FString GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const override;
	virtual FString GetStatusNote(const ATurnBattleManager* Manager) const override;
protected:
	virtual float FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker) override;
	virtual float ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual int32 PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual void PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex) override;
	virtual float GetHitMultiplierFor(ATurnBattleManager* Manager, int32 MemberIndex) const override;
	void EnsureMemberArrays(const ATurnBattleManager* Manager);
	int32 GetSubjugation(int32 MemberIndex) const;
	int32 GetTotalSubjugation() const;
	void AddSubjugation(ATurnBattleManager* Manager, int32 MemberIndex, int32 Amount);
	void UpdateSubjugationEffects();
	void UpdateMemberSubjugation(int32 MemberIndex, const FBossMoveRow* Row);
	void UpdateLionDamage(const FBossMoveRow* Row);
	void CheckPride();
	bool DoesEntryTargetLion(const ATurnBattleManager* Manager, FName EntryName) const;
	float GetHealthPercent(APartyMemberCharacter* Member) const;
	bool CanExecute(const ATurnBattleManager* Manager, int32 MemberIndex) const;
	bool IsNearExecute(const ATurnBattleManager* Manager, int32 MemberIndex, float Margin) const;
	bool IsMarked(int32 MemberIndex) const;
	bool IsHuntActive(const ATurnBattleManager* Manager) const;
	bool IsHunted(const ATurnBattleManager* Manager, int32 MemberIndex) const;
	float GetHuntMultiplier(ATurnBattleManager* Manager, int32 MemberIndex) const;
	bool IsProwling(const ATurnBattleManager* Manager) const;
	bool IsChallengeActive(const ATurnBattleManager* Manager) const;
	bool IsChallenged(const ATurnBattleManager* Manager, int32 MemberIndex) const;
	bool IsSleeping() const;
	bool IsRoarOnEveryone(ATurnBattleManager* Manager) const;
	float GetCryHeal(const FBossMoveRow* Move) const;
	float ScoreSupremity(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreSovereignty(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreHunt(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreProwl(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreCry(const FBossMoveRow* Move) const;
	float ScoreChallenge(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreRoar(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreRest(const FBossMoveRow* Move) const;
	int32 FindExecuteTarget(ATurnBattleManager* Manager) const;
	int32 FindSupremityTarget(ATurnBattleManager* Manager) const;
	int32 FindSovereigntyTarget(ATurnBattleManager* Manager) const;
	int32 FindMostSubjugatedMember(ATurnBattleManager* Manager) const;
	int32 FindLeastSubjugatedMember(ATurnBattleManager* Manager) const;
	void CastSupremity(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex);
	void ExecuteMember(ATurnBattleManager* Manager, int32 TargetIndex);
	void CastSovereignty(ATurnBattleManager* Manager, int32 TargetIndex);
	void CastHunt(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex);
	void CastProwl(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastCry(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastChallenge(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex);
	void AddChallengeModifier(APartyMemberCharacter* Member, FName MoveId, ECharacterStat Stat, float Amount, int32 Turns);
	void CastRoar(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move);
	void CastRest(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move);
	void Slumber(ATurnBattleManager* Manager);
	void TickRest(ATurnBattleManager* Manager);
	void WakeFromRest(ATurnBattleManager* Manager);
	void AddAwakenedModifier(ECharacterStat Stat, float Amount);
	void TickChallengeMana(ATurnBattleManager* Manager);
	void TriggerSovereignty(ATurnBattleManager* Manager, int32 MemberIndex);
	void RecordActionStart();
	void RecordActionDamage(ATurnBattleManager* Manager, int32 MemberIndex);
	UPROPERTY()
	ATurnBattleManager* BattleManager = nullptr;
	TArray<int32> SubjugationStacks;
	TArray<int32> AppliedSubjugation;
	int32 AppliedLionDamageStacks = -1;
	bool bPrideDone = false;
	float HealthBeforeAction = 0.0f;
	float GuardBeforeAction = 0.0f;
	float PlayerPhaseDamage = 0.0f;
	int32 CryUses = 0;
	TArray<bool> SovereignMarks;
	bool bSovereignMitigation = false;
	float SovereignMitigation = 0.0f;
	int32 HuntedIndex = -1;
	int32 HuntLastRound = -1;
	int32 ProwlLastRound = -1;
	float ProwlFailChance = 0.0f;
	bool bProwlFailedThisAction = false;
	int32 ChallengedIndex = -1;
	int32 ChallengeLastRound = -1;
	bool bRestUsed = false;
	bool bAwake = true;
	int32 RestLastRound = -1;
};
