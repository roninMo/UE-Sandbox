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

	UPROPERTY()
		bool bIsSprinting = false;


//////////////////////////////////////////////////////////////////////////
// Bunny Hopping Stuff													//
//////////////////////////////////////////////////////////////////////////
private:
	 /**
	  * Called from CharacterMovementComponent to notify the character that the movement mode has changed.
	  * @param	PrevMovementMode	Movement mode before the change
	  * @param	PrevCustomMode		Custom mode before the change (applicable if PrevMovementMode is Custom)
	  */
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;


	// Reset Friction and Rpcs
	UFUNCTION(Server, Reliable)
		void ServerResetFriction();
	UFUNCTION()
		void ResetFriction();
	UFUNCTION()
		void HandleResetFriction();

	// Remove Friction and Rpcs
	UFUNCTION(Server, Reliable)
		void ServerRemoveFriction();
	UFUNCTION()
		void RemoveFriction();
	UFUNCTION()
		void HandleRemoveFriction();

	// Add Ramp Momentum
	UFUNCTION(Server, Reliable)
		void ServerAddRampMomentum();
	UFUNCTION()
		void AddRampMomentum();
	UFUNCTION()
		void HandleAddRampMomentum();


	// the base bhop values
	#pragma region Base Bhop values
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float DefaultMaxWalkSpeed = 800.f; // CharacterMovement->MaxWalkSpeed
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float DefaultBraking = 200.f; // CharacterMovement->BrakingDecelerationWalking
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float DefaultFriction = 8.f; // CharacterMovement->GroundFriction
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float DefaultJumpVelocity = 1000.f; // CharacterMovement->JumpZVelocity

	UPROPERTY(EditAnywhere, Category = "Bhop_AirAccel")
		bool bEnableCustomAirAccel = true;
	UPROPERTY(EditAnywhere, Category = "Bhop_AirAccel")
		float AirAccelerate = 10.f;

	UPROPERTY(EditAnywhere, Category = "Bhop_Bhop")
		bool bEnablePogo = true;
	UPROPERTY(EditAnywhere, Category = "Bhop_Bhop")
		bool bEnableBunnyHopCap = false;
	UPROPERTY(EditAnywhere, Category = "Bhop_Bhop") // you monster frisbee
		float BunnyHopCapFactor = 2.f;
	UPROPERTY(EditAnywhere, Category = "Bhop_Bhop")
		float BhopBleedFactor = 1.f;
	UPROPERTY(EditAnywhere, Category = "Bhop_Bhop")
		bool bEnableCrouchJump = false;
	UPROPERTY(EditAnywhere, Category = "Bhop_Bhop")
		bool bEnableSpeedometer;

	UPROPERTY(EditAnywhere, Category = "Bhop_Trimping")
		float TrimpDownMultiplier = 1.3f;
	UPROPERTY(EditAnywhere, Category = "Bhop_Trimping")
		float TrimpDownVertCap = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Bhop_Trimping")
		float TrimpUpMultiplier = 1.f;
	UPROPERTY(EditAnywhere, Category = "Bhop_Trimping")
		float TrimpUpLateralSlow = 0.f;

	UPROPERTY(EditAnywhere, Category = "Bhop_RampSliding")
		float RampslideThresholdFactor = 2.5f;
	UPROPERTY(EditAnywhere, Category = "Bhop_RampSliding")
		float RampMomentumFactor = 1.f;

	UPROPERTY(EditAnywhere, Category = "Bhop_GroundAccel")
		bool bEnableGroundAccel = true;
	UPROPERTY(EditAnywhere, Category = "Bhop_GroundAccel")
		float GroundAccelerate = 100.f;

	UPROPERTY(EditAnywhere, Category = "Bhop_Audio")
		FVector WeaponOffset = FVector(50.f, 33.f, 10.f);
	UPROPERTY(EditAnywhere, Category = "Bhop_Audio")
		float BaseTurnRate = 45.f;
	UPROPERTY(EditAnywhere, Category = "Bhop_Audio")
		float BaseLookUpRate = 45.f;
	#pragma endregion


	// the function values
	UPROPERTY(VisibleAnywhere, Category = "Configuration") // Save the previous velocity amount for calculating the acceleration from the bhop functions
		FVector PrevVelocity = FVector::Zero();
	UPROPERTY(EditAnywhere, Category = "Configuration") // takes the magnitude of the XY velocity vector (equal to speed in X/Y plane)
		float XYspeedometer = 0.f;
	UPROPERTY(EditAnywhere, Category = "Configuration") // This is the deltatime saved from the tick component
		float FrameTime = 0.f;

	UPROPERTY() // Our own stored reference of the variable to avoid constant get calls
		TObjectPtr<UCharacterMovementComponent> CachedCharacterMovement;


//////////////////////////////////////////////////////////////////////////
// Animations and Montages												//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Getters and Setters													//
//////////////////////////////////////////////////////////////////////////
public:


};
