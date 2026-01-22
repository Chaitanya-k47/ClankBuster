// Fill out your copyright notice in the Description page of Project Settings.


#include "CBCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "DamageableInterface.h"
#include "DrawDebugHelpers.h"


// Sets default values
ACBCharacter::ACBCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//create a first person camera:
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	check(FirstPersonCameraComponent != nullptr);

	//create first person mesh:
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	check(FirstPersonMeshComponent != nullptr);

	//attach first person mesh to the character's third-person skeletal mesh:
	FirstPersonMeshComponent->SetupAttachment(GetMesh());

	//include the first person mesh in first person rendering:
	FirstPersonMeshComponent->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;

	//only the owning player should see the first person mesh:
	//FirstPersonMeshComponent->SetOnlyOwnerSee(true); migrate to beginplay()

	//Set the first person mesh to not collide with other objects:
	FirstPersonMeshComponent->SetCollisionProfileName(FName("NoCollision"));

	//owning player doesnt see the third-person character mesh but it must cast a shadow:
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	//attach first person camera to head bone of first person mesh:
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, FName("head_Socket"));

	//set first person camera's relative location and rotation:
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FirstPersonCameraOffset, FRotator(0.0f, 90.f, -90.f));
	
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	//enable fp rendering on camera and set default FOV and scale values:
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = FirstPersonFOV;
	FirstPersonCameraComponent->FirstPersonScale = FirstPersonViewScale;
}

// Called when the game starts or when spawned
void ACBCharacter::BeginPlay()
{
	Super::BeginPlay();

	//add input mapping context
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if(PlayerController)
	{
		ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
		if(Subsystem)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	//only the owning player should see the first person mesh:
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);

	//set default animation:
	FirstPersonMeshComponent->SetAnimInstanceClass(FirstPersonDefaultAnim->GeneratedClass);
	GetMesh()->SetAnimInstanceClass(CharacterMeshDefaultAnim->GeneratedClass);

	//hide head bone:
	//GetMesh()->HideBoneByName(TEXT("head"), EPhysBodyOp::PBO_None);	
}

// Called every frame
void ACBCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACBCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if(EnhancedInputComponent)
	{
		//bind Move and Look using callbacks
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACBCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACBCharacter::Look);

		//bind jump using built-in Jump functions
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACBCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACBCharacter::StopJumping);
	
		//bind dash using callback
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ACBCharacter::Dash);

		//bind slide using callbacks
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &ACBCharacter::StartSlide);
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Completed, this, &ACBCharacter::StopSlide);

		//bind fire using callback
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ACBCharacter::FireWeapon);

	}
}

void ACBCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if(Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y); //forward/backward
		AddMovementInput(GetActorRightVector(), MovementVector.X); //right/left
	}
}

void ACBCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if(Controller !=nullptr)
	{
		AddControllerYawInput(LookAxisVector.X); //yaw
		AddControllerPitchInput(-LookAxisVector.Y); //pitch
	}
}

void ACBCharacter::Dash()
{
	if(!bCanDash) return; 

	//get input direction i.e. WASD:
	FVector InputDirection = GetLastMovementInputVector();

	//now calculate dash direction based on input direction:
	//-> if no WASD key pressed(no input dir), dash forward
	//-> else dash in input dir(first normalize input dir)
	FVector DashDirection = (InputDirection.IsNearlyZero()) ? GetActorForwardVector() : InputDirection.GetSafeNormal();

	//upward launch logic:
	//-> if on ground add upward force
	//-> if in air, no upward force
	float UpForce = (GetCharacterMovement()->IsMovingOnGround()) ? DashAirLaunch : 0.f;

	//final dash launch vector:
	FVector DashLaunchVector = (DashImpulse * DashDirection) + FVector(0.f, 0.f, UpForce);

	//use LaunchCharacter() Builtin UE function to Dash:
	LaunchCharacter(
		DashLaunchVector,
		true, //XY override(replace velocity in XY, dont add to it)
		true  //Z override(replace velocity in Z, dont add to it)
	);

	//handle cooldown:
	bCanDash = false;
	GetWorldTimerManager().SetTimer(
		DashTimerHandle,
		this,
		&ACBCharacter::ResetDash,
		DashCooldownTime,
		false
	);
}

void ACBCharacter::ResetDash()
{
	bCanDash = true;
}

void ACBCharacter::StartSlide()
{
	Crouch();

	if(GetCharacterMovement()->IsMovingOnGround())
	{
		FVector SlideDirection = GetVelocity().GetSafeNormal();
		FVector SlideLaunchVector = SlideSpeed * SlideDirection;

		//reduce friction and decelaration
		GetCharacterMovement()->GroundFriction = 0.0f;
		GetCharacterMovement()->BrakingDecelerationWalking = 1024.0f;

		LaunchCharacter(
			SlideLaunchVector,
			false, //XY override (add to current velocity)
			false //Z override
		);
	}
}

void ACBCharacter::StopSlide()
{
	UnCrouch();

	//reset friction and deceleration to default UE values.
	GetCharacterMovement()->GroundFriction = 8.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
}

void ACBCharacter::FireWeapon()
{
	if(!FirstPersonCameraComponent) return;

	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();
	FVector End = Start + (ForwardVector * WeaponRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); //do not shoot yourself

	//line trace
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility, //standard vision channel
		QueryParams
	);

	//debug line
	DrawDebugLine(
		GetWorld(),
		Start,
		(bHit ? HitResult.Location : End),
		FColor::Red,
		false,
		2.0f,
		0, 
		1.0f
	);

	//interface check
	if(bHit && HitResult.GetActor())
	{
		//check if the actor that got hit implements the DamageableInterface.
		if(HitResult.GetActor()->GetClass()->ImplementsInterface(UDamageableInterface::StaticClass()))
		{
			IDamageableInterface* DamageableActor = Cast<IDamageableInterface>(HitResult.GetActor());
			if(DamageableActor)
			{
				DamageableActor->ReactToHit(WeaponDamage, ForwardVector * ShotImpulse);
			}
		}
	}

	//apply recoil to camera:
	AddControllerPitchInput(-RecoilForce);
}

