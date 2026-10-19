#include "AuraActor.h"
#include "Characters/PartyMemberCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "InputCoreTypes.h"
AAuraActor::AAuraActor()
{
        PrimaryActorTick.bCanEverTick = true;
        Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
        RootComponent = Root;
        ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
        if (SphereFinder.Succeeded())
        {
                SphereMesh = SphereFinder.Object;
        }
        ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
        if (CubeFinder.Succeeded())
        {
                CubeMesh = CubeFinder.Object;
        }
        CoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreMesh"));
        CoreMesh->SetupAttachment(Root);
        CoreMesh->SetStaticMesh(SphereMesh);
        CoreMesh->SetRelativeScale3D(FVector(CoreScale, CoreScale, CoreScale));
        CoreMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CoreMesh->SetCastShadow(false);
        Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
        Light->SetupAttachment(Root);
        Light->SetIntensity(LightIntensity);
        Light->SetAttenuationRadius(LightRadius);
        Light->SetCastShadows(false);
}
void AAuraActor::BeginPlay()
{
        Super::BeginPlay();
        SetActorLocation(StartLocation);
        SetActorRotation(FRotator::ZeroRotator);
        bAuraOn = false;
        SetLightOn(false);
}
void AAuraActor::Tick(float DeltaTime)
{
        Super::Tick(DeltaTime);
        CheckToggleKey();
        FollowLeader();
        SpinFromInput(DeltaTime);
        ApplyColorIfLeaderChanged();
}
void AAuraActor::CheckToggleKey()
{
        APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
        if (Controller == nullptr)
        {
                return;
        }
        if (Controller->WasInputKeyJustPressed(EKeys::Zero))
        {
                ToggleAura();
        }
}
void AAuraActor::ToggleAura()
{
        if (bAuraOn)
        {
                bAuraOn = false;
                SetLightOn(false);
                RemovePlate();
        }
        else
        {
                bAuraOn = true;
                SetLightOn(true);
                AddPlate();
                ApplyLeaderColor();
        }
}
void AAuraActor::SetLightOn(bool bLightOn)
{
        Light->SetVisibility(bLightOn);
        CoreMesh->SetVisibility(bLightOn);
}
void AAuraActor::AddPlate()
{
        if (Plate != nullptr)
        {
                return;
        }
        Plate = NewObject<UStaticMeshComponent>(this);
        Plate->SetStaticMesh(CubeMesh);
        Plate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Plate->SetCastShadow(false);
        Plate->RegisterComponent();
        Plate->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
        Plate->SetRelativeLocation(FVector(0.0f, 0.0f, PlateDepth));
        Plate->SetRelativeScale3D(FVector(PlateSize, PlateSize, PlateThickness));
}
void AAuraActor::RemovePlate()
{
        if (Plate == nullptr)
        {
                return;
        }
        Plate->DestroyComponent();
        Plate = nullptr;
}
APartyMemberCharacter* AAuraActor::GetLeader() const
{
        APawn* LeaderPawn = UGameplayStatics::GetPlayerPawn(this, 0);
        return Cast<APartyMemberCharacter>(LeaderPawn);
}
void AAuraActor::FollowLeader()
{
        APartyMemberCharacter* Leader = GetLeader();
        if (Leader == nullptr)
        {
                return;
        }
        FVector LeaderLocation = Leader->GetActorLocation();
        LeaderLocation.Z = LeaderLocation.Z + HeightOffset;
        SetActorLocation(LeaderLocation);
}
void AAuraActor::SpinFromInput(float DeltaTime)
{
        float CurrentSpeed = SpinSpeed;
        APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
        if (Controller != nullptr)
        {
                if (Controller->IsInputKeyDown(EKeys::Eight))
                {
                        CurrentSpeed = FastSpinSpeed;
                }
        }
        AddActorLocalRotation(FRotator(0.0f, CurrentSpeed * DeltaTime, 0.0f));
}
void AAuraActor::ApplyColorIfLeaderChanged()
{
        APartyMemberCharacter* Leader = GetLeader();
        if (Leader == LastLeader)
        {
                return;
        }
        LastLeader = Leader;
        if (bAuraOn)
        {
                ApplyLeaderColor();
        }
}
void AAuraActor::ApplyLeaderColor()
{
        APartyMemberCharacter* Leader = GetLeader();
        if (Leader == nullptr)
        {
                return;
        }
        FLinearColor Color = Leader->AuraColor;
        Light->SetLightColor(Color);
        ApplyColorToMesh(CoreMesh, Color);
        ApplyColorToMesh(Plate, Color);
}
void AAuraActor::ApplyColorToMesh(UStaticMeshComponent* Mesh, FLinearColor Color)
{
        if (Mesh == nullptr)
        {
                return;
        }
        UMaterialInstanceDynamic* DynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0);
        if (DynamicMaterial != nullptr)
        {
                DynamicMaterial->SetVectorParameterValue(FName("Color"), Color);
        }
}
