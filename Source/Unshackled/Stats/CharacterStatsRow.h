#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CharacterStatsRow.generated.h"
USTRUCT(BlueprintType)
struct FCharacterStatsRow : public FTableRowBase
{
        GENERATED_BODY()
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxHealth = 100.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Damage = 25.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Defense = 20.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float Speed = 600.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxMana = 100.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float ManaRegenPerSecond = 3.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float AttackCooldown = 0.5f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float DashCooldown = 5.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float SpecialManaCost = 20.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float SpecialCooldown = 6.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float SpecialWindup = 0.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float UltimateManaCost = 60.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float UltimateCooldown = 30.0f;
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float UltimateWindup = 0.0f;
};
