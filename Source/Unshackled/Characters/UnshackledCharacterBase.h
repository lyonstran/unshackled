#pragma once
#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "UnshackledCharacterBase.generated.h"

UCLASS()
class UNSHACKLED_API AUnshackledCharacterBase : public APaperZDCharacter
{
	GENERATED_BODY()
	
public:
	AUnshackledCharacterBase();
	void MoveCharacter(FVector2D Direction); 
};
