#include "Game/ItemChestActor.h"
#include "Game/UnshackledGameInstance.h"
#include "UI/UnshackledHUD.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
AItemChestActor::AItemChestActor()
{
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	Visual = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Root);
	Visual->SetRelativeRotation(FRotator(0.0f, 90.0f, 90.0f));
}
bool AItemChestActor::IsOpened() const
{
	return bOpened;
}
void AItemChestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DrawGrayBox();
	if (bOpened)
	{
		return;
	}
	if (IsLeaderNearby())
	{
		Open();
	}
}
bool AItemChestActor::IsLeaderNearby() const
{
	APawn* LeaderPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (LeaderPawn == nullptr)
	{
		return false;
	}
	float Distance = FVector::Dist2D(GetActorLocation(), LeaderPawn->GetActorLocation());
	return Distance <= OpenRadius;
}
void AItemChestActor::Open()
{
	bOpened = true;
	if (OpenedFlipbook != nullptr)
	{
		Visual->SetFlipbook(OpenedFlipbook);
	}
	UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
	if (GameInstanceRef == nullptr)
	{
		return;
	}
	FName Found = GameInstanceRef->AwardArtifact();
	ShowFoundMessage(Found);
}
void AItemChestActor::ShowFoundMessage(FName ArtifactId)
{
	AUnshackledHUD* Hud = AUnshackledHUD::FindPlayerHUD(this);
	if (Hud == nullptr)
	{
		return;
	}
	if (ArtifactId.IsNone())
	{
		Hud->ShowBanner(TEXT("The chest is empty"), FLinearColor(0.7f, 0.7f, 0.7f, 1.0f), BannerSeconds);
		return;
	}
	UUnshackledGameInstance* GameInstanceRef = Cast<UUnshackledGameInstance>(GetGameInstance());
	FString ArtifactName = GameInstanceRef->GetArtifactName(ArtifactId);
	Hud->ShowBanner(FString::Printf(TEXT("Artifact: %s"), *ArtifactName), FLinearColor(1.0f, 0.85f, 0.2f, 1.0f), BannerSeconds);
	if (GameInstanceRef->IsArtifactEquipped(ArtifactId))
	{
		Hud->AddMessage(FString::Printf(TEXT("%s equipped (R to manage artifacts)"), *ArtifactName), FLinearColor(1.0f, 0.85f, 0.2f, 1.0f), 5.0f);
	}
	else
	{
		Hud->AddMessage(FString::Printf(TEXT("%s added; slots full (R to swap)"), *ArtifactName), FLinearColor(1.0f, 0.85f, 0.2f, 1.0f), 5.0f);
	}
}
void AItemChestActor::DrawGrayBox() const
{
	if (Visual->GetFlipbook() != nullptr)
	{
		return;
	}
	FColor BoxColor = FColor(255, 200, 40);
	if (bOpened)
	{
		BoxColor = FColor(110, 110, 110);
	}
	FVector Extent = FVector(BoxSize, BoxSize, BoxSize);
	DrawDebugSolidBox(GetWorld(), GetActorLocation(), Extent * 0.5f, BoxColor, false, -1.0f, 0);
	DrawDebugBox(GetWorld(), GetActorLocation(), Extent * 0.5f, FColor::Black, false, -1.0f, 0, 2.0f);
}
