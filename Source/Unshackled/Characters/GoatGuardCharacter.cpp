#include "Characters/GoatGuardCharacter.h"
#include "Characters/BossCharacter.h"
#include "Battle/TurnBattleManager.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
AGoatGuardCharacter::AGoatGuardCharacter()
{
	DisplayName = TEXT("Guard");
}
float AGoatGuardCharacter::ScoreMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move)
{
	float BaseScore = Super::ScoreMove(Manager, MoveId, Move);
	if (BaseScore <= 0.0f)
	{
		return 0.0f;
	}
	if (MoveId != FName("GoatGuard_Sacrifice"))
	{
		return BaseScore;
	}
	ABossCharacter* Boss = Manager->GetBoss();
	if (Boss == nullptr)
	{
		return 0.0f;
	}
	if (Boss->Health->IsDead())
	{
		return 0.0f;
	}
	if (Boss->Health->MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	float BossHealthFraction = Boss->Health->CurrentHealth / Boss->Health->MaxHealth;
	if (BossHealthFraction >= Move->Amount2)
	{
		return 0.0f;
	}
	return BaseScore;
}
void AGoatGuardCharacter::PerformMove(ATurnBattleManager* Manager, FName MoveId, const FBossMoveRow* Move, int32 TargetIndex)
{
	if (MoveId == FName("GoatGuard_Sacrifice"))
	{
		CastSacrifice(Manager, Move);
		return;
	}
	Super::PerformMove(Manager, MoveId, Move, TargetIndex);
}
void AGoatGuardCharacter::CastSacrifice(ATurnBattleManager* Manager, const FBossMoveRow* Move)
{
	ABossCharacter* Boss = Manager->GetBoss();
	if (Boss == nullptr)
	{
		return;
	}
	float HealAmount = Boss->Health->CurrentHealth * Move->Amount1;
	Boss->Health->Heal(HealAmount);
	Manager->ShowMessage(FString::Printf(TEXT("%s gives its life to heal the Goat for %d"), *DisplayName, FMath::RoundToInt(HealAmount)), FColor::Orange);
	Health->CurrentHealth = 0.0f;
	Die();
}