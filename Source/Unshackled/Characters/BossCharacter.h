#pragma once
#include "CoreMinimal.h"
#include "UnshackledCharacterBase.h"
#include "BossCharacter.generated.h"
UCLASS()
class UNSHACKLED_API ABossCharacter : public AUnshackledCharacterBase
{
	GENERATED_BODY()
public:
	ABossCharacter();
	UPROPERTY(EditAnywhere, Category = "Boss")
	FName BossId;
	UPROPERTY(EditAnywhere, Category = "Boss")
	FString DisplayName = TEXT("Boss");
};
