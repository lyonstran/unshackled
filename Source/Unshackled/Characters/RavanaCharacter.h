#pragma once
#include "CoreMinimal.h"
#include "BossCharacter.h"
#include "RavanaCharacter.generated.h"
class APartyMemberCharacter;
class AChimeraCharacter;
UCLASS()
class UNSHACKLED_API ARavanaCharacter : public ABossCharacter
{
	GENERATED_BODY()
public:
	ARavanaCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void OnBattleRoundStart(ATurnBattleManager* Manager) override;
	virtual void BeforeMemberAction(ATurnBattleManager* Manager, int32 MemberIndex, FName EntryName) override;
	virtual void OnBossPhaseStart(ATurnBattleManager* Manager) override;
	virtual void OnRetributionCompleted(ATurnBattleManager* Manager, int32 MemberIndex) override;
	virtual int32 GetBossTurnCount(ATurnBattleManager* Manager, int32 DefaultTurns) override;
	virtual FString GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const override;
	virtual FString GetStatusNote(const ATurnBattleManager* Manager) const override;
	UPROPERTY(EditAnywhere, Category = "Ravana")
	TSubclassOf<AChimeraCharacter> ChimeraClass;
	UPROPERTY(EditAnywhere, Category = "Ravana")
	float ChimeraHealthFraction = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Ravana")
	int32 ChimeraReviveRounds = 3;
	UPROPERTY(EditAnywhere, Category = "Ravana")
	int32 BaseTurnsPerRound = 2;
	UPROPERTY(EditAnywhere, Category = "Ravana")
	float ExtraTurnHealthFraction = 0.5f;
protected:
	virtual float FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker) override;
	virtual float ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual int32 PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual void PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex) override;
	void EnsureMemberArrays(const ATurnBattleManager* Manager);
	int32 GetSaturn(int32 MemberIndex) const;
	int32 GetTotalSaturn() const;
	void AddSaturn(ATurnBattleManager* Manager, int32 MemberIndex, int32 Amount);
	void UpdateSaturnEffects();
	void UpdateMemberSaturn(int32 MemberIndex, const FBossMoveRow* Row);
	void CheckSaturnExecute(ATurnBattleManager* Manager, int32 MemberIndex);
	float RavanaHit(ATurnBattleManager* Manager, int32 MemberIndex, const FBossMoveRow* Move, float BonusDamage);
	bool IsChimeraAlive() const;
	void SpawnChimeraIfNeeded(ATurnBattleManager* Manager);
	void TrackChimeraDeath();
	void ReviveChimeraIfReady(ATurnBattleManager* Manager);
	void TickWindFunnel(ATurnBattleManager* Manager);
	void TickGale(ATurnBattleManager* Manager);
	void ResolveLonginus(ATurnBattleManager* Manager, int32 ActingIndex);
	void ApplyAbsolution(ATurnBattleManager* Manager, FName MemberId);
	void MirrorBombs(ATurnBattleManager* Manager);
	void MirrorCleanse(ATurnBattleManager* Manager);
	bool HasOwnModifier(FName SourceId, ECharacterStat Stat, EModifierKind Kind) const;
	void AddTurnModifier(AUnshackledCharacterBase* Target, FName SourceId, ECharacterStat Stat, EModifierKind Kind, float Amount, int32 Turns);
	float GetOwnMissingPercent() const;
	float ScoreKneel(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreRavage(const FBossMoveRow* Move) const;
	float ScoreLonginus(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreGale(const FBossMoveRow* Move) const;
	float ScoreVeil(const FBossMoveRow* Move) const;
	float ScoreOmnichannel(const FBossMoveRow* Move) const;
	float ScoreWindWall(const FBossMoveRow* Move) const;
	float ScoreWindsOfWar(const FBossMoveRow* Move) const;
	float ScoreDominion(const FBossMoveRow* Move) const;
	int32 FindKneelTarget(ATurnBattleManager* Manager) const;
	int32 FindLowestHealthMember(ATurnBattleManager* Manager) const;
	int32 FindSubmissionTarget(ATurnBattleManager* Manager) const;
	void CastKneel(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex);
	void CastRavage(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastLonginus(ATurnBattleManager* Manager, int32 TargetIndex);
	void CastGale(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastVeil(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move);
	void CastOmnichannel(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move);
	void CastWindWall(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastWindsOfWar(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move);
	void CastSubmission(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex);
	void CastDominion(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	UPROPERTY()
	ATurnBattleManager* BattleManager = nullptr;
	UPROPERTY()
	AChimeraCharacter* Chimera = nullptr;
	bool bChimeraSpawned = false;
	int32 ChimeraDeathRound = -1;
	TArray<int32> SaturnStacks;
	TArray<int32> AppliedSaturn;
	TArray<FName> PendingAbsolutions;
	int32 AbsolutionTurns = 0;
	TArray<float> GaleTickDamage;
	int32 GaleRoundsLeft = 0;
	int32 LonginusTarget = -1;
	int32 LonginusRound = -1;
	bool bWallUp = false;
	float WallConvert = 0.0f;
	float WallBonus = 0.0f;
	int32 WallUses = 0;
	bool bDominionPending = false;
	float DominionHeal = 0.0f;
	int32 DominionUses = 0;
};
