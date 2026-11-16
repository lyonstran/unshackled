#pragma once
#include "CoreMinimal.h"
#include "BattleSummonCharacter.h"
#include "ChimeraCharacter.generated.h"
UCLASS()
class UNSHACKLED_API AChimeraCharacter : public ABattleSummonCharacter
{
	GENERATED_BODY()
public:
	AChimeraCharacter();
	void ReviveInBattle();
protected:
	virtual float ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move) override;
	virtual void PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex) override;
};
