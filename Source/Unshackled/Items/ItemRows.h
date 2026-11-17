#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemRows.generated.h"
USTRUCT(BlueprintType)
struct FActiveItemRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Floor = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWholeParty = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOverworldOnly = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealFlat = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealPercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ManaFlat = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReviveFraction = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamagePercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefensePercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedPercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShieldPercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BuffSeconds = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BuffRounds = 2;
};
USTRUCT(BlueprintType)
struct FArtifactRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Floor = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageFlat = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamagePercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefenseFlat = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefensePercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedFlat = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedPercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownReduction = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ManaRegenBonus = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DropChanceBonus = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealShieldBonus = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnemyFailChance = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Reflect = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnemyBuffReduction = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PulseSeconds = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PulseRounds = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PulseHealPercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PulseHealFlat = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PulseShieldPercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BattleShieldPercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShieldCapPercent = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MarkBonus = 0.0f;
};
