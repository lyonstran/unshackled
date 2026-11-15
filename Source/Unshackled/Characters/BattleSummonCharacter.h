#pragma once
#include "CoreMinimal.h"
#include "UnshackledCharacterBase.h"
#include "BattleSummonCharacter.generated.h"
class ATurnBattleManager;
UCLASS()
class UNSHACKLED_API ABattleSummonCharacter : public AUnshackledCharacterBase
{
	GENERATED_BODY()
public:
	ABattleSummonCharacter();
	virtual void TakeBattleTurn(ATurnBattleManager* Manager);
	void SetBattleMaxHealth(float NewMaxHealth);
	UPROPERTY(EditAnywhere, Category = "Summon")
	FString DisplayName = TEXT("Summon");
	UPROPERTY(EditAnywhere, Category = "Summon")
	float StrikePower = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Summon")
	float BlockMultiplier = 0.5f;
protected:
	virtual void Die() override;
};