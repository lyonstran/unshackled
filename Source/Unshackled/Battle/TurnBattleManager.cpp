#include "Battle/TurnBattleManager.h"
#include "Characters/PartyMemberCharacter.h"
#include "Characters/BossCharacter.h"
#include "UI/UnshackledHUD.h"
#include "Game/UnshackledGameInstance.h"
#include "InputCoreTypes.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
ATurnBattleManager::ATurnBattleManager()
{
	PrimaryActorTick.bCanEverTick = true;
}
void ATurnBattleManager::BeginPlay()
{
	Super::BeginPlay();
	SpawnMissingParty();
	SpawnBossIfMissing();
	FindParty();
	FindBoss();
	BuildBossRows();
	StartPlayerPhase();
}
FVector ATurnBattleManager::GetBattleOrigin() const
{
	APawn* LeaderPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LeaderPawn != nullptr)
	{
		return LeaderPawn->GetActorLocation();
	}
	return GetActorLocation();
}
APartyMemberCharacter* ATurnBattleManager::FindMemberByRow(FName CharacterId) const
{
	for (TActorIterator<APartyMemberCharacter> It(GetWorld()); It; ++It)
	{
		APartyMemberCharacter* Member = *It;
		if (Member->StatsRowName == CharacterId)
		{
			return Member;
		}
	}
	return nullptr;
}
void ATurnBattleManager::SpawnMissingParty()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
	FVector Origin = GetBattleOrigin();
	int32 SpawnedCount = 0;
	for (int32 i = 0; i < PartySlots.Num(); i++)
	{
		FBattlePartySlot Slot = PartySlots[i];
		bool bShouldSpawn = bSpawnWholePartyForTesting;
		if (GameInstanceRef != nullptr)
		{
			if (GameInstanceRef->IsRecruited(Slot.CharacterId))
			{
				bShouldSpawn = true;
			}
		}
		if (Slot.MemberClass == nullptr)
		{
			bShouldSpawn = false;
		}
		if (bShouldSpawn)
		{
			if (FindMemberByRow(Slot.CharacterId) == nullptr)
			{
				SpawnedCount = SpawnedCount + 1;
				FVector SpawnLocation = Origin + FVector(0.0f, PartySpacing * SpawnedCount, 0.0f);
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				World->SpawnActor<APartyMemberCharacter>(Slot.MemberClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
			}
		}
	}
}
void ATurnBattleManager::SpawnBossIfMissing()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	for (TActorIterator<ABossCharacter> It(World); It; ++It)
	{
		return;
	}
	int32 Floor = 1;
	UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
	if (GameInstanceRef != nullptr)
	{
		Floor = GameInstanceRef->CurrentFloor;
	}
	for (int32 i = 0; i < BossSlots.Num(); i++)
	{
		FBattleBossSlot Slot = BossSlots[i];
		if (Slot.Floor == Floor && Slot.BossClass != nullptr)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<ABossCharacter>(Slot.BossClass, GetBattleOrigin() + BossOffset, FRotator::ZeroRotator, SpawnParams);
			return;
		}
	}
}
void ATurnBattleManager::FindParty()
{
	Party.Empty();
	for (TActorIterator<APartyMemberCharacter> It(GetWorld()); It; ++It)
	{
		Party.Add(*It);
	}
	HasActed.Init(false, Party.Num());
	IsBlocking.Init(false, Party.Num());
}
void ATurnBattleManager::FindBoss()
{
	Boss = nullptr;
	for (TActorIterator<ABossCharacter> It(GetWorld()); It; ++It)
	{
		if (Boss == nullptr)
		{
			Boss = *It;
		}
	}
}
void ATurnBattleManager::BuildBossRows()
{
	BossRowNames.Empty();
	if (TelegraphTable == nullptr)
	{
		ShowMessage(TEXT("Battle: no telegraph table set"), FColor::Red);
		return;
	}
	if (Boss == nullptr)
	{
		ShowMessage(TEXT("Battle: no boss in the level"), FColor::Red);
		return;
	}
	FName BossKey = Boss->BossId;
	if (BossKey.IsNone())
	{
		BossKey = Boss->StatsRowName;
	}
	TArray<FName> AllNames = TelegraphTable->GetRowNames();
	for (int32 i = 0; i < AllNames.Num(); i++)
	{
		const FTelegraphRow* Row = GetRow(AllNames[i]);
		if (Row != nullptr)
		{
			if (Row->BossId == BossKey)
			{
				BossRowNames.Add(AllNames[i]);
			}
		}
	}
	FString Report = FString::Printf(TEXT("Battle: %d of %d table rows match boss '%s'"), BossRowNames.Num(), AllNames.Num(), *BossKey.ToString());
	if (BossRowNames.Num() == 0)
	{
		ShowMessage(Report, FColor::Red);
	}
}
const FTelegraphRow* ATurnBattleManager::GetRow(FName RowName) const
{
	if (TelegraphTable == nullptr)
	{
		return nullptr;
	}
	return TelegraphTable->FindRow<FTelegraphRow>(RowName, TEXT("TurnBattleManager"));
}
void ATurnBattleManager::StartPlayerPhase()
{
	State = EBattleState::PlayerPhase;
	HasActed.Init(false, Party.Num());
	if (Round > 1)
	{
		RestoreManaForNewRound();
	}
	ResetMenuToNextMember();
	ShowMessage(FString::Printf(TEXT("Round %d: your turn"), Round), FColor::Cyan);
}
void ATurnBattleManager::ResetMenuToNextMember()
{
	MenuStage = EBattleMenuStage::PickMember;
	MenuActionIndex = 0;
	int32 NextIndex = FindNextActingMember();
	if (NextIndex >= 0)
	{
		MenuMemberIndex = NextIndex;
	}
}
void ATurnBattleManager::MoveMemberSelection(int32 Step)
{
	int32 Count = Party.Num();
	if (Count == 0)
	{
		return;
	}
	int32 Candidate = MenuMemberIndex;
	for (int32 i = 0; i < Count; i++)
	{
		Candidate = Candidate + Step;
		if (Candidate < 0)
		{
			Candidate = Count - 1;
		}
		if (Candidate >= Count)
		{
			Candidate = 0;
		}
		if (CanMemberAct(Candidate))
		{
			MenuMemberIndex = Candidate;
			return;
		}
	}
}
void ATurnBattleManager::MenuStep(int32 Horizontal, int32 Vertical)
{
	if (State != EBattleState::PlayerPhase)
	{
		return;
	}
	if (MenuStage == EBattleMenuStage::PickMember)
	{
		if (Horizontal != 0)
		{
			MoveMemberSelection(Horizontal);
		}
		return;
	}
	if (Vertical != 0)
	{
		int32 Count = GetMenuActionCount();
		MenuActionIndex = MenuActionIndex - Vertical;
		if (MenuActionIndex < 0)
		{
			MenuActionIndex = Count - 1;
		}
		if (MenuActionIndex >= Count)
		{
			MenuActionIndex = 0;
		}
	}
}
void ATurnBattleManager::MenuConfirm()
{
	if (State == EBattleState::Won)
	{
		if (bWaitingForContinue)
		{
			if (!bEndHandled)
			{
				bEndHandled = true;
				bWaitingForContinue = false;
				LeaveAfterWin();
			}
		}
		return;
	}
	if (State != EBattleState::PlayerPhase)
	{
		return;
	}
	if (MenuStage == EBattleMenuStage::PickMember)
	{
		if (CanMemberAct(MenuMemberIndex))
		{
			MenuStage = EBattleMenuStage::PickAction;
			MenuActionIndex = 0;
		}
		return;
	}
	EBattleAction Action = GetMenuAction(MenuActionIndex);
	if (!IsActionUsable(MenuMemberIndex, Action))
	{
		ShowMessage(TEXT("Cannot use that right now"), FColor::Orange);
		return;
	}
	bool bActed = TryPlayerAction(MenuMemberIndex, Action);
	if (bActed)
	{
		ResetMenuToNextMember();
	}
}
void ATurnBattleManager::MenuBack()
{
	if (State != EBattleState::PlayerPhase)
	{
		return;
	}
	if (MenuStage == EBattleMenuStage::PickAction)
	{
		MenuStage = EBattleMenuStage::PickMember;
	}
}
EBattleMenuStage ATurnBattleManager::GetMenuStage() const
{
	return MenuStage;
}
int32 ATurnBattleManager::GetMenuMemberIndex() const
{
	return MenuMemberIndex;
}
int32 ATurnBattleManager::GetMenuActionIndex() const
{
	return MenuActionIndex;
}
int32 ATurnBattleManager::GetMenuActionCount() const
{
	return 5;
}
EBattleAction ATurnBattleManager::GetMenuAction(int32 Index) const
{
	if (Index == 0)
	{
		return EBattleAction::Attack;
	}
	if (Index == 1)
	{
		return EBattleAction::Special;
	}
	if (Index == 2)
	{
		return EBattleAction::Ultimate;
	}
	if (Index == 3)
	{
		return EBattleAction::Item;
	}
	if (Index == 4)
	{
		return EBattleAction::Block;
	}
	return EBattleAction::None;
}
FString ATurnBattleManager::GetActionLabel(EBattleAction Action) const
{
	if (Action == EBattleAction::Attack)
	{
		return TEXT("Attack");
	}
	if (Action == EBattleAction::Special)
	{
		return TEXT("Special");
	}
	if (Action == EBattleAction::Ultimate)
	{
		return TEXT("Ultimate");
	}
	if (Action == EBattleAction::Item)
	{
		return TEXT("Item");
	}
	if (Action == EBattleAction::Block)
	{
		return TEXT("Block");
	}
	return TEXT("");
}
bool ATurnBattleManager::IsActionUsable(int32 MemberIndex, EBattleAction Action) const
{
	if (MemberIndex < 0 || MemberIndex >= Party.Num())
	{
		return false;
	}
	APartyMemberCharacter* Member = Party[MemberIndex];
	if (Action == EBattleAction::None || Action == EBattleAction::Item)
	{
		return false;
	}
	if (Action == EBattleAction::Special)
	{
		return Member->Stats->CurrentMana >= Member->SpecialManaCost;
	}
	if (Action == EBattleAction::Ultimate)
	{
		return Member->Stats->CurrentMana >= Member->UltimateManaCost;
	}
	return true;
}
void ATurnBattleManager::HoldPartyInPlace()
{
	for (int32 i = 0; i < Party.Num(); i++)
	{
		if (Party[i]->IsFollowing())
		{
			Party[i]->SetFollowing(false);
		}
	}
}
void ATurnBattleManager::StopRealTimeManaRegen()
{
	for (int32 i = 0; i < Party.Num(); i++)
	{
		Party[i]->ManaRegenPerSecond = 0.0f;
	}
}
void ATurnBattleManager::RestoreManaForNewRound()
{
	for (int32 i = 0; i < Party.Num(); i++)
	{
		if (!Party[i]->Health->IsDead())
		{
			float MaxMana = Party[i]->Stats->GetFinal(ECharacterStat::MaxMana);
			Party[i]->Stats->RestoreMana(MaxMana * ManaRestoreFraction);
		}
	}
}
void ATurnBattleManager::StartBossPhase()
{
	State = EBattleState::BossPhase;
	int32 Turns = CountLivingMembers();
	if (Boss != nullptr)
	{
		float HealthFraction = Boss->Health->CurrentHealth / Boss->Health->MaxHealth;
		if (HealthFraction <= ExtraTurnHealthFraction)
		{
			Turns = Turns + 1;
		}
	}
	BossTurnsLeft = Turns;
	BossTurnTimer = BossTurnDelay;
	ShowMessage(FString::Printf(TEXT("Boss phase: %d turns"), Turns), FColor::Orange);
}
void ATurnBattleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	StopRealTimeManaRegen();
	HoldPartyInPlace();
	CheckDebugKeys();
	if (State == EBattleState::BossPhase)
	{
		TickBossPhase(DeltaTime);
	}
	if (State == EBattleState::Won || State == EBattleState::Lost)
	{
		TickEnd(DeltaTime);
	}
}
void ATurnBattleManager::TickEnd(float DeltaTime)
{
	if (bEndHandled)
	{
		return;
	}
	EndTimer = EndTimer + DeltaTime;
	if (State == EBattleState::Won)
	{
		if (EndTimer >= WinInputDelay)
		{
			bWaitingForContinue = true;
			GEngine->AddOnScreenDebugMessage(7100, 0.0f, FColor::Green, TEXT("Press Space to continue"));
		}
		return;
	}
	if (EndTimer < EndDelay)
	{
		return;
	}
	bEndHandled = true;
	LeaveAfterLoss();
}
void ATurnBattleManager::CheckDebugKeys()
{
	if (!bDebugLoseKey)
	{
		return;
	}
	if (State == EBattleState::Won || State == EBattleState::Lost)
	{
		return;
	}
	APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
	if (Controller == nullptr)
	{
		return;
	}
	if (Controller->WasInputKeyJustPressed(EKeys::L))
	{
		State = EBattleState::Lost;
		ShowBanner(TEXT("Game Over"), FLinearColor::Red);
	}
}
void ATurnBattleManager::ReviveFallenMembers()
{
	for (int32 i = 0; i < Party.Num(); i++)
	{
		if (Party[i]->Health->IsDead())
		{
			Party[i]->ReviveCharacter();
		}
	}
}
void ATurnBattleManager::LeaveAfterWin()
{
	UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
	if (GameInstanceRef == nullptr)
	{
		ShowMessage(TEXT("Battle: set Game Instance Class in Project Settings"), FColor::Red);
		return;
	}
	ReviveFallenMembers();
	if (GameInstanceRef->IsLastFloor())
	{
		ShowBanner(TEXT("You escaped!"), FLinearColor(1.0f, 0.85f, 0.2f, 1.0f));
		return;
	}
	GameInstanceRef->AdvanceFloor();
	GameInstanceRef->SaveFloorStartStates();
	UGameplayStatics::OpenLevel(this, OverworldLevelName);
}
void ATurnBattleManager::LeaveAfterLoss()
{
	FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}
void ATurnBattleManager::TickBossPhase(float DeltaTime)
{
	BossTurnTimer = BossTurnTimer - DeltaTime;
	if (BossTurnTimer > 0.0f)
	{
		return;
	}
	if (BossTurnsLeft <= 0)
	{
		Round = Round + 1;
		StartPlayerPhase();
		return;
	}
	DoBossTurn();
	BossTurnsLeft = BossTurnsLeft - 1;
	BossTurnTimer = BossTurnDelay;
}
void ATurnBattleManager::DoBossTurn()
{
	if (BossRowNames.Num() == 0)
	{
		ShowMessage(TEXT("Boss has no telegraphs to set up"), FColor::Red);
		return;
	}
	int32 RowIndex = FMath::RandRange(0, BossRowNames.Num() - 1);
	FName ChosenName = BossRowNames[RowIndex];
	const FTelegraphRow* Row = GetRow(ChosenName);
	if (Row == nullptr)
	{
		return;
	}
	FPendingTelegraph NewTelegraph;
	NewTelegraph.RowName = ChosenName;
	NewTelegraph.ActionsLeft = Row->CountdownActions;
	NewTelegraph.TargetIndex = PickTargetIndex(Row->TargetMode);
	Pending.Add(NewTelegraph);
	ShowMessage(FString::Printf(TEXT("Boss sets up %s (lands in %d actions)"), *Row->DisplayName.ToString(), Row->CountdownActions), FColor::Orange);
}
bool ATurnBattleManager::TryPlayerAction(int32 MemberIndex, EBattleAction Action)
{
	if (State != EBattleState::PlayerPhase)
	{
		return false;
	}
	if (!CanMemberAct(MemberIndex))
	{
		return false;
	}
	if (Action == EBattleAction::None || Action == EBattleAction::Item)
	{
		return false;
	}
	APartyMemberCharacter* Member = Party[MemberIndex];
	if (Action == EBattleAction::Special)
	{
		if (!Member->Stats->SpendMana(Member->SpecialManaCost))
		{
			ShowMessage(TEXT("Not enough mana"), FColor::Orange);
			return false;
		}
	}
	if (Action == EBattleAction::Ultimate)
	{
		if (!Member->Stats->SpendMana(Member->UltimateManaCost))
		{
			ShowMessage(TEXT("Not enough mana"), FColor::Orange);
			return false;
		}
	}
	HasActed[MemberIndex] = true;
	IsBlocking[MemberIndex] = false;
	int32 CounterIndex = FindCounterIndex(Member, Action);
	if (CounterIndex >= 0)
	{
		FPendingTelegraph Countered = Pending[CounterIndex];
		RemovePending(CounterIndex);
		ShowMessage(TEXT("Counter!"), FColor::Green);
		ResolveTelegraph(Countered, true);
	}
	ApplyActionEffect(MemberIndex, Action);
	CheckForEnd();
	if (State == EBattleState::Won)
	{
		return true;
	}
	AdvanceTelegraphs();
	CheckForEnd();
	if (State == EBattleState::PlayerPhase)
	{
		if (AllLivingMembersActed())
		{
			StartBossPhase();
		}
	}
	return true;
}
void ATurnBattleManager::ApplyActionEffect(int32 MemberIndex, EBattleAction Action)
{
	APartyMemberCharacter* Member = Party[MemberIndex];
	if (Action == EBattleAction::Block)
	{
		IsBlocking[MemberIndex] = true;
		ShowMessage(FString::Printf(TEXT("%s blocks"), *Member->DisplayName), FColor::Cyan);
		return;
	}
	if (Boss == nullptr)
	{
		return;
	}
	float Scale = AttackScale;
	if (Action == EBattleAction::Special)
	{
		Scale = SpecialScale;
	}
	if (Action == EBattleAction::Ultimate)
	{
		Scale = UltimateScale;
	}
	Member->DealDamage(Boss, Scale, false);
	ShowMessage(FString::Printf(TEXT("%s hits the boss"), *Member->DisplayName), FColor::Cyan);
}
void ATurnBattleManager::AdvanceTelegraphs()
{
	TArray<FPendingTelegraph> StillWaiting;
	TArray<FPendingTelegraph> ToResolve;
	for (int32 i = 0; i < Pending.Num(); i++)
	{
		FPendingTelegraph Item = Pending[i];
		Item.ActionsLeft = Item.ActionsLeft - 1;
		if (Item.ActionsLeft <= 0)
		{
			ToResolve.Add(Item);
		}
		else
		{
			StillWaiting.Add(Item);
		}
	}
	Pending = StillWaiting;
	for (int32 i = 0; i < ToResolve.Num(); i++)
	{
		ResolveTelegraph(ToResolve[i], false);
	}
}
void ATurnBattleManager::ResolveTelegraph(FPendingTelegraph Telegraph, bool bCountered)
{
	const FTelegraphRow* Row = GetRow(Telegraph.RowName);
	if (Row == nullptr)
	{
		return;
	}
	float Multiplier = 1.0f;
	if (bCountered)
	{
		Multiplier = Row->CounterMultiplier;
	}
	ShowMessage(FString::Printf(TEXT("%s lands"), *Row->DisplayName.ToString()), FColor::Red);
	if (Row->TargetMode == EBossTargetMode::WholeParty)
	{
		for (int32 i = 0; i < Party.Num(); i++)
		{
			HitMember(i, Row, Multiplier);
		}
		return;
	}
	int32 TargetIndex = Telegraph.TargetIndex;
	bool bTargetAlive = false;
	if (TargetIndex >= 0 && TargetIndex < Party.Num())
	{
		if (!Party[TargetIndex]->Health->IsDead())
		{
			bTargetAlive = true;
		}
	}
	if (!bTargetAlive)
	{
		TargetIndex = PickRandomLivingMember();
	}
	HitMember(TargetIndex, Row, Multiplier);
}
void ATurnBattleManager::HitMember(int32 MemberIndex, const FTelegraphRow* Row, float Multiplier)
{
	if (MemberIndex < 0 || MemberIndex >= Party.Num())
	{
		return;
	}
	APartyMemberCharacter* Member = Party[MemberIndex];
	if (Member->Health->IsDead() || Boss == nullptr)
	{
		return;
	}
	float FinalMultiplier = Multiplier;
	if (IsBlocking[MemberIndex])
	{
		FinalMultiplier = FinalMultiplier * Row->BlockMultiplier;
	}
	if (FinalMultiplier <= 0.0f)
	{
		return;
	}
	Boss->DealDamage(Member, Row->DamageScale * FinalMultiplier, false);
}
void ATurnBattleManager::RemovePending(int32 Index)
{
	TArray<FPendingTelegraph> Remaining;
	for (int32 i = 0; i < Pending.Num(); i++)
	{
		if (i != Index)
		{
			Remaining.Add(Pending[i]);
		}
	}
	Pending = Remaining;
}
void ATurnBattleManager::CheckForEnd()
{
	if (State == EBattleState::Won || State == EBattleState::Lost)
	{
		return;
	}
	if (Boss != nullptr)
	{
		if (Boss->Health->IsDead())
		{
			State = EBattleState::Won;
			ShowBanner(TEXT("Victory!"), FLinearColor::Green);
			return;
		}
	}
	if (CountLivingMembers() == 0)
	{
		State = EBattleState::Lost;
		ShowBanner(TEXT("Game Over"), FLinearColor::Red);
	}
}
bool ATurnBattleManager::CanMemberAct(int32 MemberIndex) const
{
	if (MemberIndex < 0 || MemberIndex >= Party.Num())
	{
		return false;
	}
	if (Party[MemberIndex] == nullptr)
	{
		return false;
	}
	if (Party[MemberIndex]->Health->IsDead())
	{
		return false;
	}
	return !HasActed[MemberIndex];
}
bool ATurnBattleManager::AllLivingMembersActed() const
{
	for (int32 i = 0; i < Party.Num(); i++)
	{
		if (CanMemberAct(i))
		{
			return false;
		}
	}
	return true;
}
int32 ATurnBattleManager::CountLivingMembers() const
{
	int32 Count = 0;
	for (int32 i = 0; i < Party.Num(); i++)
	{
		if (!Party[i]->Health->IsDead())
		{
			Count = Count + 1;
		}
	}
	return Count;
}
int32 ATurnBattleManager::FindNextActingMember() const
{
	for (int32 i = 0; i < Party.Num(); i++)
	{
		if (CanMemberAct(i))
		{
			return i;
		}
	}
	return -1;
}
int32 ATurnBattleManager::FindCounterIndex(APartyMemberCharacter* Member, EBattleAction Action) const
{
	for (int32 i = 0; i < Pending.Num(); i++)
	{
		const FTelegraphRow* Row = GetRow(Pending[i].RowName);
		if (Row != nullptr)
		{
			if (Row->CounterMemberId == Member->StatsRowName && Row->CounterAction == Action)
			{
				return i;
			}
		}
	}
	return -1;
}
int32 ATurnBattleManager::PickRandomLivingMember() const
{
	TArray<int32> LivingIndices;
	for (int32 i = 0; i < Party.Num(); i++)
	{
		if (!Party[i]->Health->IsDead())
		{
			LivingIndices.Add(i);
		}
	}
	if (LivingIndices.Num() == 0)
	{
		return -1;
	}
	int32 Pick = FMath::RandRange(0, LivingIndices.Num() - 1);
	return LivingIndices[Pick];
}
int32 ATurnBattleManager::FindLeaderIndex() const
{
	APawn* LeaderPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	for (int32 i = 0; i < Party.Num(); i++)
	{
		if (Party[i] == LeaderPawn && !Party[i]->Health->IsDead())
		{
			return i;
		}
	}
	return PickRandomLivingMember();
}
int32 ATurnBattleManager::PickTargetIndex(EBossTargetMode Mode) const
{
	if (Mode == EBossTargetMode::WholeParty)
	{
		return -1;
	}
	if (Mode == EBossTargetMode::Leader)
	{
		return FindLeaderIndex();
	}
	return PickRandomLivingMember();
}
void ATurnBattleManager::ShowMessage(FString Text, FColor Color)
{
	if (GEngine != nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, Color, Text);
	}
}
void ATurnBattleManager::ShowBanner(FString Text, FLinearColor Color)
{
	APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
	if (Controller == nullptr)
	{
		return;
	}
	AUnshackledHUD* Hud = Cast<AUnshackledHUD>(Controller->GetHUD());
	if (Hud != nullptr)
	{
		Hud->ShowBanner(Text, Color, 5.0f);
	}
}
EBattleState ATurnBattleManager::GetState() const
{
	return State;
}
int32 ATurnBattleManager::GetRound() const
{
	return Round;
}
bool ATurnBattleManager::IsMemberBlocking(int32 MemberIndex) const
{
	if (MemberIndex < 0 || MemberIndex >= IsBlocking.Num())
	{
		return false;
	}
	return IsBlocking[MemberIndex];
}
int32 ATurnBattleManager::GetPartyCount() const
{
	return Party.Num();
}
APartyMemberCharacter* ATurnBattleManager::GetPartyMember(int32 Index) const
{
	if (Index < 0 || Index >= Party.Num())
	{
		return nullptr;
	}
	return Party[Index];
}
ABossCharacter* ATurnBattleManager::GetBoss() const
{
	return Boss;
}
int32 ATurnBattleManager::GetPendingCount() const
{
	return Pending.Num();
}
FString ATurnBattleManager::GetPendingName(int32 Index) const
{
	if (Index < 0 || Index >= Pending.Num())
	{
		return FString();
	}
	const FTelegraphRow* Row = GetRow(Pending[Index].RowName);
	if (Row == nullptr)
	{
		return FString();
	}
	return Row->DisplayName.ToString();
}
int32 ATurnBattleManager::GetPendingActionsLeft(int32 Index) const
{
	if (Index < 0 || Index >= Pending.Num())
	{
		return 0;
	}
	return Pending[Index].ActionsLeft;
}
