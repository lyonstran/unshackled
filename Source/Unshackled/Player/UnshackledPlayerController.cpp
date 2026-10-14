#include "UnshackledPlayerController.h"
#include "Characters/UnshackledCharacterBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
void AUnshackledPlayerController::BeginPlay()
{
        Super::BeginPlay();
        bShowMouseCursor = true;
        UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
        if (InputSystem != nullptr && MappingContext != nullptr)
        {
                InputSystem->AddMappingContext(MappingContext, 0);
        }
}
void AUnshackledPlayerController::PlayerTick(float DeltaTime)
{
        Super::PlayerTick(DeltaTime);
        float MouseDeltaX = 0.0f;
        float MouseDeltaY = 0.0f;
        GetInputMouseDelta(MouseDeltaX, MouseDeltaY);
        if (MouseDeltaX != 0.0f)
        {
                bAimWithStick = false;
        }
        if (MouseDeltaY != 0.0f)
        {
                bAimWithStick = false;
        }
}
void AUnshackledPlayerController::Aim(const FInputActionValue& Value)
{
        FVector2D StickInput = Value.Get<FVector2D>();
        float StickLength = StickInput.Size();
        if (StickLength < StickAimThreshold)
        {
                return;
        }
        FVector NewAimDirection = FVector(StickInput.Y, StickInput.X, 0.0f);
        NewAimDirection.Normalize();
        LastStickAimDirection = NewAimDirection;
        bAimWithStick = true;
}
FVector AUnshackledPlayerController::GetAimDirection(AUnshackledCharacterBase* MyCharacter)
{
        FVector FallbackDirection = MyCharacter->GetFacingDirection();
        if (bAimWithStick)
        {
                return LastStickAimDirection;
        }
        FVector MouseWorldOrigin;
        FVector MouseWorldDirection;
        bool bGotMouse = DeprojectMousePositionToWorld(MouseWorldOrigin, MouseWorldDirection);
        if (!bGotMouse)
        {
                return FallbackDirection;
        }
        FVector LineEnd = MouseWorldOrigin + (MouseWorldDirection * 100000.0f);
        FVector PlanePoint = MyCharacter->GetActorLocation();
        FVector PlaneNormal = FVector(0.0f, 0.0f, 1.0f);
        FVector CursorPoint = FMath::LinePlaneIntersection(MouseWorldOrigin, LineEnd, PlanePoint, PlaneNormal);
        FVector AimDirection = CursorPoint - MyCharacter->GetActorLocation();
        AimDirection.Z = 0.0f;
        if (AimDirection.IsNearlyZero())
        {
                return FallbackDirection;
        }
        return AimDirection;
}
void AUnshackledPlayerController::Attack()
{
        AUnshackledCharacterBase* MyCharacter = Cast<AUnshackledCharacterBase>(GetPawn());
        if (MyCharacter == nullptr)
        {
                return;
        }
        FVector AttackDirection = GetAimDirection(MyCharacter);
        MyCharacter->AttackInDirection(AttackDirection);
}
void AUnshackledPlayerController::Dash()
{
        AUnshackledCharacterBase* MyCharacter = Cast<AUnshackledCharacterBase>(GetPawn());
        if (MyCharacter != nullptr)
        {
                MyCharacter->Dash();
        }
}
void AUnshackledPlayerController::Special()
{
        AUnshackledCharacterBase* MyCharacter = Cast<AUnshackledCharacterBase>(GetPawn());
        if (MyCharacter == nullptr)
        {
                return;
        }
        FVector AimDirection = GetAimDirection(MyCharacter);
        MyCharacter->UseSpecial(AimDirection);
}
void AUnshackledPlayerController::Ultimate()
{
        AUnshackledCharacterBase* MyCharacter = Cast<AUnshackledCharacterBase>(GetPawn());
        if (MyCharacter == nullptr)
        {
                return;
        }
        FVector AimDirection = GetAimDirection(MyCharacter);
        MyCharacter->UseUltimate(AimDirection);
}
bool AUnshackledPlayerController::IsSwitchMenuOpen() const
{
        return bSwitchMenuOpen;
}
int32 AUnshackledPlayerController::GetPartyCount() const
{
        return PartyMembers.Num();
}
int32 AUnshackledPlayerController::GetSwitchMenuIndex() const
{
        return SwitchMenuIndex;
}
APartyMemberCharacter* AUnshackledPlayerController::GetPartyMember(int32 Index) const
{
        if (Index < 0)
        {
                return nullptr;
        }
        if (Index >= PartyMembers.Num())
        {
                return nullptr;
        }
        return PartyMembers[Index];
}
void AUnshackledPlayerController::BuildPartyList()
{
        if (PartyMembers.Num() > 0)
        {
                return;
        }
        for (TActorIterator<APartyMemberCharacter> It(GetWorld()); It; ++It)
        {
                APartyMemberCharacter* Member = *It;
                if (Member != nullptr)
                {
                        PartyMembers.Add(Member);
                }
        }
}
void AUnshackledPlayerController::OpenSwitchMenu()
{
        if (bSwitchMenuOpen)
        {
                return;
        }
        BuildPartyList();
        APartyMemberCharacter* CurrentLeader = Cast<APartyMemberCharacter>(GetPawn());
        if (CurrentLeader == nullptr)
        {
                return;
        }
        int32 LeaderIndex = PartyMembers.Find(CurrentLeader);
        if (LeaderIndex == INDEX_NONE)
        {
                return;
        }
        SwitchMenuIndex = LeaderIndex;
        bSwitchMenuOpen = true;
}
void AUnshackledPlayerController::CloseSwitchMenu()
{
        if (!bSwitchMenuOpen)
        {
                return;
        }
        bSwitchMenuOpen = false;
        APartyMemberCharacter* Selected = GetPartyMember(SwitchMenuIndex);
        APartyMemberCharacter* CurrentLeader = Cast<APartyMemberCharacter>(GetPawn());
        if (Selected == nullptr)
        {
                return;
        }
        if (Selected == CurrentLeader)
        {
                return;
        }
        if (Selected->Health->IsDead())
        {
                return;
        }
        SwitchTo(Selected);
}
void AUnshackledPlayerController::SwitchNext()
{
        MoveSwitchSelection(1);
}
void AUnshackledPlayerController::SwitchPrevious()
{
        MoveSwitchSelection(-1);
}
void AUnshackledPlayerController::MoveSwitchSelection(int32 Step)
{
        if (!bSwitchMenuOpen)
        {
                return;
        }
        int32 PartySize = PartyMembers.Num();
        if (PartySize == 0)
        {
                return;
        }
        int32 CandidateIndex = SwitchMenuIndex;
        for (int32 Attempt = 0; Attempt < PartySize; Attempt++)
        {
                CandidateIndex = CandidateIndex + Step;
                if (CandidateIndex >= PartySize)
                {
                        CandidateIndex = 0;
                }
                if (CandidateIndex < 0)
                {
                        CandidateIndex = PartySize - 1;
                }
                APartyMemberCharacter* Candidate = PartyMembers[CandidateIndex];
                if (Candidate != nullptr)
                {
                        if (!Candidate->Health->IsDead())
                        {
                                SwitchMenuIndex = CandidateIndex;
                                return;
                        }
                }
        }
}
bool AUnshackledPlayerController::SwitchToFirstLivingAlly()
{
        BuildPartyList();
        APartyMemberCharacter* CurrentLeader = Cast<APartyMemberCharacter>(GetPawn());
        for (int32 i = 0; i < PartyMembers.Num(); i++)
        {
                APartyMemberCharacter* Candidate = PartyMembers[i];
                if (Candidate != nullptr)
                {
                        if (Candidate != CurrentLeader)
                        {
                                if (!Candidate->Health->IsDead())
                                {
                                        bSwitchMenuOpen = false;
                                        SwitchTo(Candidate);
                                        return true;
                                }
                        }
                }
        }
        return false;
}
void AUnshackledPlayerController::SwitchTo(APartyMemberCharacter* NewLeader)
{
        APartyMemberCharacter* OldLeader = Cast<APartyMemberCharacter>(GetPawn());
        AController* FollowerController = NewLeader->GetController();
        NewLeader->SetFollowing(false);
        Possess(NewLeader);
        if (FollowerController != nullptr)
        {
                FollowerController->Destroy();
        }
        if (OldLeader != nullptr)
        {
                OldLeader->SetFollowing(true);
                OldLeader->SpawnDefaultController();
                OldLeader->OnSwappedOut(NewLeader);
        }
        NewLeader->OnSwappedIn(OldLeader);
}
void AUnshackledPlayerController::SetupInputComponent()
{
        Super::SetupInputComponent();
        UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
        if (Input != nullptr && MoveAction != nullptr)
        {
                Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUnshackledPlayerController::Move);
                if (AttackAction != nullptr)
                {
                        Input->BindAction(AttackAction, ETriggerEvent::Started, this, &AUnshackledPlayerController::Attack);
                }
                if (DashAction != nullptr)
                {
                        Input->BindAction(DashAction, ETriggerEvent::Started, this, &AUnshackledPlayerController::Dash);
                }
                if (SpecialAction != nullptr)
                {
                        Input->BindAction(SpecialAction, ETriggerEvent::Started, this, &AUnshackledPlayerController::Special);
                }
                if (UltimateAction != nullptr)
                {
                        Input->BindAction(UltimateAction, ETriggerEvent::Started, this, &AUnshackledPlayerController::Ultimate);
                }
                if (AimAction != nullptr)
                {
                        Input->BindAction(AimAction, ETriggerEvent::Triggered, this, &AUnshackledPlayerController::Aim);
                }
                if (SwitchMenuAction != nullptr)
                {
                        Input->BindAction(SwitchMenuAction, ETriggerEvent::Started, this, &AUnshackledPlayerController::OpenSwitchMenu);
                        Input->BindAction(SwitchMenuAction, ETriggerEvent::Completed, this, &AUnshackledPlayerController::CloseSwitchMenu);
                }
                if (SwitchNextAction != nullptr)
                {
                        Input->BindAction(SwitchNextAction, ETriggerEvent::Started, this, &AUnshackledPlayerController::SwitchNext);
                }
                if (SwitchPreviousAction != nullptr)
                {
                        Input->BindAction(SwitchPreviousAction, ETriggerEvent::Started, this, &AUnshackledPlayerController::SwitchPrevious);
                }
        }
}
void AUnshackledPlayerController::Move(const FInputActionValue& Value)
{
        FVector2D Direction = Value.Get<FVector2D>();
        AUnshackledCharacterBase* MyCharacter = Cast<AUnshackledCharacterBase>(GetPawn());
        if (MyCharacter != nullptr)
        {
                MyCharacter->MoveCharacter(Direction);
        }
}