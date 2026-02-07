// Fill out your copyright notice in the Description page of Project Settings.


#include "CBAnimInstance.h"
#include "CBCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"


UCBAnimInstance::UCBAnimInstance()
{
    
}

void UCBAnimInstance::NativeBeginPlay()
{
    Super::NativeBeginPlay();

    // Character = Cast<ACBCharacter>(TryGetPawnOwner());
    // if(Character)
    // {
    //     CharacterMesh = Character->GetMesh();
    //     Character->OnCurrentWeaponChanged.AddDynamic(this, &UCBAnimInstance::CurrentWeaponChanged);
    //     CurrentWeaponChanged(Character->CurrentWeapon, nullptr);
    // }

}

void UCBAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if(!Character)
    {
        Character = Cast<ACBCharacter>(TryGetPawnOwner());
        if(Character)
        {
            CharacterMesh = Character->GetMesh();
            Character->OnCurrentWeaponChanged.AddDynamic(this, &UCBAnimInstance::CurrentWeaponChanged);
            CurrentWeaponChanged(Character->CurrentWeapon, nullptr);
        }
        else return;
    }

    SetVariables(DeltaSeconds);
    CalculateWeaponSway(DeltaSeconds);

}

void UCBAnimInstance::CurrentWeaponChanged(ACBWeapon* NewWeapon, const ACBWeapon* OldWeapon)
{
    CurrentWeapon = NewWeapon;
    if(CurrentWeapon)
    {
        IKProperties = CurrentWeapon->IKProperties;
        GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UCBAnimInstance::SetIKTransforms);
    }
}

void UCBAnimInstance::SetVariables(const float DeltaSeconds)
{
    //camera transform in worldspace
    CameraTransform = FTransform(Character->GetBaseAimRotation(), Character->FirstPersonCameraComponent->GetComponentLocation());

    //RootOffset is ik_hand_root transform relative to root in component space. 
    const FTransform& RootOffset = CharacterMesh->GetSocketTransform(FName("root"), ERelativeTransformSpace::RTS_Component).Inverse() *  CharacterMesh->GetSocketTransform(FName("ik_hand_root"));
    
    //camera transform relative to root offset
    //  OR
    //camera relative to hands root
    RelativeCameraTransform = CameraTransform.GetRelativeTransform(RootOffset);

}

void UCBAnimInstance::CalculateWeaponSway(const float DeltaSeconds)
{
    
}

void UCBAnimInstance::SetIKTransforms()
{
    RHandToSightTransform = CurrentWeapon->GetSightsWorldTransform().GetRelativeTransform(CharacterMesh->GetSocketTransform(FName("hand_r")));
}
