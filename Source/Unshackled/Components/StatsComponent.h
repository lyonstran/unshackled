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
        UPROPERTY(EditAnywhere, Category = "Stats")
        float BaseDamage = 25.0f;
        UPROPERTY(EditAnywhere, Category = "Stats")
        float BaseDefense = 20.0f;
        UPROPERTY(EditAnywhere, Category = "Stats")
        float BaseSpeed = 600.0f;
        UPROPERTY(EditAnywhere, Category = "Stats")
        float BaseMaxMana = 100.0f;
        UPROPERTY(EditAnywhere, Category = "Stats")
        bool bHasMana = false;
        UPROPERTY(EditAnywhere, Category = "Stats")
        float DefenseScale = 50.0f;
        UPROPERTY(EditAnywhere, Category = "Stats")
        float MaxSlowFraction = 0.8f;
        float BaseMaxHealth = 100.0f;
        float CurrentMana = 0.0f;
        void AddModifier(FStatModifier NewModifier);
        void RemoveModifiersFromSource(FName SourceId);
        void ConsumeHit();
        float GetFinal(ECharacterStat Stat) const;
        bool CanMove() const;
        bool CanAct() const;
        bool SpendMana(float Amount);
        void RestoreMana(float Amount);
        virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction* ThisTickFunction) override;
protected:
        virtual void BeginPlay() override;
private:
        float GetBaseValue(ECharacterStat Stat) const;
        float GetFlatTotal(ECharacterStat Stat) const;
        float GetPercentTotal(ECharacterStat Stat) const;
        float GetStrongestSlow() const;
        float ApplyFloors(ECharacterStat Stat, float Value) const;
        int32 FindModifierIndex(FName SourceId, ECharacterStat Stat, EModifierKind Kind) const;
        bool HasFlag(EStatusFlag Flag) const;
        void CountDownModifiers(float DeltaTime);
        TArray<FStatModifier> Modifiers;
};