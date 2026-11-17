#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemMenuComponent.generated.h"
class UUnshackledGameInstance;
class APartyMemberCharacter;
class AUnshackledPlayerController;
enum class EItemMenuMode : uint8
{
	Items,
	Artifacts
};
enum class EItemMenuStage : uint8
{
	PickEntry,
	PickMember
};
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UNSHACKLED_API UItemMenuComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UItemMenuComponent();
	UPROPERTY(EditAnywhere, Category = "Menu")
	float PressThreshold = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Menu")
	float ReleaseThreshold = 0.3f;
	void ToggleMenu(EItemMenuMode NewMode);
	void OpenMenu(EItemMenuMode NewMode);
	void CloseMenu();
	bool IsOpen() const;
	EItemMenuMode GetMode() const;
	EItemMenuStage GetStage() const;
	void HandleNavigate(FVector2D Axis);
	void HandleNavigateReleased();
	void Confirm();
	void Back();
	int32 GetEntryCount() const;
	int32 GetSelectedIndex() const;
	FString GetEntryLabel(int32 Index) const;
	FString GetSelectedDescription() const;
	FString GetTitle() const;
	FString GetStatusText() const;
	int32 GetMemberCount() const;
	APartyMemberCharacter* GetMember(int32 Index) const;
	int32 GetMemberIndex() const;
	bool CanPickMember(int32 Index) const;
private:
	bool bOpen = false;
	bool bNavigateLatched = false;
	EItemMenuMode Mode = EItemMenuMode::Items;
	EItemMenuStage Stage = EItemMenuStage::PickEntry;
	int32 SelectedIndex = 0;
	int32 MemberIndex = 0;
	TArray<FName> Entries;
	FString StatusText;
	UPROPERTY()
	TArray<APartyMemberCharacter*> Members;
	UUnshackledGameInstance* GetUnshackledGameInstance() const;
	AUnshackledPlayerController* GetOwnerController() const;
	FName GetSelectedEntry() const;
	void BuildEntries();
	void BuildMembers();
	void StepEntry(int32 Direction);
	void StepMember(int32 Direction);
	void ConfirmEntry();
	void ConfirmMember();
	void ToggleSelectedArtifact();
	void StartMemberPick();
	void UseOnWholeParty(FName ItemId);
	void UseOnMember(FName ItemId, APartyMemberCharacter* Member);
	void FinishUse(FName ItemId, FString TargetText);
	void AddMenuInput();
	void RemoveMenuInput();
};
