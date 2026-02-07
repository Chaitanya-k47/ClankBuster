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
#include "CBWeapon.h"
#include "DrawDebugHelpers.h"


// Sets default values
ACBCharacter::ACBCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->SetTickGroup(ETickingGroup::TG_PostUpdateWork);

	//create a first person camera:
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	check(FirstPersonCameraComponent != nullptr);
	FirstPersonCameraComponent->SetupAttachment(GetMesh(), FName("headSocket"));
	//FirstPersonCameraComponent->SetRelativeLocationAndRotation(FirstPersonCameraOffset, FRotator(0.0f, 90.f, -90.f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	//enable fp rendering on camera and set default FOV and scale values:
	// FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	// FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	// FirstPersonCameraComponent->FirstPersonFieldOfView = FirstPersonFOV;
	// FirstPersonCameraComponent->FirstPersonScale = FirstPersonViewScale;
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

	//weapon spawning:
	for(const TSubclassOf<ACBWeapon>& WeaponClass : DefaultWeaponClasses)
	{
		if(!WeaponClass) continue;

		FActorSpawnParameters Params;
		Params.Owner = this;

		ACBWeapon* SpawnedWeapon = GetWorld()->SpawnActor<ACBWeapon>(WeaponClass, Params);
		
		//add spawned weapon to owned weapons array
		int32 Index = Weapons.Add(SpawnedWeapon);
		if(Index == CurrentIndex)
		{
			EquipWeapon(SpawnedWeapon);
		}
	}
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

		//bind switch weapon using callback
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Started, this, &ACBCharacter::SwitchWeapon);

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

void ACBCharacter::EquipWeapon(ACBWeapon* NewWeapon)
{
	if(!NewWeapon) return;

	ACBWeapon* OldWeapon = nullptr;

	//unequip current weapon
	if(CurrentWeapon)
	{
		CurrentWeapon->Mesh->SetVisibility(false);
		CurrentWeapon->SetActorEnableCollision(false);
		CurrentWeapon->bIsEquipped = false;
		OldWeapon = CurrentWeapon;
	}

	//equip new weapon
	CurrentWeapon = NewWeapon;

	const FTransform& PlacementTransform = CurrentWeapon->PlacementTransform * GetMesh()->GetSocketTransform(FName("HandGrip_R"));
	CurrentWeapon->SetActorTransform(PlacementTransform, false, nullptr, ETeleportType::TeleportPhysics);
	CurrentWeapon->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::KeepWorldTransform,
		FName("HandGrip_R")
	);

	CurrentWeapon->Mesh->SetVisibility(true);
	CurrentWeapon->SetActorEnableCollision(true);
	CurrentWeapon->bIsEquipped = true;

	//broadcast weapon change event using delegate
	OnCurrentWeaponChanged.Broadcast(CurrentWeapon, OldWeapon);
}

void ACBCharacter::SwitchWeapon(const FInputActionValue& Value)
{
	if(Weapons.Num() <= 1) return;
	
	const float ScrollValue = Value.Get<float>();
	if(FMath::IsNearlyZero(ScrollValue)) return;

	const int32 Direction = (ScrollValue > 0.f) ? 1 : -1;
	CurrentIndex = (CurrentIndex + Direction + Weapons.Num()) % Weapons.Num();

	EquipWeapon(Weapons[CurrentIndex]);
}
