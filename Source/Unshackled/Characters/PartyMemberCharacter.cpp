#include "PartyMemberCharacter.h"
#include "Player/UnshackledPlayerController.h"
#include "Game/UnshackledGameInstance.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
APartyMemberCharacter::APartyMemberCharacter()
{
        CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
        CameraArm->SetupAttachment(RootComponent);
        CameraArm->SetUsingAbsoluteRotation(true);
        CameraArm->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
        CameraArm->TargetArmLength = 1000.0f;
        CameraArm->bDoCollisionTest = false;
        Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
        Camera->SetupAttachment(CameraArm);
        Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
        Camera->SetOrthoWidth(1300.0f);
        Stats->bHasMana = true;
        AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}
void APartyMemberCharacter::BeginPlay()
{
        Super::BeginPlay();
        UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
        if (GameInstanceRef != nullptr)
        {
                GameInstanceRef->RestoreCharacterState(this);
        }
        if (!IsPlayerControlled())
        {
                SetFollowing(true);
        }
}
void APartyMemberCharacter::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        UpdateGroundCooldown(DeltaTime);
        if (bIsGrounded)
        {
                DrainGroundedMana(DeltaTime);
        }
        if (bIsGrounded)
        {
                if (!Health->IsDead())
                {
                        GroundedTick(DeltaTime);
                }
        }
        if (!bIsFollowing)
        {
                return;
        }
        if (Health->IsDead())
        {
                return;
        }
        FollowLeader();
}
void APartyMemberCharacter::GroundedTick(float DeltaTime)
{
}
void APartyMemberCharacter::UpdateGroundCooldown(float DeltaTime)
{
        if (GroundCooldownRemaining > 0.0f)
        {
                GroundCooldownRemaining = GroundCooldownRemaining - DeltaTime;
                if (GroundCooldownRemaining < 0.0f)
                {
                        GroundCooldownRemaining = 0.0f;
                }
        }
}
void APartyMemberCharacter::DrainGroundedMana(float DeltaTime)
{
        float ManaToDrain = GroundManaDrainPerSecond * DeltaTime;
        bool bPaid = Stats->SpendMana(ManaToDrain);
        if (!bPaid)
        {
                Stats->CurrentMana = 0.0f;
                RecallAlly();
        }
}
void APartyMemberCharacter::SetFollowing(bool bShouldFollow)
{
        bIsFollowing = bShouldFollow;
        if (bShouldFollow)
        {
                GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        else
        {
                GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
}
bool APartyMemberCharacter::IsFollowing() const
{
        return bIsFollowing;
}
bool APartyMemberCharacter::IsGrounded() const
{
        return bIsGrounded;
}
float APartyMemberCharacter::GetGroundCooldownRemaining() const
{
        return GroundCooldownRemaining;
}
bool APartyMemberCharacter::CanBeGrounded() const
{
        if (Health->IsDead())
        {
                return false;
        }
        if (IsPlayerControlled())
        {
                return false;
        }
        if (bIsGrounded)
        {
                return false;
        }
        if (GroundCooldownRemaining > 0.0f)
        {
                return false;
        }
        if (Stats->CurrentMana <= 0.0f)
        {
                return false;
        }
        return true;
}
void APartyMemberCharacter::GroundAlly()
{
        if (!CanBeGrounded())
        {
                return;
        }
        bIsGrounded = true;
        SetFollowing(false);
}
void APartyMemberCharacter::RecallAlly()
{
        if (!bIsGrounded)
        {
                return;
        }
        bIsGrounded = false;
        GroundCooldownRemaining = GroundCooldown;
        if (!IsPlayerControlled())
        {
                SetFollowing(true);
        }
}
void APartyMemberCharacter::OnSwappedIn(APartyMemberCharacter* PreviousLeader)
{
}
void APartyMemberCharacter::OnSwappedOut(APartyMemberCharacter* NewLeader)
{
}
void APartyMemberCharacter::PerformRetribution()
{
}
void APartyMemberCharacter::FollowLeader()
{
        APawn* LeaderPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        if (LeaderPawn == nullptr)
        {
                return;
        }
        if (LeaderPawn == this)
        {
                return;
        }
        FVector VectorToLeader = LeaderPawn->GetActorLocation() - GetActorLocation();
        VectorToLeader.Z = 0.0f;
        float DistanceToLeader = VectorToLeader.Size();
        if (DistanceToLeader > FollowDistance)
        {
                FVector DirectionToLeader = VectorToLeader;
                DirectionToLeader.Normalize();
                FVector2D MoveInput = FVector2D(DirectionToLeader.Y, DirectionToLeader.X);
                MoveCharacter(MoveInput);
        }
}
void APartyMemberCharacter::Die()
{
        Super::Die();
        if (bIsGrounded)
        {
                bIsGrounded = false;
                SetFollowing(true);
        }
        if (!IsPlayerControlled())
        {
                return;
        }
        AUnshackledPlayerController* MyController = Cast<AUnshackledPlayerController>(GetController());
        if (MyController != nullptr)
        {
                bool bSwitchedToAlly = MyController->SwitchToFirstLivingAlly();
                if (bSwitchedToAlly)
                {
                        return;
                }
        }
        if (GEngine != nullptr)
        {
                GEngine->AddOnScreenDebugMessage(-1, GameOverDelay, FColor::Red, TEXT("Game Over"));
        }
        GetWorldTimerManager().SetTimer(GameOverTimer, this, &APartyMemberCharacter::RestartLevelAfterDeath, GameOverDelay, false);
}
void APartyMemberCharacter::RestartLevelAfterDeath()
{
        FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
        UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}
