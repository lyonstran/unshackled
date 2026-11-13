#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "UnshackledSaveGame.generated.h"
UCLASS()
class UNSHACKLED_API UUnshackledSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "Save")
	int32 SaveVersion = 1;
	UPROPERTY(VisibleAnywhere, Category = "Save")
	int32 Floor = 1;
	UPROPERTY(VisibleAnywhere, Category = "Save")
	FName CheckpointLevelName;
	UPROPERTY(VisibleAnywhere, Category = "Save")
	TArray<FName> RecruitedIds;
	UPROPERTY(VisibleAnywhere, Category = "Save")
	TArray<FName> CharacterIds;
	UPROPERTY(VisibleAnywhere, Category = "Save")
	TArray<float> CharacterHealth;
	UPROPERTY(VisibleAnywhere, Category = "Save")
	TArray<float> CharacterMana;
};
