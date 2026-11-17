#include "Components/ArtifactPulseComponent.h"
#include "Game/UnshackledGameInstance.h"
#include "Characters/PartyMemberCharacter.h"
#include "Characters/EnemyCharacter.h"
#include "UI/UnshackledHUD.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
UArtifactPulseComponent::UArtifactPulseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}
UUnshackledGameInstance* UArtifactPulseComponent::GetUnshackledGameInstance() const
{
	UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(UGameplayStatics::GetGameInstance(this));
	return GameInstanceRef;
}
void UArtifactPulseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DrawMarkedEnemies();
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		return;
	}
	TArray<FName> EquippedIds = GameInstanceRef->GetEquippedArtifactIds();
	for (int32 i = 0; i < EquippedIds.Num(); i++)
	{
		const FArtifactRow* Row = GameInstanceRef->GetArtifactRow(EquippedIds[i]);
		if (Row != nullptr)
		{
			if (Row->PulseSeconds > 0.0f)
			{
				bool bPulseDue = AdvanceTimer(EquippedIds[i], DeltaTime, Row->PulseSeconds);
				if (bPulseDue)
				{
					RunPulse(Row);
				}
			}
		}
	}
}
bool UArtifactPulseComponent::AdvanceTimer(FName ArtifactId, float DeltaTime, float Interval)
{
	int32 Index = TimerIds.Find(ArtifactId);
	if (Index == INDEX_NONE)
	{
		TimerIds.Add(ArtifactId);
		Index = TimerValues.Add(0.0f);
	}
	TimerValues[Index] = TimerValues[Index] + DeltaTime;
	if (TimerValues[Index] < Interval)
	{
		return false;
	}
	TimerValues[Index] = TimerValues[Index] - Interval;
	return true;
}
void UArtifactPulseComponent::RunPulse(const FArtifactRow* Row)
{
	PulsePartyMembers(Row);
	if (Row->MarkBonus > 0.0f)
	{
		MarkNearestEnemy(Row->MarkBonus);
	}
}
void UArtifactPulseComponent::PulsePartyMembers(const FArtifactRow* Row)
{
	for (TActorIterator<APartyMemberCharacter> It(GetWorld()); It; ++It)
	{
		APartyMemberCharacter* Member = *It;
		if (!Member->Health->IsDead())
		{
			float HealAmount = (Row->PulseHealPercent * Member->Health->MaxHealth) + Row->PulseHealFlat;
			if (HealAmount > 0.0f)
			{
				Member->Health->Heal(HealAmount);
			}
			if (Row->PulseShieldPercent > 0.0f)
			{
				float ShieldAmount = Member->Health->CurrentHealth * Row->PulseShieldPercent;
				float ShieldCap = Member->Health->MaxHealth * Row->ShieldCapPercent;
				Member->AddCappedShield(ShieldAmount, ShieldCap);
			}
		}
	}
}
void UArtifactPulseComponent::MarkNearestEnemy(float Bonus)
{
	APawn* LeaderPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LeaderPawn == nullptr)
	{
		return;
	}
	AEnemyCharacter* Nearest = nullptr;
	float NearestDistance = 0.0f;
	for (TActorIterator<AEnemyCharacter> It(GetWorld()); It; ++It)
	{
		AEnemyCharacter* Enemy = *It;
		if (!Enemy->Health->IsDead())
		{
			int32 MarkStacks = Enemy->Stats->GetStacks(FName("CompassMark"), ECharacterStat::IncomingDamage, EModifierKind::Percent);
			if (MarkStacks == 0)
			{
				float Distance = FVector::Dist2D(Enemy->GetActorLocation(), LeaderPawn->GetActorLocation());
				if (Nearest == nullptr)
				{
					Nearest = Enemy;
					NearestDistance = Distance;
				}
				else
				{
					if (Distance < NearestDistance)
					{
						Nearest = Enemy;
						NearestDistance = Distance;
					}
				}
			}
		}
	}
	if (Nearest == nullptr)
	{
		return;
	}
	FStatModifier Mark;
	Mark.SourceId = FName("CompassMark");
	Mark.Stat = ECharacterStat::IncomingDamage;
	Mark.Kind = EModifierKind::Percent;
	Mark.Amount = Bonus;
	Mark.Duration = 0.0f;
	Mark.TurnsLeft = 0;
	Mark.Rule = EStackRule::RefreshSameSource;
	Nearest->Stats->AddModifier(Mark);
	AUnshackledHUD* Hud = AUnshackledHUD::FindPlayerHUD(this);
	if (Hud != nullptr)
	{
		Hud->AddMessage(TEXT("The compass marks an enemy"), FLinearColor(0.8f, 0.6f, 1.0f, 1.0f), 2.0f);
	}
}
void UArtifactPulseComponent::DrawMarkedEnemies()
{
	for (TActorIterator<AEnemyCharacter> It(GetWorld()); It; ++It)
	{
		AEnemyCharacter* Enemy = *It;
		if (!Enemy->Health->IsDead())
		{
			int32 MarkStacks = Enemy->Stats->GetStacks(FName("CompassMark"), ECharacterStat::IncomingDamage, EModifierKind::Percent);
			if (MarkStacks > 0)
			{
				FVector MarkerLocation = Enemy->GetActorLocation() + FVector(60.0f, 0.0f, 0.0f);
				DrawDebugSphere(GetWorld(), MarkerLocation, MarkMarkerRadius, 8, FColor(200, 120, 255), false, -1.0f);
			}
		}
	}
}
