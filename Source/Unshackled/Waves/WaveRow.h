#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Characters/EnemyCharacter.h"
#include "WaveRow.generated.h"

// One row of DT_Waves: "on this floor, in this wave, spawn this many of this enemy".
USTRUCT(BlueprintType)
struct FWaveRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Floor = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wave = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemyCharacter> EnemyClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bScales = true;
};

USTRUCT(BlueprintType)
struct FFloorRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExpectedPartySize = 2;
};