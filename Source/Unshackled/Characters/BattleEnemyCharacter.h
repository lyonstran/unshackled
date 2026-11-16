#pragma once
#include "CoreMinimal.h"
#include "UnshackledCharacterBase.h"
#include "Battle/BossMoveRow.h"
#include "Battle/TelegraphRow.h"
#include "Stats/StatTypes.h"
#include "BattleEnemyCharacter.generated.h"
class ATurnBattleManager;
UCLASS()
class UNSHACKLED_API ABattleEnemyCharacter : public AUnshackledCharacterBase
{
	GENERATED_BODY()
public:
	ABattleEnemyCharacter();
	virtual void OnBattleRoundStart(ATurnBattleManager* Manager);
	virtual bool DoesMemberMoveFail(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName, EBattleAction Category);
	virtual bool IsMemberMoveLocked(const ATurnBattleManager* Manager, int32 MemberIndex, FName MoveId) const;
	virtual FString GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const;
	virtual void BeforeMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName);
	virtual void AfterMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName);
	virtual FString GetStatusNote(const ATurnBattleManager* Manager) const;
	virtual void OnBossPhaseStart(ATurnBattleManager* Manager);
	virtual void OnRetributionCompleted(ATurnBattleManager* Manager, int32 MemberIndex);
	virtual int32 GetBossTurnCount(ATurnBattleManager* Manager, int32 DefaultTurns);
	bool TakeTurnFromMoveTable(ATurnBattleManager* Manager);
	FName GetMoveSetKey() const;
	UPROPERTY(EditAnywhere, Category = "Battle Enemy")
	FString DisplayName = TEXT("Enemy");
	UPROPERTY(EditAnywhere, Category = "Battle Enemy")
	FName MoveSetId;
	UPROPERTY(EditAnywhere, Category = "Battle Enemy")
	float BlunderChance = 0.2f;
	UPROPERTY(EditAnywhere, Category = "Battle Enemy")
	bool bPicksMovesRandomly = false;
	UPROPERTY(EditAnywhere, Category = "Battle Enemy")
	float BlockMultiplier = 0.5f;
protected:
	virtual float ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move);
	virtual int32 PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move);
	virtual void PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex);
	virtual float GetHitMultiplierFor(ATurnBattleManager* Manager, int32 MemberIndex) const;
	int32 PickRandomTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const;
	float HitMemberWithMove(ATurnBattleManager* Manager, int32 MemberIndex, const FBossMoveRow* Move, float Multiplier);
	float HitMemberWithBonus(ATurnBattleManager* Manager, int32 MemberIndex, const FBossMoveRow* Move, float Multiplier, float BonusDamage);
	void ApplyMoveModifier(AUnshackledCharacterBase* Target, FName MoveId, const FBossMoveRow* Move);
	float GetPercentBase(AUnshackledCharacterBase* Target, const FBossMoveRow* Move) const;
	float GetAttackScale() const;
	float GetDefenseMitigation(AUnshackledCharacterBase* Target) const;
	int32 CountUsesThisRound(FName MoveId) const;
	bool IsModifierOnlyMove(const FBossMoveRow* Move) const;
	bool IsAtMaxStacks(AUnshackledCharacterBase* Target, FName MoveId, const FBossMoveRow* Move) const;
	int32 PickRandomMemberBelowMaxStacks(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const;
	ECharacterStat ToCharacterStat(EBossModStat ModStat) const;
	EModifierKind GetModKind(const FBossMoveRow* Move) const;
	TArray<FName> GatherMoveIds(ATurnBattleManager* Manager) const;
	FName ChooseMove(ATurnBattleManager* Manager, TArray<FName> MoveIds);
	void ShowMoveMessage(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex);
	virtual float TakeNextHitMultiplier();
	int32 CountLivingMembers(const ATurnBattleManager* Manager) const;
	void CheckMeditation(ATurnBattleManager* Manager, int32 MemberIndex);
	bool IsMeditationPending(const ATurnBattleManager* Manager) const;
	float ScoreMeditation(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const;
	void CastMeditation(ATurnBattleManager* Manager, FName MoveId);
	void TriggerMeditation(ATurnBattleManager* Manager, int32 MemberIndex);
	void CheckCycle(ATurnBattleManager* Manager);
	int32 GetPendingCycleCharges(const ATurnBattleManager* Manager) const;
	float ScoreCycle(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const;
	void CastCycle(ATurnBattleManager* Manager, FName MoveId);
	void TriggerCycle(ATurnBattleManager* Manager);
	bool IsGrailActive(const ATurnBattleManager* Manager) const;
	float ScoreGrail(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	void CastGrail(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move);
	void SpillGrail(ATurnBattleManager* Manager);
	bool IsMaliceArmed(const ATurnBattleManager* Manager) const;
	float ScoreMalice(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const;
	void CastMalice(ATurnBattleManager* Manager, FName MoveId);
	void StartMaliceWatch(ATurnBattleManager* Manager);
	bool FinishMaliceWatch(ATurnBattleManager* Manager);
	void TriggerMalice(ATurnBattleManager* Manager, int32 MemberIndex);
	TArray<FName> MovesUsedThisRound;
	bool bLastChoiceWasBlunder = false;
	FName LastMoveId;
	FName MeditationMoveId;
	int32 MeditationRound = -1;
	int32 MeditationActionsSeen = 0;
	FName CycleMoveId;
	int32 CycleRound = -1;
	int32 CycleCharges = 0;
	FName GrailMoveId;
	int32 GrailFirstRound = -1;
	int32 GrailLastRound = -1;
	FName MaliceMoveId;
	int32 MaliceRound = -1;
	bool bMaliceWatching = false;
	TArray<int32> SavedBuffCounts;
	TArray<float> SavedDivertedHealing;
	TArray<float> SavedShieldMultipliers;
	TArray<float> SavedShields;
	TArray<float> SavedStatsDivert;
	TArray<float> SavedHealthDivert;
};
