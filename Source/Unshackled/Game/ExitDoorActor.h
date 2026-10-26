#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ExitDoorActor.generated.h"
class AWaveSpawner;
UCLASS()
class UNSHACKLED_API AExitDoorActor : public AActor
{
	GENERATED_BODY()
public:
	AExitDoorActor();
	virtual void Tick(float DeltaTime) override;
	void UnlockDoor();
	void TryUseDoor();
	void AddPortalGlow();
	void RemovePortalGlow();
	bool CanLeave() const;
	UPROPERTY(VisibleAnywhere, Category = "Exit Door")
	USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, Category = "Exit Door")
	UStaticMeshComponent* DoorSlab;
	UPROPERTY(VisibleAnywhere, Category = "Exit Door")
	UPointLightComponent* Light;
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	UMaterialInterface* ColorMaterial = nullptr;
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	FVector SlabScale = FVector(0.5f, 3.0f, 0.2f);
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	float InteractRadius = 250.0f;
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	FLinearColor LockedColor = FLinearColor(1.0f, 0.1f, 0.1f);
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	FLinearColor UnlockedColor = FLinearColor(0.1f, 1.0f, 0.3f);
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	float LockedLightIntensity = 2000.0f;
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	float UnlockedLightIntensity = 9000.0f;
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	float LightRadius = 600.0f;
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	float PulseSpeed = 3.0f;
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	float PulseAmount = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	float GlowSpinSpeed = 90.0f;
	UPROPERTY(EditAnywhere, Category = "Exit Door")
	float GlowScale = 0.8f;
protected:
	virtual void BeginPlay() override;
	void FindSpawner();
	void ApplyLockedLook();
	void PulseLight(float DeltaTime);
	bool IsPlayerNearby() const;
	void ShowPrompt() const;
	void TravelToNextFloor();
	void ApplyColorToMesh(UStaticMeshComponent* Mesh, FLinearColor Color);
	bool bUnlocked = false;
	float PulseTime = 0.0f;
	UPROPERTY()
	AWaveSpawner* Spawner = nullptr;
	UPROPERTY()
	UStaticMeshComponent* PortalGlow = nullptr;
	UPROPERTY()
	UStaticMesh* SphereMesh = nullptr;
	UPROPERTY()
	UStaticMesh* CubeMesh = nullptr;
};
