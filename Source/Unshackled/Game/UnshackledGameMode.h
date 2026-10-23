#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/DataTable.h"
#include "UnshackledGameMode.generated.h"
UCLASS()
class UNSHACKLED_API AUnshackledGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AUnshackledGameMode();
	UPROPERTY(EditDefaultsOnly, Category = "Tuning")
	UDataTable* CharacterStatsTable = nullptr;
};
