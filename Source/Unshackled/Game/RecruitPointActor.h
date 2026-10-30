#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "RecruitPointActor.generated.h"
class APartyMemberCharacter;
UCLASS()
class UNSHACKLED_API ARecruitPointActor : public AActor
{
	GENERATED_BODY()
public:
	ARecruitPointActor();
	virtual void Tick(float DeltaTime) override;
	void TryRecruit();
	UPROPERTY(VisibleAnywhere, Category = "Recruit")
	USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, Category = "Recruit")
	UStaticMeshComponent* Marker;
	UPROPERTY(VisibleAnywhere, Category = "Recruit")
	UPointLightComponent* Light;
	UPROPERTY(EditAnywhere, Category = "Recruit")
	FName RecruitId;
	UPROPERTY(EditAnywhere, Category = "Recruit")
	int32 RecruitFloor = 2;
	UPROPERTY(EditAnywhere, Category = "Recruit")
	TSubclassOf<APartyMemberCharacter> MemberClass;
	UPROPERTY(EditAnywhere, Category = "Recruit")
	UMaterialInterface* ColorMaterial = nullptr;
	UPROPERTY(EditAnywhere, Category = "Recruit")
	FVector MarkerScale = FVector(0.8f, 0.8f, 0.8f);
	UPROPERTY(EditAnywhere, Category = "Recruit")
	float InteractRadius = 250.0f;
	UPROPERTY(EditAnywhere, Category = "Recruit")
	FVector JoinOffset = FVector(-100.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, Category = "Recruit")
	float LightIntensity = 5000.0f;
	UPROPERTY(EditAnywhere, Category = "Recruit")
	float LightRadius = 500.0f;
	UPROPERTY(EditAnywhere, Category = "Recruit")
	float BannerDuration = 3.0f;
protected:
	virtual void BeginPlay() override;
	bool IsPlayerNearby() const;
	void ShowPrompt() const;
	void ApplyLook();
	void ApplyColorToMesh(UStaticMeshComponent* Mesh, FLinearColor Color);
	FVector GetLeaderSpawnLocation() const;
	void SpawnMember(FVector Location);
	void ShowJoinedBanner();
	FString GetMemberName() const;
	UPROPERTY()
	UStaticMesh* SphereMesh = nullptr;
};
