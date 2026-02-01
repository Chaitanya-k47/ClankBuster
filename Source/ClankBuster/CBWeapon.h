// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CBWeapon.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class UAnimSequence;
class ACBCharacter;

//struct for Inverse Kinematics properties
USTRUCT(BlueprintType)
struct FIKProperties
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimSequence* BasePose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AimOffset = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform CustomOffsetTransform; //offset from base pose.
};


UCLASS(Abstract)
class CLANKBUSTER_API ACBWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACBWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "State")
	ACBCharacter* CurrentOwner;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "State")
	bool bIsEquipped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
	FIKProperties IKProperties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configurations")
	FTransform PlacementTransform;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "IK")
	FTransform GetSightsWorldTransform() const; //blueprint overridable
	virtual FTransform GetSightsWorldTransform_Implementation() const; //C++ default implementation

private:
	

};
