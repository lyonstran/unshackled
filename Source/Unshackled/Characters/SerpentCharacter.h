#pragma once
#include "CoreMinimal.h"
#include "BossCharacter.h"
#include "SerpentCharacter.generated.h"
class ABattleSummonCharacter;
class APartyMemberCharacter;
UCLASS()
class UNSHACKLED_API ASerpentCharacter : public ABossCharacter
{
	GENERATED_BODY()
public:
	ASerpentCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void OnBattleRoundStart(ATurnBattleManager* Manager) override;
	virtual FString GetMemberNote(const ATurnBattleManager* Manager, int32 MemberIndex) const override;
	UPROPERTY(EditAnywhere, Category = "Serpent")
	TSubclassOf<ABattleSummonCharacter> SnakeClass;
	UPROPERTY(EditAnywhere, Category = "Serpent")
	float VenomMissingFraction = 0.01f;
	UPROPERTY(EditAnywhere, Category = "Serpent")
	float ShedVenomMissingFraction = 0.015f;
	UPROPERTY(EditAnywhere, Category = "Serpent")
	float WeaknessPerTenPercent = 0.005f;
	UPROPERTY(EditAnywhere, Category = "Serpent")
	float ShedHealthFraction = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Serpent")
	float ShedDefensePerTenPercent = 0.02f;
	UPROPERTY(EditAnywhere, Category = "Serpent")
	float DrainMinFactor = 0.5f;
protected:
	virtual float ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual int32 PickMoveTarget(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual void PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex) override;
	float ScorePounce(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreDrain(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreDecay(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreConjure(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreBeckon(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	float ScoreMiasma(ATurnBattleManager* Manager, const FBossMoveRow* Move) const;
	int32 FindMostWoundedMember(ATurnBattleManager* Manager) const;
	int32 FindHealthiestMember(ATurnBattleManager* Manager) const;
	int32 FindStrongestMemberBelowMax(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) const;
	int32 GetDecayStacks(APartyMemberCharacter* Member) const;
	int32 GetLowestDecayStacks(ATurnBattleManager* Manager) const;
	float GetAverageDecayStacks(ATurnBattleManager* Manager) const;
	float GetDrainFactor(const FBossMoveRow* Move) const;
	float GetMissingFraction(const AUnshackledCharacterBase* Target) const;
	void CastDrain(ATurnBattleManager* Manager, const FBossMoveRow* Move, int32 TargetIndex);
	void CastConjure(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastBeckon(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void CastMiasma(ATurnBattleManager* Manager, const FBossMoveRow* Move);
	void ApplyVenom(ATurnBattleManager* Manager);
	void ApplyWeakness(APartyMemberCharacter* Member);
	void TickMiasma(ATurnBattleManager* Manager);
	void UpdateShedSkin();
	UPROPERTY()
	ATurnBattleManager* BattleManager = nullptr;
	bool bShed = false;
	int32 ShedSteps = -1;
	int32 DrainUses = 0;
	int32 LastConjureRound = -1;
	int32 MiasmaRoundsLeft = 0;
	float MiasmaPower = 0.0f;
	TArray<float> MiasmaMultipliers;
};