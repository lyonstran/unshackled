#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

UCLASS(ClassGroup = (Unshackled), meta = (BlueprintSpawnableComponent))
class UNSHACKLED_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UHealthComponent();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.0f;
	
	void TakeHit(float Amount);
	void Heal(float Amount);
	void Revive();
	bool IsDead() const;
	
protected:
	virtual void BeginPlay() override;
};