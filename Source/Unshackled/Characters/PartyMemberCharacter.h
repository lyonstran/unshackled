#pragma once
#include "CoreMinimal.h"
#include "UnshackledCharacterBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "PartyMemberCharacter.generated.h"

UCLASS()
class UNSHACKLED_API APartyMemberCharacter : public AUnshackledCharacterBase
{
	GENERATED_BODY()
public:
	APartyMemberCharacter();
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* CameraArm;
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* Camera;
	UPROPERTY(EditAnywhere, Category = "Game Over")
	float GameOverDelay = 2.0f;
protected:
	virtual void Die() override;

	FTimerHandle GameOverTimer;
	UFUNCTION()
	void RestartLevelAfterDeath();
};