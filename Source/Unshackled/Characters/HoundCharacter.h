#pragma once
#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "HoundCharacter.generated.h"
enum class EHoundState : uint8
{
	Sniffing,
	Hunting,
	Pouncing,
	LeapingBack
};
UCLASS()
class UNSHACKLED_API AHoundCharacter : public AEnemyCharacter
{
	GENERATED_BODY()
public:
	AHoundCharacter();
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, Category = "Hound")
	float SniffRadius = 700.0f;
	UPROPERTY(EditAnywhere, Category = "Hound")
	float SniffSlowFraction = 0.7f;
	UPROPERTY(EditAnywhere, Category = "Hound")
	float WanderRadius = 300.0f;
	UPROPERTY(EditAnywhere, Category = "Hound")
	float WanderInterval = 3.0f;
	UPROPERTY(EditAnywhere, Category = "Hound")
	float HuntSpeedBonus = 0.6f;
	UPROPERTY(EditAnywhere, Category = "Hound")
	float BiteStopDistance = 40.0f;
	UPROPERTY(EditAnywhere, Category = "Hound Pounce")
	float PounceTriggerDistance = 220.0f;
	UPROPERTY(EditAnywhere, Category = "Hound Pounce")
	float PounceSpeed = 2200.0f;
	UPROPERTY(EditAnywhere, Category = "Hound Pounce")
	float PounceMaxTime = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Hound Pounce")
	float PounceHitDistance = 70.0f;
	UPROPERTY(EditAnywhere, Category = "Hound Pounce")
	float PounceDamageScale = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Hound Pounce")
	float PounceCooldown = 1.0f;
	UPROPERTY(EditAnywhere, Category = "Hound Pounce")
	float BiteSlowFraction = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Hound Pounce")
	float BiteSlowDuration = 2.0f;
	UPROPERTY(EditAnywhere, Category = "Hound Pounce")
	float LeapBackDistance = 300.0f;
	UPROPERTY(EditAnywhere, Category = "Hound Pounce")
	float LeapBackSpeed = 1600.0f;
protected:
	void SetupIfNeeded();
	void TickSniffing(float DeltaTime);
	void TickHunting(float DeltaTime);
	void TickPouncing(float DeltaTime);
	void TickLeapingBack(float DeltaTime);
	void StartHunting();
	void StartPounce(AUnshackledCharacterBase* Target);
	void StartLeapBack();
	void EndLeapBack();
	void BiteTarget(AUnshackledCharacterBase* Target);
	void PickWanderPoint();
	void MoveToward(FVector Direction);
	EHoundState State = EHoundState::Sniffing;
	bool bSetupDone = false;
	FVector HomeLocation = FVector::ZeroVector;
	FVector WanderPoint = FVector::ZeroVector;
	float WanderTimeLeft = 0.0f;
	float PounceTimeLeft = 0.0f;
	float PounceCooldownLeft = 0.0f;
	float LeapDistanceLeft = 0.0f;
	FVector LeapDirection = FVector(-1.0f, 0.0f, 0.0f);
	UPROPERTY()
	AUnshackledCharacterBase* PounceTarget;
};
