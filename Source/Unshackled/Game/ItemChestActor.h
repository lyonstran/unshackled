#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "ItemChestActor.generated.h"
UCLASS()
class UNSHACKLED_API AItemChestActor : public AActor
{
	GENERATED_BODY()
public:
	AItemChestActor();
	virtual void Tick(float DeltaTime) override;
	bool IsOpened() const;
	UPROPERTY(VisibleAnywhere, Category = "Chest")
	USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, Category = "Chest")
	UPaperFlipbookComponent* Visual;
	UPROPERTY(EditAnywhere, Category = "Chest")
	UPaperFlipbook* OpenedFlipbook = nullptr;
	UPROPERTY(EditAnywhere, Category = "Chest")
	float OpenRadius = 120.0f;
	UPROPERTY(EditAnywhere, Category = "Chest")
	float BoxSize = 40.0f;
	UPROPERTY(EditAnywhere, Category = "Chest")
	float BannerSeconds = 4.0f;
	bool bFromMiniboss = false;
private:
	bool bOpened = false;
	bool IsLeaderNearby() const;
	void Open();
	void DrawGrayBox() const;
	void ShowFoundMessage(FName ArtifactId);
};
