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

	//initialize stats
	ResetEnemy();
	
}

void ACBEnemy::ResetEnemy()
{
	CurrentHealth = MaxHealth;
	bIsDead = false;
	
	//reset physics and collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(false);
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
	
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void ACBEnemy::ReactToHit(float DamageAmount, const FVector& HitImpulse)
{
	if(bIsDead) return;
	CurrentHealth -=DamageAmount;
	LastHitImpulse = HitImpulse;
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, FString::Printf(TEXT("Ouch! Health: %f"), CurrentHealth));
	
	OnHit(DamageAmount); //triggers bp logic first then c++ default

	if(CurrentHealth <= 0.f)
	{
		Die();
	}
}

void ACBEnemy::Die()
{
	if(bIsDead) return;
	bIsDead = true;
	
	OnDeath(); //triggers bp logic first then c++ default

	DetachFromControllerPendingDestroy();
	SetLifeSpan(3.0f);
}

void ACBEnemy::OnHit_Implementation(float DamageAmount)
{
	
}

void ACBEnemy::OnDeath_Implementation()
{
	//disable capsule collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//enable ragdoll
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);

	//add shot impulse
	GetMesh()->AddImpulse(LastHitImpulse, NAME_None, true);
}
