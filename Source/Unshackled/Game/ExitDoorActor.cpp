#include "ExitDoorActor.h"
#include "Waves/WaveSpawner.h"
#include "Game/UnshackledGameInstance.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "InputCoreTypes.h"
AExitDoorActor::AExitDoorActor()
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
	DoorSlab = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorSlab"));
	DoorSlab->SetupAttachment(Root);
	DoorSlab->SetStaticMesh(CubeMesh);
	DoorSlab->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DoorSlab->SetCastShadow(false);
	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Root);
	Light->SetAttenuationRadius(LightRadius);
	Light->SetCastShadows(false);
}
void AExitDoorActor::BeginPlay()
{
	Super::BeginPlay();
	DoorSlab->SetRelativeScale3D(SlabScale);
	ApplyLockedLook();
	APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
	if (Controller != nullptr)
	{
		EnableInput(Controller);
		if (InputComponent != nullptr)
		{
			InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AExitDoorActor::TryUseDoor);
		}
	}
}
void AExitDoorActor::ApplyLockedLook()
{
	bUnlocked = false;
	Light->SetIntensity(LockedLightIntensity);
	Light->SetLightColor(LockedColor);
	ApplyColorToMesh(DoorSlab, LockedColor);
}
void AExitDoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Spawner == nullptr)
	{
		FindSpawner();
	}
	if (!bUnlocked)
	{
		if (Spawner != nullptr)
		{
			if (Spawner->IsFloorCleared())
			{
				UnlockDoor();
			}
		}
	}
	if (bUnlocked)
	{
		PulseLight(DeltaTime);
		if (PortalGlow != nullptr)
		{
			PortalGlow->AddLocalRotation(FRotator(0.0f, GlowSpinSpeed * DeltaTime, 0.0f));
		}
	}
	if (IsPlayerNearby())
	{
		ShowPrompt();
	}
}
void AExitDoorActor::FindSpawner()
{
	for (TActorIterator<AWaveSpawner> It(GetWorld()); It; ++It)
	{
		Spawner = *It;
		return;
	}
}
void AExitDoorActor::UnlockDoor()
{
	bUnlocked = true;
	Light->SetLightColor(UnlockedColor);
	Light->SetIntensity(UnlockedLightIntensity);
	ApplyColorToMesh(DoorSlab, UnlockedColor);
	AddPortalGlow();
}
void AExitDoorActor::AddPortalGlow()
{
	if (PortalGlow != nullptr)
	{
		return;
	}
	PortalGlow = NewObject<UStaticMeshComponent>(this, TEXT("PortalGlow"));
	PortalGlow->SetStaticMesh(SphereMesh);
	PortalGlow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PortalGlow->SetCastShadow(false);
	PortalGlow->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	PortalGlow->SetRelativeScale3D(FVector(GlowScale, GlowScale, GlowScale));
	PortalGlow->RegisterComponent();
	ApplyColorToMesh(PortalGlow, UnlockedColor);
}
void AExitDoorActor::RemovePortalGlow()
{
	if (PortalGlow == nullptr)
	{
		return;
	}
	PortalGlow->DestroyComponent();
	PortalGlow = nullptr;
}
void AExitDoorActor::PulseLight(float DeltaTime)
{
	PulseTime = PulseTime + DeltaTime;
	float PulseWave = FMath::Sin(PulseTime * PulseSpeed);
	float Brightness = UnlockedLightIntensity * (1.0f + PulseAmount * PulseWave);
	Light->SetIntensity(Brightness);
}
bool AExitDoorActor::CanLeave() const
{
	return bUnlocked;
}
bool AExitDoorActor::IsPlayerNearby() const
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn == nullptr)
	{
		return false;
	}
	float Distance = FVector::Dist2D(GetActorLocation(), PlayerPawn->GetActorLocation());
	return Distance <= InteractRadius;
}
void AExitDoorActor::ShowPrompt() const
{
	if (bUnlocked)
	{
		GEngine->AddOnScreenDebugMessage(7001, 0.0f, FColor::Green, TEXT("Press E to go to the next floor"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(7001, 0.0f, FColor::Red, TEXT("The door is sealed. Clear the floor first."));
	}
}
void AExitDoorActor::TryUseDoor()
{
	if (!CanLeave())
	{
		return;
	}
	if (!IsPlayerNearby())
	{
		return;
	}
	TravelToNextFloor();
}
void AExitDoorActor::TravelToNextFloor()
{
	UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
	if (GameInstanceRef == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("Exit door: set Game Instance Class in Project Settings"));
		return;
	}
	bool bAdvanced = GameInstanceRef->AdvanceFloor();
	if (!bAdvanced)
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Yellow, TEXT("There are no more floors. You escaped!"));
		return;
	}
	RemovePortalGlow();
	FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}
void AExitDoorActor::ApplyColorToMesh(UStaticMeshComponent* Mesh, FLinearColor Color)
{
	UMaterialInstanceDynamic* DynamicMaterial = nullptr;
	if (ColorMaterial != nullptr)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(ColorMaterial, this);
		Mesh->SetMaterial(0, DynamicMaterial);
	}
	else
	{
		DynamicMaterial = Mesh->CreateDynamicMaterialInstance(0);
	}
	if (DynamicMaterial != nullptr)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	}
}
