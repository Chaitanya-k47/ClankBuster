// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DamageableInterface.h"
#include "CBEnemy.generated.h"

UCLASS()
class CLANKBUSTER_API ACBEnemy : public ACharacter, public IDamageableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACBEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//---- MOVEMENT -----
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float MovementSpeed = 350.f;

	//------ HEALTH --------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	//----- DAMAGE HANDLING ----- 
	//Interface implementation:
	virtual void ReactToHit(float DamageAmount) override;

	//helper to handle death:
	virtual void Die();

	//----- HOOKS FOR VARIANTS -----
	virtual void OnHit(float DamageAmount);
	virtual void OnDeath();

private:
	bool bIsDead = false;

public:


};
