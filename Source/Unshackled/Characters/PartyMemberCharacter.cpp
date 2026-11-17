#include "PartyMemberCharacter.h"
#include "Player/UnshackledPlayerController.h"
#include "Game/UnshackledGameInstance.h"
#include "UI/UnshackledHUD.h"
#include "Battle/TurnBattleManager.h"
#include "Battle/BattleMoveRow.h"
#include "Characters/BossCharacter.h"
#include "Items/ItemRows.h"
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
        RefreshArtifactEffects();
        if (!IsPlayerControlled())
        {
                SetFollowing(true);
        }
}
void APartyMemberCharacter::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        UpdateGroundCooldown(DeltaTime);
        TickItemShield(DeltaTime);
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
        TActorIterator<ATurnBattleManager> It(GetWorld());
        if (It)
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
        if (Move->TargetKind == EBattleTargetKind::AllEnemies)
        {
                HitAllBattleEnemies(Manager, Move->Power, false);
                return;
        }
        if (Move->TargetKind == EBattleTargetKind::Everyone)
        {
                HitAllBattleEnemies(Manager, Move->Power, false);
                return;
        }
        HitBattleTarget(Manager, TargetIndex, Move->Power, false);
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
void APartyMemberCharacter::OnBattleEnemyTurn(ATurnBattleManager* Manager, int32 EnemyIndex)
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
        if (Move->TargetKind == EBattleTargetKind::TwoEnemies)
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
        TArray<AUnshackledCharacterBase*> BoostedEnemies;
        bool bBoosted = false;
        if (NextAttackBonus > 0.0f)
        {
                if (MoveTargetsEnemy(Move))
                {
                        BoostedEnemies = Manager->GetMoveEnemyTargets(Move, TargetIndex);
                        for (int32 i = 0; i < BoostedEnemies.Num(); i++)
                        {
                                AddBattleModifier(BoostedEnemies[i], FName("BattlesongBoost"), ECharacterStat::IncomingDamage, EModifierKind::Percent, NextAttackBonus, 0);
                        }
                        if (BoostedEnemies.Num() > 0)
                        {
                                bBoosted = true;
                        }
                }
        }
        PerformBattleMove(Manager, MoveId, Move, TargetIndex);
        if (bBoosted)
        {
                for (int32 i = 0; i < BoostedEnemies.Num(); i++)
                {
                        BoostedEnemies[i]->Stats->RemoveModifiersFromSource(FName("BattlesongBoost"));
                }
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
        if (Result > 0.0f)
        {
                if (Attacker != nullptr)
                {
                        if (Attacker->bIsEnemy)
                        {
                                if (RollEnemyAttackFails())
                                {
                                        return 0.0f;
                                }
                        }
                }
        }
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
float APartyMemberCharacter::HitBattleEnemy(AUnshackledCharacterBase* Target, float Power, bool bTrueDamage)
{
        if (Target == nullptr)
        {
                return 0.0f;
        }
        if (Target->Health->IsDead())
        {
                return 0.0f;
        }
        if (Power <= 0.0f)
        {
                return 0.0f;
        }
        float DamageStat = Stats->GetFinal(ECharacterStat::Damage);
        float RawDamage = DamageStat * Power;
        return Target->ApplyHit(RawDamage, bTrueDamage, this);
}
float APartyMemberCharacter::HitBattleTarget(ATurnBattleManager* Manager, int32 TargetIndex, float Power, bool bTrueDamage)
{
        if (Manager == nullptr)
        {
                return 0.0f;
        }
        AUnshackledCharacterBase* Target = Manager->GetTargetEnemy(TargetIndex);
        return HitBattleEnemy(Target, Power, bTrueDamage);
}
float APartyMemberCharacter::HitAllBattleEnemies(ATurnBattleManager* Manager, float Power, bool bTrueDamage)
{
        if (Manager == nullptr)
        {
                return 0.0f;
        }
        TArray<AUnshackledCharacterBase*> Targets = Manager->GetLivingEnemies();
        float TotalDealt = 0.0f;
        for (int32 i = 0; i < Targets.Num(); i++)
        {
                TotalDealt = TotalDealt + HitBattleEnemy(Targets[i], Power, bTrueDamage);
        }
        return TotalDealt;
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
        TActorIterator<ATurnBattleManager> It(GetWorld());
        if (It)
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
        UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
        if (GameInstanceRef != nullptr)
        {
                GameInstanceRef->RestoreFloorStartRecruits();
                GameInstanceRef->RestoreFloorStartInventory();
        }
        FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
        UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}
float APartyMemberCharacter::GetFloorStatScale() const
{
        UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
        if (GameInstanceRef == nullptr)
        {
                return 1.0f;
        }
        return GameInstanceRef->GetPartyStatScale();
}
float APartyMemberCharacter::GetFloorManaCostScale() const
{
        UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
        if (GameInstanceRef == nullptr)
        {
                return 1.0f;
        }
        return GameInstanceRef->GetPartyManaCostScale();
}
float APartyMemberCharacter::GetManaRegenScale() const
{
        UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
        if (GameInstanceRef == nullptr)
        {
                return 1.0f;
        }
        return GameInstanceRef->GetOverworldManaRegenScale();
}
float APartyMemberCharacter::GetCooldownScale() const
{
        UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
        if (GameInstanceRef == nullptr)
        {
                return 1.0f;
        }
        return 1.0f - GameInstanceRef->GetCooldownReduction();
}
bool APartyMemberCharacter::RollEnemyAttackFails()
{
        UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
        if (GameInstanceRef == nullptr)
        {
                return false;
        }
        bool bInBattle = IsInTurnBattle();
        float FailChance = GameInstanceRef->GetEnemyFailChance(bInBattle);
        if (FailChance <= 0.0f)
        {
                return false;
        }
        if (FMath::FRand() >= FailChance)
        {
                return false;
        }
        FString Text = FString::Printf(TEXT("Omen: an attack on %s fails"), *DisplayName);
        if (bInBattle)
        {
                ShowBattleMessage(Text, FColor::Purple);
        }
        else
        {
                AUnshackledHUD* Hud = AUnshackledHUD::FindPlayerHUD(this);
                if (Hud != nullptr)
                {
                        Hud->AddMessage(Text, FLinearColor(0.7f, 0.5f, 1.0f, 1.0f), 2.0f);
                }
        }
        return true;
}
bool APartyMemberCharacter::CanReceiveItem(const FActiveItemRow* Item) const
{
        if (Item == nullptr)
        {
                return false;
        }
        if (Item->ReviveFraction > 0.0f)
        {
                return Health->IsDead();
        }
        return !Health->IsDead();
}
bool APartyMemberCharacter::ReceiveItem(const FActiveItemRow* Item, bool bInBattle)
{
        if (!CanReceiveItem(Item))
        {
                return false;
        }
        if (Item->ReviveFraction > 0.0f)
        {
                ReviveCharacterAt(Item->ReviveFraction);
        }
        float HealAmount = Item->HealFlat + (Item->HealPercent * Health->MaxHealth);
        if (HealAmount > 0.0f)
        {
                Health->Heal(HealAmount);
        }
        if (Item->ManaFlat > 0.0f)
        {
                Stats->RestoreMana(Item->ManaFlat);
        }
        AddItemModifier(FName("ItemDamage"), ECharacterStat::Damage, Item->DamagePercent, Item, bInBattle);
        AddItemModifier(FName("ItemDefense"), ECharacterStat::Defense, Item->DefensePercent, Item, bInBattle);
        if (!bInBattle)
        {
                AddItemModifier(FName("ItemSpeed"), ECharacterStat::Speed, Item->SpeedPercent, Item, bInBattle);
        }
        if (Item->ShieldPercent > 0.0f)
        {
                float ShieldAmount = Item->ShieldPercent * Health->MaxHealth;
                Health->AddShield(ShieldAmount);
                ItemShieldAmount = ShieldAmount;
                if (bInBattle)
                {
                        ItemShieldRoundsLeft = Item->BuffRounds;
                        ItemShieldSecondsLeft = 0.0f;
                }
                else
                {
                        ItemShieldSecondsLeft = Item->BuffSeconds;
                        ItemShieldRoundsLeft = 0;
                }
        }
        return true;
}
void APartyMemberCharacter::AddItemModifier(FName SourceId, ECharacterStat Stat, float Amount, const FActiveItemRow* Item, bool bInBattle)
{
        if (Amount == 0.0f)
        {
                return;
        }
        FStatModifier NewModifier;
        NewModifier.SourceId = SourceId;
        NewModifier.Stat = Stat;
        NewModifier.Kind = EModifierKind::Percent;
        NewModifier.Amount = Amount;
        NewModifier.Rule = EStackRule::RefreshSameSource;
        if (bInBattle)
        {
                NewModifier.Duration = 0.0f;
                NewModifier.TurnsLeft = Item->BuffRounds;
        }
        else
        {
                NewModifier.Duration = Item->BuffSeconds;
                NewModifier.TurnsLeft = 0;
        }
        Stats->AddModifier(NewModifier);
}
void APartyMemberCharacter::RefreshArtifactEffects()
{
        Stats->RemoveModifiersFromSource(FName("ArtifactBonus"));
        Health->ArtifactBonus = 0.0f;
        UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
        if (GameInstanceRef == nullptr)
        {
                return;
        }
        float DamageFlat = 0.0f;
        float DamagePercent = 0.0f;
        float DefenseFlat = 0.0f;
        float DefensePercent = 0.0f;
        float SpeedFlat = 0.0f;
        float SpeedPercent = 0.0f;
        float ReflectFraction = 0.0f;
        TArray<FName> EquippedIds = GameInstanceRef->GetEquippedArtifactIds();
        for (int32 i = 0; i < EquippedIds.Num(); i++)
        {
                const FArtifactRow* Row = GameInstanceRef->GetArtifactRow(EquippedIds[i]);
                if (Row != nullptr)
                {
                        DamageFlat = DamageFlat + Row->DamageFlat;
                        DamagePercent = DamagePercent + Row->DamagePercent;
                        DefenseFlat = DefenseFlat + Row->DefenseFlat;
                        DefensePercent = DefensePercent + Row->DefensePercent;
                        SpeedFlat = SpeedFlat + Row->SpeedFlat;
                        SpeedPercent = SpeedPercent + Row->SpeedPercent;
                        ReflectFraction = ReflectFraction + Row->Reflect;
                }
        }
        AddArtifactModifier(ECharacterStat::Damage, EModifierKind::Flat, DamageFlat);
        AddArtifactModifier(ECharacterStat::Damage, EModifierKind::Percent, DamagePercent);
        AddArtifactModifier(ECharacterStat::Defense, EModifierKind::Flat, DefenseFlat);
        AddArtifactModifier(ECharacterStat::Defense, EModifierKind::Percent, DefensePercent);
        AddArtifactModifier(ECharacterStat::Speed, EModifierKind::Flat, SpeedFlat);
        AddArtifactModifier(ECharacterStat::Speed, EModifierKind::Percent, SpeedPercent);
        AddArtifactModifier(ECharacterStat::Reflect, EModifierKind::Flat, ReflectFraction);
        Health->ArtifactBonus = GameInstanceRef->GetHealShieldBonus();
}
void APartyMemberCharacter::AddArtifactModifier(ECharacterStat Stat, EModifierKind Kind, float Amount)
{
        if (Amount == 0.0f)
        {
                return;
        }
        FStatModifier NewModifier;
        NewModifier.SourceId = FName("ArtifactBonus");
        NewModifier.Stat = Stat;
        NewModifier.Kind = Kind;
        NewModifier.Amount = Amount;
        NewModifier.Duration = 0.0f;
        NewModifier.TurnsLeft = 0;
        NewModifier.Rule = EStackRule::RefreshSameSource;
        Stats->AddModifier(NewModifier);
}
void APartyMemberCharacter::AddCappedShield(float Amount, float CapAmount)
{
        if (Health->IsDead())
        {
                return;
        }
        float Room = CapAmount - Health->Shield;
        if (Room <= 0.0f)
        {
                return;
        }
        float ShieldToAdd = Amount;
        if (ShieldToAdd > Room)
        {
                ShieldToAdd = Room;
        }
        Health->AddShield(ShieldToAdd);
}
void APartyMemberCharacter::TickItemShield(float DeltaTime)
{
        if (ItemShieldSecondsLeft <= 0.0f)
        {
                return;
        }
        ItemShieldSecondsLeft = ItemShieldSecondsLeft - DeltaTime;
        if (ItemShieldSecondsLeft <= 0.0f)
        {
                ItemShieldSecondsLeft = 0.0f;
                ExpireItemShield();
        }
}
void APartyMemberCharacter::CountDownItemShieldRound()
{
        if (ItemShieldRoundsLeft <= 0)
        {
                return;
        }
        ItemShieldRoundsLeft = ItemShieldRoundsLeft - 1;
        if (ItemShieldRoundsLeft <= 0)
        {
                ExpireItemShield();
        }
}
void APartyMemberCharacter::ExpireItemShield()
{
        float ShieldToRemove = ItemShieldAmount;
        if (ShieldToRemove > Health->Shield)
        {
                ShieldToRemove = Health->Shield;
        }
        Health->Shield = Health->Shield - ShieldToRemove;
        ItemShieldAmount = 0.0f;
}
