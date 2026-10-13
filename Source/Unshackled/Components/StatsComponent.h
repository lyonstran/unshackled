#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Stats/StatTypes.h"
#include "StatsComponent.generated.h"

UCLASS(ClassGroup = (Unshackled), meta = (BlueprintSpawnableComponent))
class UNSHACKLED_API UStatsComponent : public UActorComponent
{
        GENERATED_BODY()

public:
        UStatsComponent();

        // Base values (before any modifiers). Editable per Blueprint.
        UPROPERTY(EditAnywhere, Category = "Stats")
        float BaseDamage = 25.0f;

        UPROPERTY(EditAnywhere, Category = "Stats")
        float BaseDefense = 20.0f;

        UPROPERTY(EditAnywhere, Category = "Stats")
        float BaseSpeed = 600.0f;

        UPROPERTY(EditAnywhere, Category = "Stats")
        float BaseMaxMana = 100.0f;

        // Only the player party uses mana. Leave false on enemies.
        UPROPERTY(EditAnywhere, Category = "Stats")
        bool bHasMana = false;

        // Defense formula: damage taken = raw * DefenseScale / (DefenseScale + Defense).
        UPROPERTY(EditAnywhere, Category = "Stats")
        float DefenseScale = 50.0f;

        // The strongest a slow can be (0.8 = at most 80% slower).
        UPROPERTY(EditAnywhere, Category = "Stats")
        float MaxSlowFraction = 0.8f;

        // Set by the character at BeginPlay from the HealthComponent's MaxHealth,
        // so the number typed in the Blueprint is kept.
        float BaseMaxHealth = 100.0f;

        float CurrentMana = 0.0f;

        // Adding and removing modifiers.
        void AddModifier(FStatModifier NewModifier);
        void RemoveModifiersFromSource(FName SourceId);
        void ConsumeHit();

        // The final value of a stat after all modifiers are applied.
        float GetFinal(ECharacterStat Stat) const;

        // Status checks (root / stun / freeze).
        bool CanMove() const;
        bool CanAct() const;

        // Mana.
        bool SpendMana(float Amount);
        void RestoreMana(float Amount);

        virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction* ThisTickFunction) override;

protected:
        virtual void BeginPlay() override;

private:
        // Helpers that GetFinal uses, one job each.
        float GetBaseValue(ECharacterStat Stat) const;
        float GetFlatTotal(ECharacterStat Stat) const;
        float GetPercentTotal(ECharacterStat Stat) const;
        float GetStrongestSlow() const;
        float ApplyFloors(ECharacterStat Stat, float Value) const;

        // Helpers for the modifier list.
        int32 FindModifierIndex(FName SourceId, ECharacterStat Stat, EModifierKind Kind) const;
        bool HasFlag(EStatusFlag Flag) const;
        void CountDownModifiers(float DeltaTime);

        TArray<FStatModifier> Modifiers;
};