#pragma once
#include "CoreMinimal.h"
#include "BattleEnemyCharacter.h"
#include "BossCharacter.generated.h"
UCLASS()
class UNSHACKLED_API ABossCharacter : public ABattleEnemyCharacter
{
	GENERATED_BODY()
public:
	ABossCharacter();
	UPROPERTY(EditAnywhere, Category = "Boss")
	FName BossId;
	UPROPERTY(EditAnywhere, Category = "Boss")
	FName ExcludedMemberId;
};