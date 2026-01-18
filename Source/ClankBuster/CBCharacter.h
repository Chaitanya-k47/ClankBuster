// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CBCharacter.generated.h"

class UAnimBlueprint;
class UInputAction;
class UInputMappingContext;
class UCameraComponent;


UCLASS()
class CLANKBUSTER_API ACBCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACBCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Input config:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* DashAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SlideAction;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//---------FIRST PERSON CAMERA---------//
	//First person Camera component:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	FVector FirstPersonCameraOffset = FVector(2.8f, 5.9f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float FirstPersonFOV = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float FirstPersonViewScale = 0.6f;

	//first person mesh visible only to the owning player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USkeletalMeshComponent* FirstPersonMeshComponent;
	//-------------------------------------//


	//---------ANIMATIONS---------//
	//First person default/idle animation:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|idle")
	UAnimBlueprint* FirstPersonDefaultAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|idle")
	UAnimBlueprint* CharacterMeshDefaultAnim;

	//----------------------------//


protected:

	//Input callbacks:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	//---------DASH---------//
	//Dash config:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Dash")
	float DashImpulse = 2000.f; //force of dashing(the distance covered in a dash)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Dash")
	float DashAirLaunch = 200.f; //little upward force when dashing on ground

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Dash")
	float DashCooldownTime = 2.f;

	bool bCanDash = true;
	FTimerHandle DashTimerHandle; //timer handle for dash cooldown

	//Dash callbacks:
	void Dash();
	void ResetDash();
	//----------------------//


	//---------SLIDE---------//
	//Slide config:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement|Slide")
	float SlideSpeed = 1500.f;
	
	//Slide callbacks:
	void StartSlide();
	void StopSlide();
	//-----------------------//

};
