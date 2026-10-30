#include "RecruitPointActor.h"
#include "Characters/PartyMemberCharacter.h"
#include "Player/UnshackledPlayerController.h"
#include "Game/UnshackledGameInstance.h"
#include "UI/UnshackledHUD.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "InputCoreTypes.h"
ARecruitPointActor::ARecruitPointActor()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereFinder.Succeeded())
	{
		SphereMesh = SphereFinder.Object;
	}
	Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
	Marker->SetupAttachment(Root);
	Marker->SetStaticMesh(SphereMesh);
	Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Marker->SetCastShadow(false);
	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(Root);
	Light->SetCastShadows(false);
}
void ARecruitPointActor::BeginPlay()
{
	Super::BeginPlay();
	UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
	if (GameInstanceRef != nullptr)
	{
		if (GameInstanceRef->CurrentFloor < RecruitFloor)
		{
			Destroy();
			return;
		}
		if (GameInstanceRef->IsRecruited(RecruitId))
		{
			SpawnMember(GetLeaderSpawnLocation());
			Destroy();
			return;
		}
	}
	Marker->SetRelativeScale3D(MarkerScale);
	Light->SetAttenuationRadius(LightRadius);
	Light->SetIntensity(LightIntensity);
	ApplyLook();
	APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
	if (Controller != nullptr)
	{
		EnableInput(Controller);
		if (InputComponent != nullptr)
		{
			InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ARecruitPointActor::TryRecruit);
		}
	}
}
void ARecruitPointActor::ApplyLook()
{
	FLinearColor LookColor = FLinearColor(1.0f, 1.0f, 1.0f);
	if (MemberClass != nullptr)
	{
		const APartyMemberCharacter* DefaultMember = GetDefault<APartyMemberCharacter>(MemberClass);
		LookColor = DefaultMember->AuraColor;
	}
	Light->SetLightColor(LookColor);
	ApplyColorToMesh(Marker, LookColor);
}
void ARecruitPointActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsPlayerNearby())
	{
		ShowPrompt();
	}
}
FString ARecruitPointActor::GetMemberName() const
{
	if (MemberClass != nullptr)
	{
		const APartyMemberCharacter* DefaultMember = GetDefault<APartyMemberCharacter>(MemberClass);
		return DefaultMember->DisplayName;
	}
	return RecruitId.ToString();
}
bool ARecruitPointActor::IsPlayerNearby() const
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn == nullptr)
	{
		return false;
	}
	float Distance = FVector::Dist2D(GetActorLocation(), PlayerPawn->GetActorLocation());
	return Distance <= InteractRadius;
}
void ARecruitPointActor::ShowPrompt() const
{
	FString Message = FString::Printf(TEXT("Press E to recruit %s"), *GetMemberName());
	GEngine->AddOnScreenDebugMessage(7003, 0.0f, FColor::Yellow, Message);
}
void ARecruitPointActor::TryRecruit()
{
	if (!IsPlayerNearby())
	{
		return;
	}
	UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
	if (GameInstanceRef == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("Recruit point: set Game Instance Class in Project Settings"));
		return;
	}
	if (MemberClass == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("Recruit point: Member Class is not set"));
		return;
	}
	GameInstanceRef->Recruit(RecruitId);
	SpawnMember(GetActorLocation());
	ShowJoinedBanner();
	Destroy();
}
FVector ARecruitPointActor::GetLeaderSpawnLocation() const
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn == nullptr)
	{
		return GetActorLocation();
	}
	return PlayerPawn->GetActorLocation() + JoinOffset;
}
void ARecruitPointActor::SpawnMember(FVector Location)
{
	if (MemberClass == nullptr)
	{
		return;
	}
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn != nullptr)
	{
		Location.Z = PlayerPawn->GetActorLocation().Z;
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	APartyMemberCharacter* NewMember = World->SpawnActor<APartyMemberCharacter>(MemberClass, Location, FRotator::ZeroRotator, SpawnParams);
	if (NewMember == nullptr)
	{
		return;
	}
	AUnshackledPlayerController* PlayerController = Cast<AUnshackledPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (PlayerController != nullptr)
	{
		PlayerController->AddPartyMember(NewMember);
	}
}
void ARecruitPointActor::ShowJoinedBanner()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr)
	{
		return;
	}
	AUnshackledHUD* Hud = Cast<AUnshackledHUD>(PlayerController->GetHUD());
	if (Hud == nullptr)
	{
		return;
	}
	FString Message = FString::Printf(TEXT("%s joined the party!"), *GetMemberName());
	Hud->ShowBanner(Message, FLinearColor(1.0f, 0.85f, 0.2f, 1.0f), BannerDuration);
}
void ARecruitPointActor::ApplyColorToMesh(UStaticMeshComponent* Mesh, FLinearColor Color)
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
