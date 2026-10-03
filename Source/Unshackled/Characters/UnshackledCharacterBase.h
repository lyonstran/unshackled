#pragma once
#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "UnshackledCharacterBase.generated.h"

UCLASS()
class UNSHACKLED_API AUnshackledCharacterBase : public APaperCharacter
{
	GENERATED_BODY()
	
public:
	AUnshackledCharacterBase();
	void MoveCharacter(FVector2D Direction); 
};
