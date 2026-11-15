#pragma once
#include "CoreMinimal.h"
#include "UnshackledCharacterBase.h"
#include "Battle/BossMoveRow.h"
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
	int32 PickRandomTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const;
	float HitMemberWithMove(ATurnBattleManager* Manager, int32 MemberIndex, const FBossMoveRow* Move, float Multiplier);
	void ApplyMoveModifier(AUnshackledCharacterBase* Target, FName MoveId, const FBossMoveRow* Move);
	float GetPercentBase(AUnshackledCharacterBase* Target, const FBossMoveRow* Move) const;
	float GetAttackScale() const;
	float GetDefenseMitigation(AUnshackledCharacterBase* Target) const;
	int32 CountUsesThisRound(FName MoveId) const;
	bool IsModifierOnlyMove(const FBossMoveRow* Move) const;
	bool IsAtMaxStacks(AUnshackledCharacterBase* Target, FName MoveId, const FBossMoveRow* Move) const;
	int32 PickRandomMemberBelowMaxStacks(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const;
	ECharacterStat ToCharacterStat(EBossModStat ModStat) const;
	TArray<FName> GatherMoveIds(ATurnBattleManager* Manager) const;
	FName ChooseMove(ATurnBattleManager* Manager, TArray<FName> MoveIds);
	void ShowMoveMessage(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex);
	TArray<FName> MovesUsedThisRound;
	bool bLastChoiceWasBlunder = false;
};