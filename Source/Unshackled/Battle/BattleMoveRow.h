#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Battle/TelegraphRow.h"
#include "BattleMoveRow.generated.h"
UENUM(BlueprintType)
enum class EBattleTargetKind : uint8
{
	Enemy,
	Ally,
	Caster,
	AllAllies,
	AllEnemies,
	Everyone
};
USTRUCT(BlueprintType)
struct FBattleMoveRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MemberId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBattleAction Category = EBattleAction::Special;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBattleTargetKind TargetKind = EBattleTargetKind::Enemy;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ManaCost = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Power = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amount1 = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amount2 = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amount3 = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amount4 = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Turns = 0;
};