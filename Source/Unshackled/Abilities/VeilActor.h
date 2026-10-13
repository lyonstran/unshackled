#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperFlipbookComponent.h"
#include "Characters/UnshackledCharacterBase.h"
#include "VeilActor.generated.h"
UCLASS()
class UNSHACKLED_API AVeilActor : public AActor
{
	GENERATED_BODY()
public:
	AVeilActor();
	virtual void Tick(float DeltaTime) override;
	void Setup(AUnshackledCharacterBase* InOwner);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Veil")
	UPaperFlipbookComponent* Visual;
	UPROPERTY(EditAnywhere, Category = "Veil")
	float Lifetime = 10.0f;
	UPROPERTY(EditAnywhere, Category = "Veil")
	float Radius = 250.0f;
	UPROPERTY(EditAnywhere, Category = "Veil")
	float DamageInterval = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Veil")
	float DamageScale = 0.2f;
	UPROPERTY(EditAnywhere, Category = "Veil")
	float SlowFraction = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Veil")
	float DefenseReductionFraction = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Veil")
	float AuraInterval = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Veil")
	float AuraDuration = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Veil")
	bool bDrawDebugShape = true;
protected:
	TArray<AUnshackledCharacterBase*> FindEnemiesInside();
	void ApplyDebuffsToEnemies();
	void ApplyDebuffsToTarget(AUnshackledCharacterBase* Target);
	void DamageEnemies();
	void FollowOwner();
	void DrawDebugShapeIfNoSprite();
	UPROPERTY()
	AUnshackledCharacterBase* OwnerCharacter;
	float AgeSeconds = 0.0f;
	float TimeSinceDamage = 0.0f;
	float TimeSinceAura = 0.0f;
};
