#include "UI/UnshackledHUD.h"
#include "Characters/UnshackledCharacterBase.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
void AUnshackledHUD::DrawHUD()
{
        Super::DrawHUD();
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Character = *It;
                if (Character != nullptr)
                {
                        DrawHealthBar(Character);
                        DrawManaBar(Character);
                }
        }
}
void AUnshackledHUD::DrawHealthBar(AUnshackledCharacterBase* Character)
{
        if (Character->Health->IsDead())
        {
                return;
        }
        float MaxHealth = Character->Health->MaxHealth;
        if (MaxHealth <= 0.0f)
        {
                return;
        }
        float HealthFraction = Character->Health->CurrentHealth / MaxHealth;
        FVector WorldPosition = Character->GetActorLocation() + FVector(BarWorldOffset, 0.0f, 0.0f);
        FVector ScreenPosition = Project(WorldPosition);
        float BarLeft = ScreenPosition.X - (BarWidth / 2.0f);
        float BarTop = ScreenPosition.Y;
        FLinearColor BackColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.8f);
        DrawRect(BackColor, BarLeft, BarTop, BarWidth, BarHeight);
        FLinearColor FrontColor = FLinearColor(0.1f, 0.9f, 0.2f, 1.0f);
        if (Character->bIsEnemy)
        {
                FrontColor = FLinearColor(0.9f, 0.1f, 0.1f, 1.0f);
        }
        DrawRect(FrontColor, BarLeft, BarTop, BarWidth * HealthFraction, BarHeight);
}
void AUnshackledHUD::DrawManaBar(AUnshackledCharacterBase* Character)
{
        if (!Character->Stats->bHasMana)
        {
                return;
        }
        if (Character->Health->IsDead())
        {
                return;
        }
        float MaxMana = Character->Stats->GetFinal(ECharacterStat::MaxMana);
        if (MaxMana <= 0.0f)
        {
                return;
        }
        float ManaFraction = Character->Stats->CurrentMana / MaxMana;
        FVector WorldPosition = Character->GetActorLocation() + FVector(BarWorldOffset, 0.0f, 0.0f);
        FVector ScreenPosition = Project(WorldPosition);
        float BarLeft = ScreenPosition.X - (BarWidth / 2.0f);
        float BarTop = ScreenPosition.Y + BarHeight + ManaBarGap;
        FLinearColor BackColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.8f);
        DrawRect(BackColor, BarLeft, BarTop, BarWidth, ManaBarHeight);
        FLinearColor FrontColor = FLinearColor(0.2f, 0.5f, 1.0f, 1.0f);
        DrawRect(FrontColor, BarLeft, BarTop, BarWidth * ManaFraction, ManaBarHeight);
}
