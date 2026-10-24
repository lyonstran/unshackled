#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "CasterWardenCharacter.generated.h"
enum class ECasterWardenState : uint8
{
	Idle,
	Winding
};
UCLASS()
class UNSHACKLED_API ACasterWardenCharacter : public AEnemyCharacter
{
	GENERATED_BODY()
public:
	ACasterWardenCharacter();
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, Category = "Caster Warden")
	float CometWindupTime = 2.0f;
	UPROPERTY(EditAnywhere, Category = "Caster Warden")
	float CometCooldown = 4.0f;
	UPROPERTY(EditAnywhere, Category = "Caster Warden")
	float CometCurrentHealthFraction = 0.1f;
	UPROPERTY(EditAnywhere, Category = "Caster Warden")
	float CometDefenseLoss = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Caster Warden")
	float CometDefenseLossDuration = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Caster Warden Circle")
	float CircleRadius = 300.0f;
	UPROPERTY(EditAnywhere, Category = "Caster Warden Circle")
	float CircleDefenseLoss = 0.1f;
	UPROPERTY(EditAnywhere, Category = "Caster Warden Circle")
	float CircleInterval = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Caster Warden Circle")
	float CircleDuration = 0.5f;
protected:
	void TickIdle(float DeltaTime);
	void TickWinding(float DeltaTime);
	void StartWindup(AUnshackledCharacterBase* Target);
	void FireComets();
	void TickMagicCircle(float DeltaTime);
	void ApplyCircleDebuff(AUnshackledCharacterBase* Ally);
	ECasterWardenState State = ECasterWardenState::Idle;
	float WindupTimeLeft = 0.0f;
	float AttackCooldownLeft = 0.0f;
	float CircleTimer = 0.0f;
	UPROPERTY()
	AUnshackledCharacterBase* LockedTarget;
};
