#include "Characters/ChimeraCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Components/HealthComponent.h"
AChimeraCharacter::AChimeraCharacter()
{
	DisplayName = TEXT("Chimera");
	bPicksMovesRandomly = true;
}
void AChimeraCharacter::ReviveInBattle()
{
	if (!Health->IsDead())
	{
		return;
	}
	Health->Revive();
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}
float AChimeraCharacter::ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	float BaseScore = Super::ScoreMove(Manager, MoveId, Move);
	if (BaseScore <= 0.0f)
	{
		return 0.0f;
	}
	if (MoveId == FName("Chimera_Meditation"))
	{
		return ScoreMeditation(Manager, MoveId, Move);
	}
	if (MoveId == FName("Chimera_Cycle"))
	{
		return ScoreCycle(Manager, MoveId, Move);
	}
	if (MoveId == FName("Chimera_Grail"))
	{
		return ScoreGrail(Manager, Move);
	}
	if (MoveId == FName("Chimera_Malice"))
	{
		return ScoreMalice(Manager, MoveId, Move);
	}
	return BaseScore;
}
void AChimeraCharacter::PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex)
{
	if (MoveId == FName("Chimera_Meditation"))
	{
		CastMeditation(Manager, MoveId);
	}
	else if (MoveId == FName("Chimera_Cycle"))
	{
		CastCycle(Manager, MoveId);
	}
	else if (MoveId == FName("Chimera_Grail"))
	{
		CastGrail(Manager, MoveId, Move);
	}
	else if (MoveId == FName("Chimera_Malice"))
	{
		CastMalice(Manager, MoveId);
	}
	else
	{
		Super::PerformMove(Manager, MoveId, Move, TargetIndex);
	}
}
