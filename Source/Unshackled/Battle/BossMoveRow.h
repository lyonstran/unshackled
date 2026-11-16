#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BossMoveRow.generated.h"
UENUM(BlueprintType)
enum class EBossMoveTargetKind : uint8
{
	OneMember,
	WholeParty,
	Self
};
UENUM(BlueprintType)
enum class EBossPercentOf : uint8
{
	None,
	MaxHealth,
	CurrentHealth,
	MissingHealth
};
UENUM(BlueprintType)
enum class EBossModStat : uint8
{
	None,
	Damage,
	Defense,
	IncomingDamage,
	Poison
};
USTRUCT(BlueprintType)
struct FBossMoveRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MoveSetId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBossMoveTargetKind TargetKind = EBossMoveTargetKind::OneMember;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Power = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBossPercentOf PercentOf = EBossPercentOf::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Percent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBossModStat ModStat = EBossModStat::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ModAmount = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Turns = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStacks = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UsesPerRound = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScoreBase = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScoreBonus1 = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScoreBonus2 = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amount1 = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amount2 = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Amount3 = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OwnMissingPercent = 0.0f;
};