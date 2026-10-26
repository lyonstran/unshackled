#include "UnshackledGameInstance.h"
bool UUnshackledGameInstance::IsLastFloor() const
{
	return CurrentFloor >= MaxFloor;
}
bool UUnshackledGameInstance::AdvanceFloor()
{
	if (IsLastFloor())
	{
		return false;
	}
	CurrentFloor = CurrentFloor + 1;
	return true;
}
void UUnshackledGameInstance::ResetToFirstFloor()
{
	CurrentFloor = 1;
}
