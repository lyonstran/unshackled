#pragma once
#include "CoreMinimal.h"
#include "BattleSummonCharacter.h"
#include "GoatGuardCharacter.generated.h"
UCLASS()
class UNSHACKLED_API AGoatGuardCharacter : public ABattleSummonCharacter
{
	GENERATED_BODY()
public:
	AGoatGuardCharacter();
protected:
	virtual float ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual void PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex) override;
	void CastSacrifice(ATurnBattleManager* Manager, const FBossMoveRow* Move);
};