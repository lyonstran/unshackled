#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Characters/EnemyCharacter.h"
#include "WaveRow.generated.h"
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