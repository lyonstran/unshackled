#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TelegraphRow.generated.h"
UENUM(BlueprintType)
enum class EBattleAction : uint8
{
	None,
	Attack,
	Special,
	Ultimate,
	Item,
	Block
};
UENUM(BlueprintType)
enum class EBossTargetMode : uint8
{
	OneRandom,
	Leader,
	WholeParty
};
USTRUCT(BlueprintType)
struct FTelegraphRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BossId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CountdownActions = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageScale = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBossTargetMode TargetMode = EBossTargetMode::OneRandom;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CounterMemberId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBattleAction CounterAction = EBattleAction::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlockMultiplier = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CounterMultiplier = 0.0f;
};
