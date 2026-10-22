#include "RetributionComponent.h"
#include "Player/UnshackledPlayerController.h"
#include "Characters/PartyMemberCharacter.h"
#include "Components/HealthComponent.h"
#include "Engine/Engine.h"
URetributionComponent::URetributionComponent()
{
        PrimaryComponentTick.bCanEverTick = true;
}
void URetributionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
        Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
        AUnshackledPlayerController* MyController = GetPlayerController();
        if (MyController == nullptr)
        {
                return;
        }
        MyController->BuildPartyList();
        if (CooldownRemaining > 0.0f)
        {
                CountDownCooldown(DeltaTime);
                return;
        }
        if (CountLivingMembers() < 2)
        {
                Sequence.Empty();
                Progress = 0;
                return;
        }
        if (SequenceIsStale())
        {
                RollSequence();
        }
}
AUnshackledPlayerController* URetributionComponent::GetPlayerController() const
{
        return Cast<AUnshackledPlayerController>(GetOwner());
}
void URetributionComponent::CountDownCooldown(float DeltaTime)
{
        CooldownRemaining = CooldownRemaining - DeltaTime;
        if (CooldownRemaining < 0.0f)
        {
                CooldownRemaining = 0.0f;
        }
}
int32 URetributionComponent::CountLivingMembers() const
{
        AUnshackledPlayerController* MyController = GetPlayerController();
        if (MyController == nullptr)
        {
                return 0;
        }
        int32 LivingCount = 0;
        for (int32 i = 0; i < MyController->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Member = MyController->GetPartyMember(i);
                if (Member != nullptr)
                {
                        if (!Member->Health->IsDead())
                        {
                                LivingCount = LivingCount + 1;
                        }
                }
        }
        return LivingCount;
}
bool URetributionComponent::SequenceIsStale() const
{
        if (Sequence.Num() == 0)
        {
                return true;
        }
        if (Sequence.Num() != CountLivingMembers())
        {
                return true;
        }
        for (int32 i = 0; i < Sequence.Num(); i++)
        {
                if (Sequence[i] == nullptr)
                {
                        return true;
                }
                if (Sequence[i]->Health->IsDead())
                {
                        return true;
                }
        }
        return false;
}
void URetributionComponent::RollSequence()
{
        Sequence.Empty();
        Progress = 0;
        AUnshackledPlayerController* MyController = GetPlayerController();
        if (MyController == nullptr)
        {
                return;
        }
        TArray<APartyMemberCharacter*> Living;
        for (int32 i = 0; i < MyController->GetPartyCount(); i++)
        {
                APartyMemberCharacter* Member = MyController->GetPartyMember(i);
                if (Member != nullptr)
                {
                        if (!Member->Health->IsDead())
                        {
                                Living.Add(Member);
                        }
                }
        }
        if (Living.Num() < 2)
        {
                return;
        }
        for (int32 i = 0; i < Living.Num() - 1; i++)
        {
                int32 SwapIndex = FMath::RandRange(i, Living.Num() - 1);
                Living.Swap(i, SwapIndex);
        }
        APartyMemberCharacter* CurrentLeader = Cast<APartyMemberCharacter>(MyController->GetPawn());
        if (Living[0] == CurrentLeader)
        {
                Living.Swap(0, Living.Num() - 1);
        }
        Sequence = Living;
}
void URetributionComponent::OnLeaderSwitched(APartyMemberCharacter* NewLeader)
{
        if (NewLeader == nullptr)
        {
                return;
        }
        if (CooldownRemaining > 0.0f)
        {
                return;
        }
        if (SequenceIsStale())
        {
                RollSequence();
                return;
        }
        APartyMemberCharacter* ExpectedMember = Sequence[Progress];
        if (NewLeader != ExpectedMember)
        {
                FailSequence();
                return;
        }
        Progress = Progress + 1;
        if (Progress >= Sequence.Num())
        {
                CompleteSequence(NewLeader);
        }
}
void URetributionComponent::FailSequence()
{
        Sequence.Empty();
        Progress = 0;
        CooldownRemaining = FailCooldown;
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Retribution failed"));
}
void URetributionComponent::CompleteSequence(APartyMemberCharacter* Finisher)
{
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Retribution!"));
        Finisher->PerformRetribution();
        RollSequence();
}
int32 URetributionComponent::GetSequenceLength() const
{
        return Sequence.Num();
}
APartyMemberCharacter* URetributionComponent::GetSequenceMember(int32 Index) const
{
        if (Index < 0)
        {
                return nullptr;
        }
        if (Index >= Sequence.Num())
        {
                return nullptr;
        }
        return Sequence[Index];
}
int32 URetributionComponent::GetProgress() const
{
        return Progress;
}
float URetributionComponent::GetCooldownRemaining() const
{
        return CooldownRemaining;
}
