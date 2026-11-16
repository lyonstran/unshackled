#pragma once
#include "CoreMinimal.h"
#include "BossCharacter.h"
#include "DragonCharacter.generated.h"
class APartyMemberCharacter;
UCLASS()
class UNSHACKLED_API ADragonCharacter : public ABossCharacter
{
	GENERATED_BODY()
public:
	ADragonCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void OnBattleRoundStart(ATurnBattleManager* Manager) override;
	virtual void BeforeMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName) override;
	virtual void AfterMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName) override;
	virtual FString GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const override;
	virtual FString GetStatusNote(const ATurnBattleManager* Manager) const override;
	UPROPERTY(EditAnywhere, Category = "Dragon")
	float DefensePerTenPercent = 0.05f;
protected:
	virtual float FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker) override;
	virtual float ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual int32 PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual void PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex) override;
	float GetOwnMissingPercent() const;
	float ScoreScorch(const FBossMoveRow* Move) const;
	float ScoreRuination(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScorePlume(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreAlmighty(const FBossMoveRow* Move) const;
	float ScoreFury(const FBossMoveRow* Move) const;
	float ScoreFlameHorizon(const FBossMoveRow* Move) const;
	float ScoreGrail(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	int32 FindLowestHealthMember(ATurnBattleManager* Manager) const;
	int32 FindHighestDefenseWithoutBlaze(ATurnBattleManager* Manager) const;
	int32 FindRuinationTarget(ATurnBattleManager* Manager) const;
	bool HasBlaze(APartyMemberCharacter* Member) const;
	bool IsRuined(int32 MemberIndex) const;
	bool IsGrailActive(const ATurnBattleManager* Manager) const;
	TArray<FName> GetRuinableMoves(const ATurnBattleManager* Manager, APartyMemberCharacter* Member) const;
	void CastScorch(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastRuination(ATurnBattleManager* Manager, int32 TargetIndex);
	void CastAlmighty(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move);
	void CastFury(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastFlameHorizon(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastGrail(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void AddAlmightyModifier(FName MoveId, ECharacterStat Stat, const FBossMoveRow* Move);
	void TriggerRuination(ATurnBattleManager* Manager, int32 MemberIndex);
	void SpillGrail(ATurnBattleManager* Manager);
	void SpewFireballs(ATurnBattleManager* Manager);
	void LandFireballs(ATurnBattleManager* Manager);
	void TickFlameHorizon(ATurnBattleManager* Manager);
	void CheckHellfire();
	void CastHellfire(ATurnBattleManager* Manager);
	void UpdateScales();
	UPROPERTY()
	ATurnBattleManager* BattleManager = nullptr;
	int32 ScaleSteps = -1;
	bool bHellfireDone = false;
	TArray<int32> VolleyLandRounds;
	TArray<int32> VolleyCounts;
	TArray<FName> RuinedMoves;
	int32 FuryCharges = 0;
	int32 FlameRoundsLeft = 0;
	float FlamePower = 0.0f;
	float FlameOwnMissingPercent = 0.0f;
	FName GrailMoveId;
	int32 GrailFirstRound = -1;
	int32 GrailLastRound = -1;
};
