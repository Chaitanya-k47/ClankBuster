// Fill out your copyright notice in the Description page of Project Settings.


#include "CBEnemy.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ACBEnemy::ACBEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->bUseControllerDesiredRotation = false;
	MoveComp->RotationRate = FRotator(0.f, 600.f, 0.f);
	

}

// Called when the game starts or when spawned
void ACBEnemy::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
	
}

void ACBEnemy::ReactToHit(float DamageAmount)
{
	if(bIsDead) return;

	CurrentHealth -=DamageAmount;
	OnHit(DamageAmount);

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Ouch! Health: %f"), CurrentHealth));
	if(CurrentHealth <= 0.f)
	{
		Die();
	}
}

void ACBEnemy::Die()
{
	if(bIsDead) return;

	bIsDead = true;
	OnDeath();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DetachFromControllerPendingDestroy();
	SetLifeSpan(0.1f);
}

void ACBEnemy::OnHit(float DamageAmount)
{
	//base hit reaction
}

void ACBEnemy::OnDeath()
{
	//base death behavior
}

