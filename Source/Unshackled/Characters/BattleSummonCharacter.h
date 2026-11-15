#pragma once
#include "CoreMinimal.h"
#include "BattleEnemyCharacter.h"
#include "BattleSummonCharacter.generated.h"
class ATurnBattleManager;
UCLASS()
class UNSHACKLED_API ABattleSummonCharacter : public ABattleEnemyCharacter
{
	GENERATED_BODY()
public:
	ABattleSummonCharacter();
	virtual void TakeBattleTurn(ATurnBattleManager* Manager);
	void SetBattleMaxHealth(float NewMaxHealth);
	void DieInBattle();
	UPROPERTY(EditAnywhere, Category = "Summon")
	float StrikePower = 1.0f;
protected:
	virtual void Die() override;
};