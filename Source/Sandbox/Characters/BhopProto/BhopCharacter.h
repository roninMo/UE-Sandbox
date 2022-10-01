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
	void StartJump();
	void StopJump();
	void StartSprint();
	void StopSprint();
	void CrouchButtonPressed();

	// Actions
	void EquipButtonPress();
	void UnEquipButtonPress();


private:
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
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode = 0) override;

	UPROPERTY(EditAnywhere, Category = "Audio")
		class USoundCue* LegBonkSound;
	UPROPERTY(EditAnywhere, Category = "Audio")
		class USoundCue* JumpLandSound;

	#pragma region Reset Friction
	UFUNCTION(Server, Reliable)
		void ServerResetFriction();
	UFUNCTION()
		void ResetFriction();
	UFUNCTION()
		void HandleResetFriction();
	#pragma endregion

	#pragma region Remove Friction
	UFUNCTION(Server, Reliable)
		void ServerRemoveFriction();
	UFUNCTION()
		void RemoveFriction();
	UFUNCTION()
		void HandleRemoveFriction();
	#pragma endregion

	#pragma region Ramp Slide and Checks
	UFUNCTION()
		bool RamSlide();
	UFUNCTION()
		bool RampCheck();
	#pragma endregion

	#pragma region Add Ramp Momentum
	UFUNCTION(Server, Reliable)
		void ServerAddRampMomentum();
	UFUNCTION()
		void AddRampMomentum();
	UFUNCTION()
		void HandleAddRampMomentum();
	#pragma endregion

	#pragma region Ground Acceleration Functions
	UFUNCTION() 
		void AccelerateGround(FVector& OutGroundAccelDir, bool& OutbApplyingGroundAccel, float& OutCalcMaxWalkSpeed);
	UFUNCTION(Server, Reliable)
		void ServerAccelerateGroundReplication(const FVector GroundAccelDir, const float CalcMaxWalk);
	UFUNCTION()
		void AccelerateGroundReplication(const FVector GroundAccelDir, const bool bApplyingGroundAccel, const float CalcMaxWalk);
	UFUNCTION()
		void HandleAccelerateGroundReplication(const FVector GroundAccelDir, const float CalcMaxWalk);
	#pragma endregion

	#pragma region Air Acceleration
	UFUNCTION()
		void AccelerateAir();
	UFUNCTION(Server, Reliable)
		void ServerAccelerateAirReplication();
	UFUNCTION()
		void AccelerateAirReplication();
	UFUNCTION()
		void HandleAccelerateAirReplication();
	#pragma endregion

	#pragma region Movement Input Air and Ground logic
	UFUNCTION() 
		void HandleMovement();
	UFUNCTION() 
		void BaseMovementLogic();
	UFUNCTION() 
		void MovementDelayLogic();
	#pragma endregion

	#pragma region Bhop and Trimp Logic
	UFUNCTION()
		void BhopAndTrimpLogic();
	UFUNCTION()
		void ApplyTrimp();
	UFUNCTION(Server, Reliable)
		void ServerApplyTrimpReplication();
	UFUNCTION()
		void ApplyTrimpReplication();
	UFUNCTION()
		void HandleApplyTrimpReplication();
	#pragma endregion

	#pragma region Bhop cap
	UFUNCTION()
		void BhopCap();
	UFUNCTION(Server, Reliable)
		void ServerBhopCapReplication();
	UFUNCTION()
		void BhopCapReplication();
	UFUNCTION()
		void HandleBhopCapReplication();
	#pragma endregion
	

	// Other bhop functions
	UFUNCTION()
		void ResetFrictionDelay();

	// the base bhop values
	#pragma region Base Bhop values
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis") // Save the previous velocity amount for calculating the acceleration from the bhop functions
		FVector PrevVelocity = FVector::Zero();
	UPROPERTY(EditAnywhere, Category = "Bhop_Analysis") // takes the magnitude of the XY velocity vector (equal to speed in X/Y plane)
		float XYspeedometer = 0.f;
	UPROPERTY(Replicated, EditAnywhere, Category = "Bhop_Analysis") // This is the deltatime saved from the tick component
		float FrameTime = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis") // he direction the air acceleration force is applied
		FVector AirAccelDir = FVector::Zero();
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis") // Whether they're applying air acceleration
		bool bApplyingAirAccel = false;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis") // The new air acceleration based on the current bhop buildup
		float CalcMaxAirSpeed = 0.f;
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Bhop_Analysis")
		FVector InputDirection = FVector::Zero();
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Bhop_Analysis")
		FVector InputForwardVector = FVector::Zero();
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float InputForwardAxis = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		FVector InputSideVector = FVector::Zero();
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float InputSideAxis = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		bool bIsRampSliding = false;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		int32 NumberOfTimesRampSlided = 0;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float DefaultMaxWalkSpeed = 800.f; // CharacterMovement->MaxWalkSpeed
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float DefaultBraking = 200.f; // CharacterMovement->BrakingDecelerationWalking
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float DefaultFriction = 8.f; // CharacterMovement->GroundFriction
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float DefaultJumpVelocity = 1000.f; // CharacterMovement->JumpZVelocity
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		FVector TrimpImpulse = FVector::Zero();
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float TrimpLateralImpulse = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float TrimpJumpImpulse = 0.f;

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
	UPROPERTY(EditAnywhere, Category = "Bhop_Bhop") // nannniiiiiiii?!?
		bool bEnableCrouchJump = false;
	UPROPERTY(EditAnywhere, Category = "Bhop_Bhop")
		bool bEnableSpeedometer = true;
	UPROPERTY(EditAnywhere, Category = "Bhop_Bhop") // The max speed achieved through majestic bhopping
		float MaxSeaDemonSpeed = 12069.f;

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
	UPROPERTY(Replicated, EditAnywhere, Category = "Bhop_RampSliding")
		float RampMomentumFactor = 1.f;

	UPROPERTY(EditAnywhere, Category = "Bhop_GroundAccel")
		bool bEnableGroundAccel = true;
	UPROPERTY(EditAnywhere, Category = "Bhop_GroundAccel")
		float GroundAccelerate = 100.f;

	UPROPERTY(EditAnywhere, Category = "Bhop_Audio")
		float LandingSoundCooldownTotal = 0.f;
	UPROPERTY(EditAnywhere, Category = "Bhop_Audio")
		float LegBreakThreshold = 2.5f;
	UPROPERTY(EditAnywhere, Category = "Bhop_Audio")
		float LandSoundCooldown = 1.f;
	UPROPERTY(EditAnywhere, Category = "Bhop_Audio")
		float JumpSoundCooldown = 1.f;

	UPROPERTY(EditAnywhere, Category = "Bhop_Other")
		FVector WeaponOffset = FVector(50.f, 33.f, 10.f);
	UPROPERTY(EditAnywhere, Category = "Bhop_Other")
		float BaseTurnRate = 45.f;
	UPROPERTY(EditAnywhere, Category = "Bhop_Other")
		float BaseLookUpRate = 45.f;
	UPROPERTY()
		float RampCheckGroundAngleDotproduct = 0.f;
	#pragma endregion

	UPROPERTY() // Our own stored reference of the variable to avoid constant get calls
		TObjectPtr<UCharacterMovementComponent> CachedCharacterMovement;
	UPROPERTY()
		uint32 NumberOfTimesPogoResetFrictionUUID = 0;

	UPROPERTY(EditAnywhere, Category = "Bhop_Other")
		uint32 DebugCharacterName = 0;


//////////////////////////////////////////////////////////////////////////
// Animations and Montages												//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
// Getters and Setters													//
//////////////////////////////////////////////////////////////////////////
public:
	void PrintToScreen(FColor color, FString message);

};
