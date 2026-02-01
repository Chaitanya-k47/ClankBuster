// Fill out your copyright notice in the Description page of Project Settings.


#include "CBWeapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "CBCharacter.h"
#include "Animation/AnimSequence.h"

// Sets default values
ACBWeapon::ACBWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);

}

// Called when the game starts or when spawned
void ACBWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(!CurrentOwner && !bIsEquipped)
	{
		Mesh->SetVisibility(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

// Called every frame
void ACBWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FTransform ACBWeapon::GetSightsWorldTransform_Implementation() const
{
	return Mesh->GetSocketTransform(FName("Sight"));
}

