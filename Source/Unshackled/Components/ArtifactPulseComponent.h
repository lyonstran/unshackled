#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemRows.h"
#include "ArtifactPulseComponent.generated.h"
class UUnshackledGameInstance;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UNSHACKLED_API UArtifactPulseComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UArtifactPulseComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY(EditAnywhere, Category = "Artifacts")
	float MarkMarkerRadius = 20.0f;
private:
	TArray<FName> TimerIds;
	TArray<float> TimerValues;
	UUnshackledGameInstance* GetUnshackledGameInstance() const;
	bool AdvanceTimer(FName ArtifactId, float DeltaTime, float Interval);
	void RunPulse(const FArtifactRow* Row);
	void PulsePartyMembers(const FArtifactRow* Row);
	void MarkNearestEnemy(float Bonus);
	void DrawMarkedEnemies();
};
