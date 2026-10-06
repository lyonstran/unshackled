#pragma once
#include "CoreMinimal.h"
#include "UnshackledCharacterBase.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class UNSHACKLED_API AEnemyCharacter: public AUnshackledCharacterBase
{
	GENERATED_BODY()
public:
	AEnemyCharacter();
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, Category = "AI")
	float AttackTriggerDistance = 90.0f;
protected:
	virtual void Die() override;
};