#pragma once
#include "CoreMinimal.h"
#include "BossCharacter.h"
#include "GoatCharacter.generated.h"
class ABattleSummonCharacter;
class APartyMemberCharacter;
UCLASS()
class UNSHACKLED_API AGoatCharacter : public ABossCharacter
{
	GENERATED_BODY()
public:
	AGoatCharacter();
	virtual void OnBattleRoundStart(ATurnBattleManager* Manager) override;
	virtual bool DoesMemberMoveFail(ATurnBattleManager* Manager, int32 MemberIndex, EBattleAction Category) override;
	virtual bool IsMemberMoveLocked(const ATurnBattleManager* Manager, int32 MemberIndex, FName MoveId) const override;
	virtual FString GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const override;
	UPROPERTY(EditAnywhere, Category = "Goat")
	TSubclassOf<ABattleSummonCharacter> GuardClass;
	UPROPERTY(EditAnywhere, Category = "Goat")
	float DoubtChancePerStack = 0.02f;
	UPROPERTY(EditAnywhere, Category = "Goat")
	int32 DoubtMaxStacks = 50;
protected:
	virtual void Die() override;
	virtual float ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual int32 PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual void PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex) override;
	virtual float TakeNextHitMultiplier() override;
	float ScorePummel(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreChannel(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreWhispers(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const;
	float ScoreEvoke(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreGrasp(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreImprison(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreTether(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	int32 FindLowestHealthMember(ATurnBattleManager* Manager) const;
	int32 FindHighestManaMember(ATurnBattleManager* Manager) const;
	int32 FindHighestDefenseMemberBelowMax(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const;
	int32 FindMostBuffedMember(ATurnBattleManager* Manager) const;
	int32 FindImprisonTarget(ATurnBattleManager* Manager) const;
	TArray<FName> GetLockableMoves(const ATurnBattleManager* Manager, APartyMemberCharacter* Member) const;
	bool CanAffordUltimate(ATurnBattleManager* Manager, APartyMemberCharacter* Member) const;
	bool IsCageWaiting(ATurnBattleManager* Manager) const;
	float HitWithChannel(ATurnBattleManager* Manager, int32 MemberIndex, const FBossMoveRow* Move);
	void CastChannel(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastEvoke(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastGrasp(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex);
	void CastImprison(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex);
	void CastTether(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex);
	void AddDoubtToParty(ATurnBattleManager* Manager);
	int32 GetDoubtStacks(int32 MemberIndex) const;
	void UpdateTether(ATurnBattleManager* Manager);
	void CollectTetherShares(ATurnBattleManager* Manager);
	void BreakTether(ATurnBattleManager* Manager, bool bDealDamage);
	void ClearTetherDivert();
	TArray<int32> DoubtStacks;
	bool bChannelReady = false;
	float ChannelMultiplier = 1.0f;
	int32 CagedMemberIndex = -1;
	FName CagedMoveId;
	int32 CageRound = 0;
	UPROPERTY()
	APartyMemberCharacter* TetherMember = nullptr;
	FName TetherMoveId;
	int32 TetherBreakRound = 0;
};