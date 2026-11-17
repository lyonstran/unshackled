#pragma once
#include "CoreMinimal.h"
#include "UnshackledCharacterBase.h"
#include "Game/ItemChestActor.h"
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
	UPROPERTY(EditAnywhere, Category = "Loot")
	bool bIsMiniboss = false;
	UPROPERTY(EditAnywhere, Category = "Loot")
	TSubclassOf<AItemChestActor> ChestClass;
protected:
	virtual void Die() override;
	virtual float GetFloorStatScale() const override;
	AUnshackledCharacterBase* FindClosestTarget();
	void DropLoot();
	void SpawnMinibossChest();
	bool HasMinibossChest() const;
};