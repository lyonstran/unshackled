#include "PartyMemberCharacter.h"
#include "Player/UnshackledPlayerController.h"
#include "Game/UnshackledGameInstance.h"
#include "UI/UnshackledHUD.h"
#include "Battle/TurnBattleManager.h"
#include "Battle/BattleMoveRow.h"
#include "Characters/BossCharacter.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
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
bool APartyMemberCharacter::IsInTurnBattle() const
{
        for (TActorIterator<ATurnBattleManager> It(GetWorld()); It; ++It)
        {
                return true;
        }
        return false;
}
void APartyMemberCharacter::PerformBattleMove(ATurnBattleManager* Manager, FName MoveId, const FBattleMoveRow* Move, int32 TargetIndex)
{
        if (Manager == nullptr)
        {
                return;
        }
        if (Move == nullptr)
        {
                return;
        }
        if (Move->Power <= 0.0f)
        {
                return;
        }
        HitBattleBoss(Manager, Move->Power, false);
}
void APartyMemberCharacter::OnBattleRoundEnd(ATurnBattleManager* Manager)
{
}
void APartyMemberCharacter::OnBattlePlayerPhaseStart(ATurnBattleManager* Manager)
{
}
void APartyMemberCharacter::OnBattlePlayerPhaseEnd(ATurnBattleManager* Manager)
{
}
bool APartyMemberCharacter::CanUseBattleMove(FName MoveId) const
{
        return true;
}
FString APartyMemberCharacter::GetBattleCardNote() const
{
        FString Note;
        if (ShelterGuardian != nullptr)
        {
                Note = Note + TEXT("Sheltered ");
        }
        if (NextAttackBonus > 0.0f)
        {
                Note = Note + TEXT("Song ");
        }
        return Note;
}
void APartyMemberCharacter::OnBattleBossTurn(ATurnBattleManager* Manager)
{
}
void APartyMemberCharacter::OnBattleActionChosen(ATurnBattleManager* Manager, FName EntryName)
{
}
void APartyMemberCharacter::PerformBattleRetribution(ATurnBattleManager* Manager)
{
}
void APartyMemberCharacter::SetNextAttackBonus(float Bonus)
{
        NextAttackBonus = Bonus;
}
bool APartyMemberCharacter::MoveTargetsEnemy(const FBattleMoveRow* Move) const
{
        if (Move == nullptr)
        {
                return false;
        }
        if (Move->TargetKind == EBattleTargetKind::Enemy)
        {
                return true;
        }
        if (Move->TargetKind == EBattleTargetKind::AllEnemies)
        {
                return true;
        }
        if (Move->TargetKind == EBattleTargetKind::Everyone)
        {
                return true;
        }
        return false;
}
void APartyMemberCharacter::UseBattleMove(ATurnBattleManager* Manager, FName MoveId, const FBattleMoveRow* Move, int32 TargetIndex)
{
        if (Manager == nullptr)
        {
                return;
        }
        if (Move == nullptr)
        {
                return;
        }
        ABossCharacter* Boss = Manager->GetBoss();
        bool bBoosted = false;
        if (NextAttackBonus > 0.0f)
        {
                if (MoveTargetsEnemy(Move))
                {
                        if (Boss != nullptr)
                        {
                                AddBattleModifier(Boss, FName("BattlesongBoost"), ECharacterStat::IncomingDamage, EModifierKind::Percent, NextAttackBonus, 0);
                                bBoosted = true;
                        }
                }
        }
        PerformBattleMove(Manager, MoveId, Move, TargetIndex);
        if (bBoosted)
        {
                Boss->Stats->RemoveModifiersFromSource(FName("BattlesongBoost"));
                NextAttackBonus = 0.0f;
                ShowBattleMessage(FString::Printf(TEXT("Battlesong empowers %s's move"), *DisplayName), FColor::Magenta);
        }
}
void APartyMemberCharacter::SetBattleShelter(APartyMemberCharacter* Guardian, float Share)
{
        ShelterGuardian = Guardian;
        ShelterShare = Share;
}
void APartyMemberCharacter::ClearBattleShelter()
{
        ShelterGuardian = nullptr;
        ShelterShare = 0.0f;
}
APartyMemberCharacter* APartyMemberCharacter::GetShelterGuardian() const
{
        return ShelterGuardian;
}
float APartyMemberCharacter::FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker)
{
        float Result = Super::FilterIncomingDamage(RawDamage, bTrueDamage, Attacker);
        if (ShelterGuardian == nullptr)
        {
                return Result;
        }
        if (Result <= 0.0f)
        {
                return Result;
        }
        if (Attacker == nullptr)
        {
                return Result;
        }
        if (!Attacker->bIsEnemy)
        {
                return Result;
        }
        if (ShelterGuardian->Health->IsDead())
        {
                return Result;
        }
        float GuardianPart = Result * ShelterShare;
        ShelterGuardian->ApplyHit(GuardianPart, bTrueDamage, Attacker);
        ShowBattleMessage(FString::Printf(TEXT("%s shares the hit on %s"), *ShelterGuardian->DisplayName, *DisplayName), FColor::Green);
        return Result - GuardianPart;
}
float APartyMemberCharacter::HitBattleBoss(ATurnBattleManager* Manager, float Power, bool bTrueDamage)
{
        if (Manager == nullptr)
        {
                return 0.0f;
        }
        if (Power <= 0.0f)
        {
                return 0.0f;
        }
        ABossCharacter* Boss = Manager->GetBoss();
        if (Boss == nullptr)
        {
                return 0.0f;
        }
        float DamageStat = Stats->GetFinal(ECharacterStat::Damage);
        float RawDamage = DamageStat * Power;
        return Boss->ApplyHit(RawDamage, bTrueDamage, this);
}
void APartyMemberCharacter::AddBattleModifier(AUnshackledCharacterBase* Target, FName SourceId, ECharacterStat Stat, EModifierKind Kind, float Amount, int32 Turns)
{
        if (Target == nullptr)
        {
                return;
        }
        if (Target->Health->IsDead())
        {
                return;
        }
        FStatModifier NewModifier;
        NewModifier.SourceId = SourceId;
        NewModifier.Stat = Stat;
        NewModifier.Kind = Kind;
        NewModifier.Amount = Amount;
        NewModifier.Duration = 0.0f;
        NewModifier.TurnsLeft = Turns;
        NewModifier.Rule = EStackRule::RefreshSameSource;
        Target->Stats->AddModifier(NewModifier);
}
ATurnBattleManager* APartyMemberCharacter::FindBattleManager() const
{
        for (TActorIterator<ATurnBattleManager> It(GetWorld()); It; ++It)
        {
                return *It;
        }
        return nullptr;
}
void APartyMemberCharacter::ShowBattleMessage(FString Text, FColor Color)
{
        ATurnBattleManager* Manager = FindBattleManager();
        if (Manager != nullptr)
        {
                Manager->ShowMessage(Text, Color);
        }
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
        if (MyController == nullptr)
        {
                return;
        }
        if (MyController != nullptr)
        {
                bool bSwitchedToAlly = MyController->SwitchToFirstLivingAlly();
                if (bSwitchedToAlly)
                {
                        return;
                }
        }
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
        if (PlayerController != nullptr)
        {
                AUnshackledHUD* Hud = Cast<AUnshackledHUD>(PlayerController->GetHUD());
                if (Hud != nullptr)
                {
                        Hud->ShowBanner(TEXT("Game Over"), FLinearColor(1.0f, 0.2f, 0.2f, 1.0f), GameOverDelay);
                }
        }
        GetWorldTimerManager().SetTimer(GameOverTimer, this, &APartyMemberCharacter::RestartLevelAfterDeath, GameOverDelay, false);
}
void APartyMemberCharacter::RestartLevelAfterDeath()
{
        FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
        UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}
