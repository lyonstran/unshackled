#include "Components/ItemMenuComponent.h"
#include "Components/GameMenuComponent.h"
#include "Game/UnshackledGameInstance.h"
#include "Player/UnshackledPlayerController.h"
#include "Characters/PartyMemberCharacter.h"
#include "Items/ItemRows.h"
#include "UI/UnshackledHUD.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
UItemMenuComponent::UItemMenuComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
UUnshackledGameInstance* UItemMenuComponent::GetUnshackledGameInstance() const
{
	UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(UGameplayStatics::GetGameInstance(this));
	return GameInstanceRef;
}
AUnshackledPlayerController* UItemMenuComponent::GetOwnerController() const
{
	AUnshackledPlayerController* Controller = Cast<AUnshackledPlayerController>(GetOwner());
	return Controller;
}
void UItemMenuComponent::ToggleMenu(EItemMenuMode NewMode)
{
	if (bOpen)
	{
		if (Mode == NewMode)
		{
			CloseMenu();
			return;
		}
		Mode = NewMode;
		Stage = EItemMenuStage::PickEntry;
		SelectedIndex = 0;
		StatusText = TEXT("");
		BuildEntries();
		return;
	}
	OpenMenu(NewMode);
}
void UItemMenuComponent::OpenMenu(EItemMenuMode NewMode)
{
	if (bOpen)
	{
		return;
	}
	AUnshackledPlayerController* Controller = GetOwnerController();
	if (Controller == nullptr)
	{
		return;
	}
	if (Controller->GameMenu != nullptr)
	{
		if (Controller->GameMenu->IsOpen())
		{
			return;
		}
	}
	Mode = NewMode;
	Stage = EItemMenuStage::PickEntry;
	SelectedIndex = 0;
	MemberIndex = 0;
	bNavigateLatched = false;
	StatusText = TEXT("");
	BuildEntries();
	BuildMembers();
	bOpen = true;
	UGameplayStatics::SetGamePaused(this, true);
	AddMenuInput();
}
void UItemMenuComponent::CloseMenu()
{
	if (!bOpen)
	{
		return;
	}
	bOpen = false;
	RemoveMenuInput();
	UGameplayStatics::SetGamePaused(this, false);
}
bool UItemMenuComponent::IsOpen() const
{
	return bOpen;
}
EItemMenuMode UItemMenuComponent::GetMode() const
{
	return Mode;
}
EItemMenuStage UItemMenuComponent::GetStage() const
{
	return Stage;
}
void UItemMenuComponent::AddMenuInput()
{
	AUnshackledPlayerController* Controller = GetOwnerController();
	if (Controller == nullptr)
	{
		return;
	}
	if (Controller->GameMenu == nullptr)
	{
		return;
	}
	if (Controller->GameMenu->MenuMappingContext == nullptr)
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer());
	if (InputSystem == nullptr)
	{
		return;
	}
	InputSystem->AddMappingContext(Controller->GameMenu->MenuMappingContext, Controller->GameMenu->MenuMappingPriority);
}
void UItemMenuComponent::RemoveMenuInput()
{
	AUnshackledPlayerController* Controller = GetOwnerController();
	if (Controller == nullptr)
	{
		return;
	}
	if (Controller->GameMenu == nullptr)
	{
		return;
	}
	if (Controller->GameMenu->MenuMappingContext == nullptr)
	{
		return;
	}
	UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer());
	if (InputSystem == nullptr)
	{
		return;
	}
	InputSystem->RemoveMappingContext(Controller->GameMenu->MenuMappingContext);
}
void UItemMenuComponent::BuildEntries()
{
	Entries.Empty();
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		return;
	}
	if (Mode == EItemMenuMode::Items)
	{
		Entries = GameInstanceRef->GetHeldItemIds();
	}
	else
	{
		Entries = GameInstanceRef->GetOwnedArtifacts();
	}
	if (SelectedIndex >= Entries.Num())
	{
		SelectedIndex = Entries.Num() - 1;
	}
	if (SelectedIndex < 0)
	{
		SelectedIndex = 0;
	}
}
void UItemMenuComponent::BuildMembers()
{
	Members.Empty();
	AUnshackledPlayerController* Controller = GetOwnerController();
	if (Controller == nullptr)
	{
		return;
	}
	Controller->BuildPartyList();
	for (int32 i = 0; i < Controller->GetPartyCount(); i++)
	{
		APartyMemberCharacter* Member = Controller->GetPartyMember(i);
		if (Member != nullptr)
		{
			Members.Add(Member);
		}
	}
}
void UItemMenuComponent::HandleNavigate(FVector2D Axis)
{
	if (!bOpen)
	{
		return;
	}
	float AbsX = FMath::Abs(Axis.X);
	float AbsY = FMath::Abs(Axis.Y);
	if (bNavigateLatched)
	{
		if (AbsX < ReleaseThreshold)
		{
			if (AbsY < ReleaseThreshold)
			{
				bNavigateLatched = false;
			}
		}
		return;
	}
	float Strongest = AbsX;
	if (AbsY > Strongest)
	{
		Strongest = AbsY;
	}
	if (Strongest < PressThreshold)
	{
		return;
	}
	bNavigateLatched = true;
	if (Stage == EItemMenuStage::PickMember)
	{
		if (AbsX > AbsY)
		{
			if (Axis.X > 0.0f)
			{
				StepMember(1);
			}
			else
			{
				StepMember(-1);
			}
		}
		return;
	}
	if (AbsY > AbsX)
	{
		if (Axis.Y > 0.0f)
		{
			StepEntry(-1);
		}
		else
		{
			StepEntry(1);
		}
	}
}
void UItemMenuComponent::HandleNavigateReleased()
{
	bNavigateLatched = false;
}
void UItemMenuComponent::StepEntry(int32 Direction)
{
	int32 Count = Entries.Num();
	if (Count == 0)
	{
		return;
	}
	SelectedIndex = SelectedIndex + Direction;
	if (SelectedIndex < 0)
	{
		SelectedIndex = Count - 1;
	}
	if (SelectedIndex >= Count)
	{
		SelectedIndex = 0;
	}
	StatusText = TEXT("");
}
void UItemMenuComponent::StepMember(int32 Direction)
{
	int32 Count = Members.Num();
	if (Count == 0)
	{
		return;
	}
	int32 Candidate = MemberIndex;
	for (int32 i = 0; i < Count; i++)
	{
		Candidate = Candidate + Direction;
		if (Candidate < 0)
		{
			Candidate = Count - 1;
		}
		if (Candidate >= Count)
		{
			Candidate = 0;
		}
		if (CanPickMember(Candidate))
		{
			MemberIndex = Candidate;
			return;
		}
	}
}
void UItemMenuComponent::Confirm()
{
	if (!bOpen)
	{
		return;
	}
	if (Stage == EItemMenuStage::PickMember)
	{
		ConfirmMember();
		return;
	}
	ConfirmEntry();
}
void UItemMenuComponent::Back()
{
	if (!bOpen)
	{
		return;
	}
	if (Stage == EItemMenuStage::PickMember)
	{
		Stage = EItemMenuStage::PickEntry;
		StatusText = TEXT("");
		return;
	}
	CloseMenu();
}
FName UItemMenuComponent::GetSelectedEntry() const
{
	if (SelectedIndex < 0)
	{
		return NAME_None;
	}
	if (SelectedIndex >= Entries.Num())
	{
		return NAME_None;
	}
	return Entries[SelectedIndex];
}
void UItemMenuComponent::ConfirmEntry()
{
	FName EntryId = GetSelectedEntry();
	if (EntryId.IsNone())
	{
		return;
	}
	if (Mode == EItemMenuMode::Artifacts)
	{
		ToggleSelectedArtifact();
		return;
	}
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		return;
	}
	const FActiveItemRow* Row = GameInstanceRef->GetItemRow(EntryId);
	if (Row == nullptr)
	{
		StatusText = TEXT("Unknown item (reimport DT_Items)");
		return;
	}
	if (Row->bWholeParty)
	{
		UseOnWholeParty(EntryId);
		return;
	}
	StartMemberPick();
}
void UItemMenuComponent::StartMemberPick()
{
	BuildMembers();
	MemberIndex = -1;
	APawn* LeaderPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	for (int32 i = 0; i < Members.Num(); i++)
	{
		if (Members[i] == LeaderPawn)
		{
			if (CanPickMember(i))
			{
				MemberIndex = i;
			}
		}
	}
	if (MemberIndex < 0)
	{
		for (int32 i = 0; i < Members.Num(); i++)
		{
			if (MemberIndex < 0)
			{
				if (CanPickMember(i))
				{
					MemberIndex = i;
				}
			}
		}
	}
	if (MemberIndex < 0)
	{
		MemberIndex = 0;
		StatusText = TEXT("Nobody can use that right now");
		return;
	}
	StatusText = TEXT("");
	Stage = EItemMenuStage::PickMember;
}
void UItemMenuComponent::ConfirmMember()
{
	if (!CanPickMember(MemberIndex))
	{
		StatusText = TEXT("Choose someone else");
		return;
	}
	UseOnMember(GetSelectedEntry(), Members[MemberIndex]);
}
void UItemMenuComponent::ToggleSelectedArtifact()
{
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		return;
	}
	FName ArtifactId = GetSelectedEntry();
	bool bChanged = GameInstanceRef->ToggleArtifact(ArtifactId);
	if (!bChanged)
	{
		StatusText = FString::Printf(TEXT("All %d slots are full: unequip one first"), GameInstanceRef->MaxEquippedArtifacts);
		return;
	}
	if (GameInstanceRef->IsArtifactEquipped(ArtifactId))
	{
		StatusText = FString::Printf(TEXT("Equipped %s"), *GameInstanceRef->GetArtifactName(ArtifactId));
	}
	else
	{
		StatusText = FString::Printf(TEXT("Unequipped %s"), *GameInstanceRef->GetArtifactName(ArtifactId));
	}
}
void UItemMenuComponent::UseOnWholeParty(FName ItemId)
{
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		return;
	}
	const FActiveItemRow* Row = GameInstanceRef->GetItemRow(ItemId);
	BuildMembers();
	int32 UsedCount = 0;
	for (int32 i = 0; i < Members.Num(); i++)
	{
		if (Members[i]->ReceiveItem(Row, false))
		{
			UsedCount = UsedCount + 1;
		}
	}
	if (UsedCount == 0)
	{
		StatusText = TEXT("Nobody can use that right now");
		return;
	}
	FinishUse(ItemId, TEXT("the party"));
}
void UItemMenuComponent::UseOnMember(FName ItemId, APartyMemberCharacter* Member)
{
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		return;
	}
	if (Member == nullptr)
	{
		return;
	}
	const FActiveItemRow* Row = GameInstanceRef->GetItemRow(ItemId);
	bool bUsed = Member->ReceiveItem(Row, false);
	if (!bUsed)
	{
		StatusText = TEXT("That has no effect on them");
		return;
	}
	FinishUse(ItemId, Member->DisplayName);
}
void UItemMenuComponent::FinishUse(FName ItemId, FString TargetText)
{
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	FString ItemName = GameInstanceRef->GetItemName(ItemId);
	GameInstanceRef->ConsumeItem(ItemId);
	AUnshackledHUD* Hud = AUnshackledHUD::FindPlayerHUD(this);
	if (Hud != nullptr)
	{
		Hud->AddMessage(FString::Printf(TEXT("Used %s on %s"), *ItemName, *TargetText), FLinearColor(0.6f, 1.0f, 0.6f, 1.0f), 3.0f);
	}
	CloseMenu();
}
int32 UItemMenuComponent::GetEntryCount() const
{
	return Entries.Num();
}
int32 UItemMenuComponent::GetSelectedIndex() const
{
	return SelectedIndex;
}
FString UItemMenuComponent::GetEntryLabel(int32 Index) const
{
	if (Index < 0)
	{
		return FString();
	}
	if (Index >= Entries.Num())
	{
		return FString();
	}
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		return Entries[Index].ToString();
	}
	if (Mode == EItemMenuMode::Items)
	{
		int32 Count = GameInstanceRef->GetItemCount(Entries[Index]);
		return FString::Printf(TEXT("%s  x%d"), *GameInstanceRef->GetItemName(Entries[Index]), Count);
	}
	FString Marker = TEXT("[  ] ");
	if (GameInstanceRef->IsArtifactEquipped(Entries[Index]))
	{
		Marker = TEXT("[E] ");
	}
	return Marker + GameInstanceRef->GetArtifactName(Entries[Index]);
}
FString UItemMenuComponent::GetSelectedDescription() const
{
	FName EntryId = GetSelectedEntry();
	if (EntryId.IsNone())
	{
		return FString();
	}
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		return FString();
	}
	if (Mode == EItemMenuMode::Items)
	{
		const FActiveItemRow* ItemRow = GameInstanceRef->GetItemRow(EntryId);
		if (ItemRow == nullptr)
		{
			return FString();
		}
		return ItemRow->Description.ToString();
	}
	const FArtifactRow* ArtifactRow = GameInstanceRef->GetArtifactRow(EntryId);
	if (ArtifactRow == nullptr)
	{
		return FString();
	}
	return ArtifactRow->Description.ToString();
}
FString UItemMenuComponent::GetTitle() const
{
	if (Mode == EItemMenuMode::Items)
	{
		if (Stage == EItemMenuStage::PickMember)
		{
			return TEXT("Use on who?");
		}
		return TEXT("Items");
	}
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		return TEXT("Artifacts");
	}
	return FString::Printf(TEXT("Artifacts (equipped %d / %d)"), GameInstanceRef->GetEquippedCount(), GameInstanceRef->MaxEquippedArtifacts);
}
FString UItemMenuComponent::GetStatusText() const
{
	return StatusText;
}
int32 UItemMenuComponent::GetMemberCount() const
{
	return Members.Num();
}
APartyMemberCharacter* UItemMenuComponent::GetMember(int32 Index) const
{
	if (Index < 0)
	{
		return nullptr;
	}
	if (Index >= Members.Num())
	{
		return nullptr;
	}
	return Members[Index];
}
int32 UItemMenuComponent::GetMemberIndex() const
{
	return MemberIndex;
}
bool UItemMenuComponent::CanPickMember(int32 Index) const
{
	APartyMemberCharacter* Member = GetMember(Index);
	if (Member == nullptr)
	{
		return false;
	}
	UUnshackledGameInstance* GameInstanceRef = GetUnshackledGameInstance();
	if (GameInstanceRef == nullptr)
	{
		return false;
	}
	const FActiveItemRow* Row = GameInstanceRef->GetItemRow(GetSelectedEntry());
	return Member->CanReceiveItem(Row);
}
