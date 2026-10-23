#include "KaliaCharacter.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/HealthComponent.h"
#include "Components/StatsComponent.h"
#include "Stats/StatTypes.h"
AKaliaCharacter::AKaliaCharacter()
{
        StatsRowName = FName("Kalia");
        DisplayName = TEXT("Kalia");
        AuraColor = FLinearColor(1.0f, 0.1f, 0.1f);
        AttackDamage = 28.0f;
        AttackCooldown = 0.45f;
        SpecialWindup = 1.2f;
        Health->MaxHealth = 80.0f;
        Stats->BaseDefense = 10.0f;
}
void AKaliaCharacter::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        if (!IsGrounded())
        {
                ResetGroundedState();
        }
        TickUltimate(DeltaTime);
}
void AKaliaCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
        if (bUltimateActive)
        {
                bUltimateActive = false;
                RestoreTimeDilation();
        }
        Super::EndPlay(EndPlayReason);
}
void AKaliaCharacter::PerformBasicAttack(FVector Direction)
{
        Direction.Z = 0.0f;
        Direction.Normalize();
        FVector SlashCenter = GetActorLocation() + (Direction * DaggerRange);
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        FCollisionShape SlashShape = FCollisionShape::MakeSphere(DaggerRadius);
        GetWorld()->OverlapMultiByObjectType(OverlapResults, SlashCenter, FQuat::Identity, ObjectParams, SlashShape, QueryParams);
        TArray<AActor*> ActorsAlreadyHit;
        for (int32 i = 0; i < OverlapResults.Num(); i++)
        {
                AActor* HitActor = OverlapResults[i].GetActor();
                if (HitActor != nullptr)
                {
                        if (!ActorsAlreadyHit.Contains(HitActor))
                        {
                                ActorsAlreadyHit.Add(HitActor);
                                AUnshackledCharacterBase* OtherCharacter = Cast<AUnshackledCharacterBase>(HitActor);
                                if (OtherCharacter != nullptr)
                                {
                                        if (OtherCharacter->bIsEnemy != bIsEnemy)
                                        {
                                                DealDamage(OtherCharacter, DaggerDamageScale, false);
                                        }
                                }
                        }
                }
        }
        DrawDebugSphere(GetWorld(), SlashCenter, DaggerRadius, 12, FColor::Red, false, 0.15f);
}
void AKaliaCharacter::PerformSpecial(FVector AimDirection)
{
        if (bUltimateActive)
        {
                BlinkToRandomTarget();
                return;
        }
        FVector Direction = AimDirection;
        Direction.Z = 0.0f;
        Direction.Normalize();
        FVector Origin = GetActorLocation();
        AUnshackledCharacterBase* Target = FindKnifeTarget(Direction);
        if (Target == nullptr)
        {
                DrawDebugLine(GetWorld(), Origin, Origin + (Direction * KnifeRange), FColor::Silver, false, 0.4f, 0, 3.0f);
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Silver, TEXT("Knife missed"));
                return;
        }
        DrawDebugLine(GetWorld(), Origin, Target->GetActorLocation(), FColor::Red, false, 0.4f, 0, 3.0f);
        DealDamage(Target, KnifeDamageScale, false);
        ApplyKnifeDebuff(Target);
        RefreshSpecialIfLow(Target);
        BlinkBehind(Target, Direction);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("Knife hit!"));
}
AUnshackledCharacterBase* AKaliaCharacter::FindKnifeTarget(FVector Direction)
{
        AUnshackledCharacterBase* BestTarget = nullptr;
        float BestDistanceAlong = KnifeRange;
        FVector Origin = GetActorLocation();
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (Candidate->bIsEnemy != bIsEnemy)
                {
                        if (!Candidate->Health->IsDead())
                        {
                                FVector ToCandidate = Candidate->GetActorLocation() - Origin;
                                ToCandidate.Z = 0.0f;
                                float DistanceAlong = FVector::DotProduct(ToCandidate, Direction);
                                if (DistanceAlong > 0.0f)
                                {
                                        if (DistanceAlong <= BestDistanceAlong)
                                        {
                                                FVector PointOnLine = Direction * DistanceAlong;
                                                float SidewaysDistance = (ToCandidate - PointOnLine).Size();
                                                if (SidewaysDistance <= KnifeWidth)
                                                {
                                                        BestDistanceAlong = DistanceAlong;
                                                        BestTarget = Candidate;
                                                }
                                        }
                                }
                        }
                }
        }
        return BestTarget;
}
void AKaliaCharacter::ApplyKnifeDebuff(AUnshackledCharacterBase* Target)
{
        if (Target->Health->IsDead())
        {
                return;
        }
        FStatModifier DefenseLoss;
        DefenseLoss.SourceId = FName("KaliaKnife");
        DefenseLoss.Stat = ECharacterStat::Defense;
        DefenseLoss.Kind = EModifierKind::Percent;
        DefenseLoss.Amount = 0.0f - KnifeDefenseReduction;
        DefenseLoss.Duration = KnifeDefenseDuration;
        DefenseLoss.Rule = EStackRule::RefreshSameSource;
        Target->Stats->AddModifier(DefenseLoss);
}
void AKaliaCharacter::RefreshSpecialIfLow(AUnshackledCharacterBase* Target)
{
        bool bShouldRefresh = false;
        if (Target->Health->IsDead())
        {
                bShouldRefresh = true;
        }
        else
        {
                float TargetMaxHealth = Target->Health->MaxHealth;
                if (TargetMaxHealth > 0.0f)
                {
                        float HealthFraction = Target->Health->CurrentHealth / TargetMaxHealth;
                        if (HealthFraction < RefreshHealthFraction)
                        {
                                bShouldRefresh = true;
                        }
                }
        }
        if (bShouldRefresh)
        {
                GetWorldTimerManager().ClearTimer(SpecialCooldownTimer);
                bCanUseSpecial = true;
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("Knife refreshed!"));
        }
}
void AKaliaCharacter::BlinkBehind(AUnshackledCharacterBase* Target, FVector Direction)
{
        FVector BehindLocation = Target->GetActorLocation() + (Direction * BlinkBehindDistance);
        BehindLocation.Z = GetActorLocation().Z;
        SetActorLocation(BehindLocation);
}
void AKaliaCharacter::OnKilledTarget(AUnshackledCharacterBase* Target)
{
        Super::OnKilledTarget(Target);
        if (Target == nullptr)
        {
                return;
        }
        if (Target->bIsEnemy == bIsEnemy)
        {
                return;
        }
        FreezeEnemiesAround(Target->GetActorLocation());
}
void AKaliaCharacter::FreezeEnemiesAround(FVector Center)
{
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        FCollisionObjectQueryParams ObjectParams;
        ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
        FCollisionShape ExplosionShape = FCollisionShape::MakeSphere(ExplosionRadius);
        GetWorld()->OverlapMultiByObjectType(OverlapResults, Center, FQuat::Identity, ObjectParams, ExplosionShape, QueryParams);
        TArray<AActor*> ActorsAlreadyHit;
        for (int32 i = 0; i < OverlapResults.Num(); i++)
        {
                AActor* HitActor = OverlapResults[i].GetActor();
                if (HitActor != nullptr)
                {
                        if (!ActorsAlreadyHit.Contains(HitActor))
                        {
                                ActorsAlreadyHit.Add(HitActor);
                                AUnshackledCharacterBase* OtherCharacter = Cast<AUnshackledCharacterBase>(HitActor);
                                if (OtherCharacter != nullptr)
                                {
                                        if (OtherCharacter->bIsEnemy != bIsEnemy)
                                        {
                                                FreezeEnemy(OtherCharacter);
                                        }
                                }
                        }
                }
        }
        DrawDebugSphere(GetWorld(), Center, ExplosionRadius, 16, FColor::Cyan, false, 0.5f);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, TEXT("Kalia freeze explosion!"));
}
void AKaliaCharacter::FreezeEnemy(AUnshackledCharacterBase* Enemy)
{
        if (Enemy->Health->IsDead())
        {
                return;
        }
        FStatModifier FreezeModifier;
        FreezeModifier.SourceId = FName("KaliaFreeze");
        FreezeModifier.Stat = ECharacterStat::Speed;
        FreezeModifier.Kind = EModifierKind::Flat;
        FreezeModifier.Amount = 0.0f;
        FreezeModifier.Duration = FreezeDuration;
        FreezeModifier.Rule = EStackRule::RefreshSameSource;
        FreezeModifier.Flag = EStatusFlag::Freeze;
        Enemy->Stats->AddModifier(FreezeModifier);
}
void AKaliaCharacter::PerformRetribution()
{
        Super::PerformRetribution();
        if (Health->IsDead())
        {
                return;
        }
        AddRetributionModifier(ECharacterStat::Speed, EModifierKind::Percent, RetributionSpeedBonus);
        AddRetributionModifier(ECharacterStat::Defense, EModifierKind::Percent, RetributionDefenseBonus);
        AddRetributionModifier(ECharacterStat::Damage, EModifierKind::Percent, RetributionDamageBonus);
        AddRetributionModifier(ECharacterStat::Lifesteal, EModifierKind::Flat, RetributionLifesteal);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Kalia Retribution"));
}
void AKaliaCharacter::AddRetributionModifier(ECharacterStat Stat, EModifierKind Kind, float Amount)
{
        FStatModifier Modifier;
        Modifier.SourceId = FName("KaliaRetribution");
        Modifier.Stat = Stat;
        Modifier.Kind = Kind;
        Modifier.Amount = Amount;
        Modifier.Duration = RetributionDuration;
        Modifier.Rule = EStackRule::RefreshSameSource;
        Stats->AddModifier(Modifier);
}
void AKaliaCharacter::OnDealtDamage(AUnshackledCharacterBase* Target, float DamageDealt)
{
        Super::OnDealtDamage(Target, DamageDealt);
        if (!bUltimateActive)
        {
                return;
        }
        if (Target == nullptr)
        {
                return;
        }
        if (Target->bIsEnemy == bIsEnemy)
        {
                return;
        }
        if (DamageDealt <= 0.0f)
        {
                return;
        }
        AddEchoDamage(Target, DamageDealt);
}
void AKaliaCharacter::PerformUltimate(FVector AimDirection)
{
        if (bUltimateActive)
        {
                EndUltimate();
                return;
        }
        StartUltimate();
}
void AKaliaCharacter::StartUltimate()
{
        bUltimateActive = true;
        UltimateTimeLeft = UltimateDuration;
        EchoTargets.Empty();
        EchoDamage.Empty();
        EchoHealing.Empty();
        SavedSpecialManaCost = SpecialManaCost;
        SavedSpecialWindup = SpecialWindup;
        SavedUltimateManaCost = UltimateManaCost;
        bSavedCanDash = bCanDash;
        SpecialManaCost = 0.0f;
        SpecialWindup = 0.0f;
        UltimateManaCost = 0.0f;
        Health->HealthFloor = Health->MaxHealth * UltimateHealthFloorFraction;
        UGameplayStatics::SetGlobalTimeDilation(this, UltimateTimeDilation);
        bDilationSynced = false;
        DamageEnemiesOnCast();
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, TEXT("Kalia stops time!"));
}
void AKaliaCharacter::DamageEnemiesOnCast()
{
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (Candidate->bIsEnemy != bIsEnemy)
                {
                        if (!Candidate->Health->IsDead())
                        {
                                float CastDamage = Candidate->Health->MaxHealth * UltimateCastDamageFraction;
                                Candidate->ApplyHit(CastDamage, true, this);
                        }
                }
        }
}
void AKaliaCharacter::EndUltimate()
{
        bUltimateActive = false;
        RestoreTimeDilation();
        SpecialManaCost = SavedSpecialManaCost;
        SpecialWindup = SavedSpecialWindup;
        UltimateManaCost = SavedUltimateManaCost;
        bCanDash = bSavedCanDash;
        Health->HealthFloor = 0.0f;
        bCanUseUltimate = false;
        GetWorldTimerManager().SetTimer(UltimateCooldownTimer, this, &AKaliaCharacter::ResetUltimate, UltimateCooldown, false);
        ApplyEcho();
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, TEXT("Time resumes"));
}
void AKaliaCharacter::RestoreTimeDilation()
{
        UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
        CustomTimeDilation = 1.0f;
}
void AKaliaCharacter::TickUltimate(float DeltaTime)
{
        if (!bUltimateActive)
        {
                return;
        }
        if (!bDilationSynced)
        {
                CustomTimeDilation = 1.0f / UltimateTimeDilation;
                bDilationSynced = true;
                return;
        }
        if (Health->IsDead())
        {
                EndUltimate();
                return;
        }
        if (!IsPlayerControlled())
        {
                EndUltimate();
                return;
        }
        KeepAbilitiesReady();
        DrainUltimateHealth(DeltaTime);
        UltimateTimeLeft = UltimateTimeLeft - DeltaTime;
        if (UltimateTimeLeft <= 0.0f)
        {
                EndUltimate();
        }
}
void AKaliaCharacter::KeepAbilitiesReady()
{
        GetWorldTimerManager().ClearTimer(SpecialCooldownTimer);
        bCanUseSpecial = true;
        GetWorldTimerManager().ClearTimer(UltimateCooldownTimer);
        bCanUseUltimate = true;
        GetWorldTimerManager().ClearTimer(AttackTimer);
        bCanAttack = true;
        bCanDash = false;
}
void AKaliaCharacter::DrainUltimateHealth(float DeltaTime)
{
        float DrainAmount = Health->CurrentHealth * UltimateDrainFraction * DeltaTime;
        Health->TakeHit(DrainAmount);
}
void AKaliaCharacter::BlinkToRandomTarget()
{
        TArray<AUnshackledCharacterBase*> Candidates;
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (Candidate != this)
                {
                        if (!Candidate->Health->IsDead())
                        {
                                if (Candidate->bIsEnemy != bIsEnemy)
                                {
                                        Candidates.Add(Candidate);
                                }
                                else
                                {
                                        APartyMemberCharacter* AllyMember = Cast<APartyMemberCharacter>(Candidate);
                                        if (AllyMember != nullptr)
                                        {
                                                if (AllyMember->IsGrounded())
                                                {
                                                        Candidates.Add(Candidate);
                                                }
                                        }
                                }
                        }
                }
        }
        if (Candidates.Num() == 0)
        {
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Silver, TEXT("No blink targets"));
                return;
        }
        int32 ChosenIndex = FMath::RandRange(0, Candidates.Num() - 1);
        AUnshackledCharacterBase* ChosenTarget = Candidates[ChosenIndex];
        FVector FromTarget = GetActorLocation() - ChosenTarget->GetActorLocation();
        FromTarget.Z = 0.0f;
        if (FromTarget.IsNearlyZero())
        {
                FromTarget = FVector(1.0f, 0.0f, 0.0f);
        }
        FromTarget.Normalize();
        FVector BlinkLocation = ChosenTarget->GetActorLocation() + (FromTarget * UltimateBlinkOffset);
        BlinkLocation.Z = GetActorLocation().Z;
        DrawDebugLine(GetWorld(), GetActorLocation(), BlinkLocation, FColor::Purple, false, 0.4f, 0, 3.0f);
        SetActorLocation(BlinkLocation);
        ApplyBlinkEffects(ChosenTarget);
}
void AKaliaCharacter::ApplyBlinkEffects(AUnshackledCharacterBase* Target)
{
        if (Target->bIsEnemy != bIsEnemy)
        {
                if (UltimateSlashScale > 0.0f)
                {
                        DealDamage(Target, UltimateSlashScale, false);
                }
                if (Target->Health->IsDead())
                {
                        return;
                }
                float MissingHealth = Target->Health->MaxHealth - Target->Health->CurrentHealth;
                float BlinkDamage = MissingHealth * UltimateMissingDamageFraction;
                Target->ApplyHit(BlinkDamage, true, this);
                return;
        }
        float HealthBefore = Target->Health->CurrentHealth;
        float HealAmount = Target->Health->MaxHealth * UltimateHealFraction;
        Target->Health->Heal(HealAmount);
        float HealthGained = Target->Health->CurrentHealth - HealthBefore;
        AddEchoHealing(Target, HealthGained);
}
void AKaliaCharacter::ApplyEcho()
{
        for (int32 i = 0; i < EchoTargets.Num(); i++)
        {
                AUnshackledCharacterBase* Target = EchoTargets[i];
                if (IsValid(Target))
                {
                        for (int32 Repeat = 0; Repeat < EchoRepeats; Repeat++)
                        {
                                if (EchoDamage[i] > 0.0f)
                                {
                                        Target->ApplyHit(EchoDamage[i] * EchoFraction, true, this);
                                }
                                if (EchoHealing[i] > 0.0f)
                                {
                                        Target->Health->Heal(EchoHealing[i] * EchoFraction);
                                }
                        }
                }
        }
        EchoTargets.Empty();
        EchoDamage.Empty();
        EchoHealing.Empty();
}
int32 AKaliaCharacter::FindEchoIndex(AUnshackledCharacterBase* Target)
{
        int32 FoundIndex = EchoTargets.Find(Target);
        if (FoundIndex != INDEX_NONE)
        {
                return FoundIndex;
        }
        EchoTargets.Add(Target);
        EchoDamage.Add(0.0f);
        EchoHealing.Add(0.0f);
        return EchoTargets.Num() - 1;
}
void AKaliaCharacter::AddEchoDamage(AUnshackledCharacterBase* Target, float Amount)
{
        int32 TargetIndex = FindEchoIndex(Target);
        EchoDamage[TargetIndex] = EchoDamage[TargetIndex] + Amount;
}
void AKaliaCharacter::AddEchoHealing(AUnshackledCharacterBase* Target, float Amount)
{
        int32 TargetIndex = FindEchoIndex(Target);
        EchoHealing[TargetIndex] = EchoHealing[TargetIndex] + Amount;
}
void AKaliaCharacter::GroundedTick(float DeltaTime)
{
        Super::GroundedTick(DeltaTime);
        if (!Stats->CanAct())
        {
                return;
        }
        if (!bHasGroundedHome)
        {
                GroundedHome = GetActorLocation();
                bHasGroundedHome = true;
        }
        if (GroundedState == EKaliaGroundedState::Idle)
        {
                TickGroundedIdle(DeltaTime);
        }
        else if (GroundedState == EKaliaGroundedState::Out)
        {
                TickGroundedOut(DeltaTime);
        }
        else
        {
                TickGroundedBack(DeltaTime);
        }
}
void AKaliaCharacter::ResetGroundedState()
{
        GroundedState = EKaliaGroundedState::Idle;
        bHasGroundedHome = false;
        GroundedWaitLeft = 0.0f;
        GroundedTarget = nullptr;
}
void AKaliaCharacter::TickGroundedIdle(float DeltaTime)
{
        GroundedWaitLeft = GroundedWaitLeft - DeltaTime;
        if (GroundedWaitLeft > 0.0f)
        {
                return;
        }
        GroundedWaitLeft = 0.0f;
        AUnshackledCharacterBase* Target = FindGroundedTarget();
        if (Target == nullptr)
        {
                return;
        }
        GroundedTarget = Target;
        GroundedState = EKaliaGroundedState::Out;
}
void AKaliaCharacter::TickGroundedOut(float DeltaTime)
{
        if (GroundedTarget == nullptr)
        {
                GroundedState = EKaliaGroundedState::Back;
                return;
        }
        if (GroundedTarget->Health->IsDead())
        {
                GroundedTarget = nullptr;
                GroundedState = EKaliaGroundedState::Back;
                return;
        }
        bool bArrived = MoveTowardPoint(GroundedTarget->GetActorLocation(), GroundedStopDistance, DeltaTime);
        if (bArrived)
        {
                DealDamage(GroundedTarget, GroundedSlashScale, false);
                DrawDebugSphere(GetWorld(), GroundedTarget->GetActorLocation(), DaggerRadius, 12, FColor::Red, false, 0.15f);
                GroundedTarget = nullptr;
                GroundedState = EKaliaGroundedState::Back;
        }
}
void AKaliaCharacter::TickGroundedBack(float DeltaTime)
{
        bool bArrived = MoveTowardPoint(GroundedHome, 10.0f, DeltaTime);
        if (bArrived)
        {
                GroundedState = EKaliaGroundedState::Idle;
                GroundedWaitLeft = GroundedInterval;
        }
}
AUnshackledCharacterBase* AKaliaCharacter::FindGroundedTarget()
{
        AUnshackledCharacterBase* NearestEnemy = nullptr;
        float NearestDistance = GroundedRadius;
        for (TActorIterator<AUnshackledCharacterBase> It(GetWorld()); It; ++It)
        {
                AUnshackledCharacterBase* Candidate = *It;
                if (Candidate->bIsEnemy != bIsEnemy)
                {
                        if (!Candidate->Health->IsDead())
                        {
                                float Distance = FVector::Dist2D(GroundedHome, Candidate->GetActorLocation());
                                if (Distance <= NearestDistance)
                                {
                                        NearestDistance = Distance;
                                        NearestEnemy = Candidate;
                                }
                        }
                }
        }
        return NearestEnemy;
}
bool AKaliaCharacter::MoveTowardPoint(FVector Destination, float StopDistance, float DeltaTime)
{
        FVector ToDestination = Destination - GetActorLocation();
        ToDestination.Z = 0.0f;
        float Distance = ToDestination.Size();
        if (Distance <= StopDistance)
        {
                return true;
        }
        float StepLength = GroundedDashSpeed * DeltaTime;
        if (StepLength > Distance)
        {
                StepLength = Distance;
        }
        FVector StepDirection = ToDestination / Distance;
        SetActorLocation(GetActorLocation() + (StepDirection * StepLength));
        return false;
}
