#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Characters/PartyMemberCharacter.h"
#include "Components/RetributionComponent.h"
#include "UnshackledPlayerController.generated.h"
enum class EPartyMenuMode : uint8
{
        Switch,
        Ground,
        Recall
};
UCLASS()
class UNSHACKLED_API AUnshackledPlayerController : public APlayerController
{
        GENERATED_BODY()
public:
        AUnshackledPlayerController();
        UPROPERTY(VisibleAnywhere, Category = "Retribution")
        URetributionComponent* Retribution;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputMappingContext *MappingContext;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *MoveAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *AttackAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *DashAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *SpecialAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *UltimateAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *AimAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *SwitchMenuAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *GroundMenuAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *RecallMenuAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *SwitchNextAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        UInputAction *SwitchPreviousAction;
        UPROPERTY(EditAnywhere, Category = "Input")
        float StickAimThreshold = 0.25f;
        bool IsSwitchMenuOpen() const;
        int32 GetPartyCount() const;
        int32 GetSwitchMenuIndex() const;
        APartyMemberCharacter* GetPartyMember(int32 Index) const;
        FString GetMenuTitle() const;
        bool CanChooseMember(APartyMemberCharacter* Member) const;
        bool SwitchToFirstLivingAlly();
        void BuildPartyList();
        void SwitchTo(APartyMemberCharacter* NewLeader);
        FVector GetAimDirection(class AUnshackledCharacterBase* MyCharacter);
protected:
        virtual void BeginPlay() override;
        virtual void SetupInputComponent() override;
        virtual void PlayerTick(float DeltaTime) override;
        void Move(const FInputActionValue& Value);
        void Aim(const FInputActionValue& Value);
        void Attack();
        void Dash();
        void Special();
        void Ultimate();
        void OpenSwitchMenu();
        void CloseSwitchMenu();
        void OpenGroundMenu();
        void CloseGroundMenu();
        void OpenRecallMenu();
        void CloseRecallMenu();
        void OpenMenu(EPartyMenuMode Mode);
        void CloseMenu(EPartyMenuMode Mode);
        void SwitchNext();
        void SwitchPrevious();
        void MoveSwitchSelection(int32 Step);
        bool bAimWithStick = false;
        FVector LastStickAimDirection = FVector(0.0f, 1.0f, 0.0f);
        bool bSwitchMenuOpen = false;
        EPartyMenuMode SwitchMenuMode = EPartyMenuMode::Switch;
        int32 SwitchMenuIndex = 0;
        UPROPERTY()
        TArray<APartyMemberCharacter*> PartyMembers;
};
