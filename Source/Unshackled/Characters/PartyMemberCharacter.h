#pragma once
#include "CoreMinimal.h"
#include "UnshackledCharacterBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/Texture2D.h"
#include "PartyMemberCharacter.generated.h"
UCLASS()
class UNSHACKLED_API APartyMemberCharacter : public AUnshackledCharacterBase
{
	GENERATED_BODY()
public:
	APartyMemberCharacter();
	virtual void Tick(float DeltaTime) override;
	void SetFollowing(bool bShouldFollow);
	bool IsFollowing() const;
	virtual void OnSwappedIn(APartyMemberCharacter* PreviousLeader);
	virtual void OnSwappedOut(APartyMemberCharacter* NewLeader);
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* CameraArm;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* Camera;
	UPROPERTY(EditAnywhere, Category = "Game Over")
	float GameOverDelay = 2.0f;
	UPROPERTY(EditAnywhere, Category = "Party")
	float FollowDistance = 150.0f;
	UPROPERTY(EditAnywhere, Category = "Party")
	FString DisplayName = TEXT("Ally");
	UPROPERTY(EditAnywhere, Category = "Party")
	UTexture2D* Portrait = nullptr;
protected:
	virtual void BeginPlay() override;
	virtual void Die() override;
	void FollowLeader();
	bool bIsFollowing = false;
	FTimerHandle GameOverTimer;
	UFUNCTION()
	void RestartLevelAfterDeath();
};