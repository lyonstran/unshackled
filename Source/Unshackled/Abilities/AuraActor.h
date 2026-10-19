#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "AuraActor.generated.h"
class APartyMemberCharacter;
UCLASS()
class UNSHACKLED_API AAuraActor : public AActor
{
        GENERATED_BODY()
public:
        AAuraActor();
        virtual void Tick(float DeltaTime) override;
        void ToggleAura();
        void AddPlate();
        void RemovePlate();
        void SetLightOn(bool bLightOn);
        UPROPERTY(VisibleAnywhere, Category = "Aura")
        USceneComponent* Root;
        UPROPERTY(VisibleAnywhere, Category = "Aura")
        UStaticMeshComponent* CoreMesh;
        UPROPERTY(VisibleAnywhere, Category = "Aura")
        UPointLightComponent* Light;
        UPROPERTY(EditAnywhere, Category = "Aura")
        FVector StartLocation = FVector::ZeroVector;
        UPROPERTY(EditAnywhere, Category = "Aura")
        float HeightOffset = 50.0f;
        UPROPERTY(EditAnywhere, Category = "Aura")
        float SpinSpeed = 90.0f;
        UPROPERTY(EditAnywhere, Category = "Aura")
        float FastSpinSpeed = 360.0f;
        UPROPERTY(EditAnywhere, Category = "Aura")
        float PlateSize = 2.0f;
        UPROPERTY(EditAnywhere, Category = "Aura")
        float PlateThickness = 0.05f;
        UPROPERTY(EditAnywhere, Category = "Aura")
        float PlateDepth = -70.0f;
        UPROPERTY(EditAnywhere, Category = "Aura")
        float CoreScale = 0.3f;
        UPROPERTY(EditAnywhere, Category = "Aura")
        float LightIntensity = 8000.0f;
        UPROPERTY(EditAnywhere, Category = "Aura")
        float LightRadius = 500.0f;
protected:
        virtual void BeginPlay() override;
        void CheckToggleKey();
        void FollowLeader();
        void SpinFromInput(float DeltaTime);
        void ApplyColorIfLeaderChanged();
        void ApplyLeaderColor();
        void ApplyColorToMesh(UStaticMeshComponent* Mesh, FLinearColor Color);
        APartyMemberCharacter* GetLeader() const;
        bool bAuraOn = false;
        UPROPERTY()
        APartyMemberCharacter* LastLeader = nullptr;
        UPROPERTY()
        UStaticMesh* SphereMesh = nullptr;
        UPROPERTY()
        UStaticMesh* CubeMesh = nullptr;
        UPROPERTY()
        UStaticMeshComponent* Plate = nullptr;
};
