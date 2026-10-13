#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperFlipbookComponent.h"
#include "Characters/UnshackledCharacterBase.h"
#include "TornadoActor.generated.h"
UCLASS()
class UNSHACKLED_API ATornadoActor : public AActor
{
	GENERATED_BODY()
public:
	ATornadoActor();
	virtual void Tick(float DeltaTime) override;
	void Setup(AUnshackledCharacterBase* InOwner);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tornado")
	UPaperFlipbookComponent* Visual;
	UPROPERTY(EditAnywhere, Category = "Tornado")
	float Lifetime = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Tornado")
	float MoveSpeed = 250.0f;
	UPROPERTY(EditAnywhere, Category = "Tornado")
	float StopDistance = 20.0f;
	UPROPERTY(EditAnywhere, Category = "Tornado")
	float Radius = 120.0f;
	UPROPERTY(EditAnywhere, Category = "Tornado")
	float DamageInterval = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Tornado")
	float DamageScale = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Tornado")
	bool bDrawDebugShape = true;
protected:
	AUnshackledCharacterBase* FindNearestEnemy();
	void ChaseNearestEnemy(float DeltaTime);
	void DamageEnemiesInside();
	void DrawDebugShapeIfNoSprite();
	UPROPERTY()
	AUnshackledCharacterBase* OwnerCharacter;
	float AgeSeconds = 0.0f;
	float TimeSinceDamage = 0.0f;
};
