// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BhopCharacter.generated.h"

UCLASS()
class SANDBOX_API ABhopCharacter : public ACharacter
{
	GENERATED_BODY()



//////////////////////////////////////////////////////////////////////////
// Base functions and components										//
//////////////////////////////////////////////////////////////////////////
public:
	ABhopCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	//virtual void Destroyed() override; // This is a replicated function, handle logic pertaining to character death in here for free

	// overriding this to properly replicate simulated proxies movement: https://www.udemy.com/course/unreal-engine-5-cpp-multiplayer-shooter/learn/lecture/31515548#questions
	//virtual void OnRep_ReplicatedMovement() override;

protected:
	virtual void BeginPlay() override;
	

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class UCameraComponent* CharacterCam;

	
//////////////////////////////////////////////////////////////////////////
// Input functions														//
//////////////////////////////////////////////////////////////////////////
protected:
	// Movement
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void Lookup(float Value);
	virtual void Jump() override;
	void StartSprint();
	void StopSprint();
	void CrouchButtonPressed();

	// Actions
	void EquipButtonPress();
	void UnEquipButtonPress();


private:
	void MoveTheCharacter(float Value, bool isForward);

	//// bhop vars ////
	// the base bhop values
	UPROPERTY(VisibleAnywhere, Category = "Configuration")
		float DefaultMaxWalkSpeed = 800.f; // CharacterMovement->MaxWalkSpeed
	UPROPERTY(VisibleAnywhere, Category = "Configuration")
		float DefaultBraking = 200.f; // CharacterMovement->BrakingDecelerationWalking
	UPROPERTY(VisibleAnywhere, Category = "Configuration")
		float DefaultFriction = 8.f; // CharacterMovement->GroundFriction
	UPROPERTY(VisibleAnywhere, Category = "Configuration")
		float DefaultJumpVelocity = 1000.f; // CharacterMovement->JumpZVelocity


	// the function values
	UPROPERTY(VisibleAnywhere, Category = "Configuration") // Save the previous velocity amount for calculating the acceleration from the bhop functions
		FVector PrevVelocity = FVector::Zero();
	UPROPERTY(EditAnywhere, Category = "Configuration") // takes the magnitude of the XY velocity vector (equal to speed in X/Y plane)
		float XYspeedometer = 0.f;
	UPROPERTY(EditAnywhere, Category = "Configuration") // This is the deltatime saved from the tick component
		float FrameTime = 0.f;


	// general
	UPROPERTY()
		bool bIsSprinting = false;



//////////////////////////////////////////////////////////////////////////
// Animations and Montages												//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Getters and Setters													//
//////////////////////////////////////////////////////////////////////////
public:


};
