#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UnshackledGameInstance.generated.h"
UCLASS()
class UNSHACKLED_API UUnshackledGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "Progress")
	int32 CurrentFloor = 1;
	UPROPERTY(EditAnywhere, Category = "Progress")
	int32 MaxFloor = 5;
	bool IsLastFloor() const;
	bool AdvanceFloor();
	void ResetToFirstFloor();
};
