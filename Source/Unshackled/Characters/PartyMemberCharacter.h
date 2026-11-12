#pragma once
#include "CoreMinimal.h"
#include "UnshackledCharacterBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/Texture2D.h"
#include "PartyMemberCharacter.generated.h"
class ATurnBattleManager;
struct FBattleMoveRow;
UCLASS()
class UNSHACKLED_API APartyMemberCharacter : public AUnshackledCharacterBase
{
        GENERATED_BODY()
public:
        APartyMemberCharacter();
        virtual void Tick(float DeltaTime) override;
        void SetFollowing(bool bShouldFollow);
        bool IsFollowing() const;
        bool IsGrounded() const;
        bool IsInTurnBattle() const;
        bool CanBeGrounded() const;
        float GetGroundCooldownRemaining() const;
        void GroundAlly();
        void RecallAlly();
        virtual void OnSwappedIn(APartyMemberCharacter* PreviousLeader);
        virtual void OnSwappedOut(APartyMemberCharacter* NewLeader);
        virtual void PerformRetribution();
        virtual void PerformBattleMove(ATurnBattleManager* Manager, FName MoveId, const FBattleMoveRow* Move, int32 TargetIndex);
        virtual void OnBattleRoundEnd(ATurnBattleManager* Manager);
        virtual void OnBattlePlayerPhaseStart(ATurnBattleManager* Manager);
        virtual void OnBattlePlayerPhaseEnd(ATurnBattleManager* Manager);
        virtual bool CanUseBattleMove(FName MoveId) const;
        virtual FString GetBattleCardNote() const;
        void SetBattleShelter(APartyMemberCharacter* Guardian, float Share);
        void ClearBattleShelter();
        APartyMemberCharacter* GetShelterGuardian() const;
        UPROPERTY(VisibleAnywhere, Category = "Camera")
        USpringArmComponent* CameraArm;
        UPROPERTY(VisibleAnywhere, Category = "Camera")
        UCameraComponent* Camera;
        UPROPERTY(EditAnywhere, Category = "Game Over")
        float GameOverDelay = 2.0f;
        UPROPERTY(EditAnywhere, Category = "Party")
        float FollowDistance = 150.0f;
        UPROPERTY(EditAnywhere, Category = "Party")
        FString DisplayName = TEXT("Ally");
        UPROPERTY(EditAnywhere, Category = "Party")
        UTexture2D* Portrait = nullptr;
        UPROPERTY(EditAnywhere, Category = "Party")
        FLinearColor AuraColor = FLinearColor(0.0f, 0.8f, 0.8f, 1.0f);
        UPROPERTY(EditAnywhere, Category = "Grounding")
        float GroundCooldown = 30.0f;
        UPROPERTY(EditAnywhere, Category = "Grounding")
        float GroundManaDrainPerSecond = 8.0f;
protected:
        virtual void BeginPlay() override;
        virtual void Die() override;
        virtual void GroundedTick(float DeltaTime);
        virtual float FilterIncomingDamage(float RawDamage, bool bTrueDamage, AUnshackledCharacterBase* Attacker) override;
        float HitBattleBoss(ATurnBattleManager* Manager, float Power, bool bTrueDamage);
        void AddBattleModifier(AUnshackledCharacterBase* Target, FName SourceId, ECharacterStat Stat, EModifierKind Kind, float Amount, int32 Turns);
        ATurnBattleManager* FindBattleManager() const;
        void ShowBattleMessage(FString Text, FColor Color);
        void FollowLeader();
        void UpdateGroundCooldown(float DeltaTime);
        void DrainGroundedMana(float DeltaTime);
        bool bIsFollowing = false;
        bool bIsGrounded = false;
        float GroundCooldownRemaining = 0.0f;
        UPROPERTY()
        APartyMemberCharacter* ShelterGuardian = nullptr;
        float ShelterShare = 0.0f;
        FTimerHandle GameOverTimer;
        UFUNCTION()
        void RestartLevelAfterDeath();
};
