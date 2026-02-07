// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CBWeapon.h"
#include "CBAnimInstance.generated.h"

class ACBCharacter;
class USkeletalMeshComponent;
class ACBWeapon;

/**
 * 
 */
UCLASS()
class CLANKBUSTER_API UCBAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UCBAnimInstance();

protected:
	virtual void NativeBeginPlay() override; //beginplay override for animation instance
	virtual void NativeUpdateAnimation (float DeltaSeconds) override; //tick override for animation instance

	UFUNCTION()
	virtual void CurrentWeaponChanged(ACBWeapon* NewWeapon, const ACBWeapon* OldWeapon);
	
	virtual void SetVariables(const float DeltaSeconds);
	virtual void CalculateWeaponSway(const float DeltaSeconds);

	virtual void SetIKTransforms();

public:

	//REFERENCES:
	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	ACBCharacter* Character;

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	USkeletalMeshComponent* CharacterMesh;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	ACBWeapon* CurrentWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FIKProperties IKProperties;


	//IK VARIABLES:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FTransform CameraTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FTransform RelativeCameraTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FTransform RHandToSightTransform;

};