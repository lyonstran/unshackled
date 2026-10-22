#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RetributionComponent.generated.h"
class APartyMemberCharacter;
class AUnshackledPlayerController;
UCLASS(ClassGroup = (Unshackled), meta = (BlueprintSpawnableComponent))
class UNSHACKLED_API URetributionComponent : public UActorComponent
{
        GENERATED_BODY()
public:
        URetributionComponent();
        virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction* ThisTickFunction) override;
        void OnLeaderSwitched(APartyMemberCharacter* NewLeader);
        int32 GetSequenceLength() const;
        APartyMemberCharacter* GetSequenceMember(int32 Index) const;
        int32 GetProgress() const;
        float GetCooldownRemaining() const;
        UPROPERTY(EditAnywhere, Category = "Retribution")
        float FailCooldown = 30.0f;
protected:
        AUnshackledPlayerController* GetPlayerController() const;
        void CountDownCooldown(float DeltaTime);
        int32 CountLivingMembers() const;
        bool SequenceIsStale() const;
        void RollSequence();
        void FailSequence();
        void CompleteSequence(APartyMemberCharacter* Finisher);
        UPROPERTY()
        TArray<APartyMemberCharacter*> Sequence;
        int32 Progress = 0;
        float CooldownRemaining = 0.0f;
};
