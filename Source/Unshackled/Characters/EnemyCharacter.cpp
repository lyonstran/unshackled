#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsEnemy = true;
	GetCharacterMovement()->MaxWalkSpeed = 320.0f;
	AutoPossessAI = EAutoPossessAI::Disabled;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Health->IsDead())
	{
		return;
	}
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn==nullptr)
	{
		return;
	}
	AUnshackledCharacterBase* PlayerCharacter = Cast<AUnshackledCharacterBase>(PlayerPawn);
	if (PlayerCharacter != nullptr)
	{
		if (PlayerCharacter->Health->IsDead())
		{
			return;
		}
	}
	FVector VectorToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
	VectorToPlayer.Z = 0.0f;
	float DistanceToPlayer = VectorToPlayer.Size();
	FVector DirectionToPlayer = VectorToPlayer;
	DirectionToPlayer.Normalize();
	if (DistanceToPlayer > AttackTriggerDistance)
	{
		FVector2D MoveInput = FVector2D(DirectionToPlayer.Y, DirectionToPlayer.X);
		MoveCharacter(MoveInput);
	} 
	else
	{
		FacingDirection = DirectionToPlayer;
		BasicAttack();
	}
}

void AEnemyCharacter::Die()
{
	Super::Die();
	SetLifeSpan(1.0f);
}
