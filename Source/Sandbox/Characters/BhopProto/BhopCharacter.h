// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

// Gameplay Ability System
#include "AbilitySystemInterface.h"

#include "BhopCharacter.generated.h"


UCLASS()
class SANDBOX_API ABhopCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()



//////////////////////////////////////////////////////////////////////////
// Base functions and components										//
//////////////////////////////////////////////////////////////////////////
public:
	ABhopCharacter(const FObjectInitializer& ObjectInitializer );
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	//virtual void Destroyed() override; // This is a replicated function, handle logic pertaining to character death in here for free

	//virtual void OnRep_ReplicatedMovement() override; // overriding this to replicate simulated proxies movement: https://www.udemy.com/course/unreal-engine-5-cpp-multiplayer-shooter/learn/lecture/31515548#questions


protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement) class UBhopCharacterMovementComponent* BhopCharacterMovementComponent;


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
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)  UBhopCharacterMovementComponent* BhopCharacterMovement;

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
	UPROPERTY(EditAnywhere, Category = "Audio")
		class USoundCue* JumpSound;

	#pragma region Reset Friction
	UFUNCTION()
		void ResetFriction();
	UFUNCTION(Server, Reliable)
		void ServerResetFriction();
	UFUNCTION(NetMulticast, Reliable)
		void HandleResetFriction();
	#pragma endregion

	#pragma region Remove Friction
	UFUNCTION()
		void RemoveFriction();
	UFUNCTION(Server, Reliable)
		void ServerRemoveFriction();
	UFUNCTION(NetMulticast, Reliable)
		void HandleRemoveFriction();
	#pragma endregion

	#pragma region Ramp Slide and Checks
	UFUNCTION()
		bool RamSlide();
	UFUNCTION()
		bool RampCheck();
	#pragma endregion

	#pragma region Add Ramp Momentum
	UFUNCTION()
		void AddRampMomentum();
	UFUNCTION(Server, Reliable)
		void ServerAddRampMomentum();
	UFUNCTION(NetMulticast, Reliable)
		void HandleAddRampMomentum();
	#pragma endregion

	#pragma region Acceleration Functions
	UFUNCTION() 
		void AccelerateGround();
	UFUNCTION()
		void AccelerateAir();
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
	UFUNCTION()
		void ApplyTrimpReplication();
	UFUNCTION(Server, Reliable)
		void ServerApplyTrimpReplication();
	UFUNCTION(NetMulticast, Reliable)
		void HandleApplyTrimpReplication();
	#pragma endregion

	#pragma region Bhop cap
	UFUNCTION()
		void ApplyBhopCap();
	UFUNCTION()
		void BhopCapReplication();
	UFUNCTION(Server, Reliable)
		void ServerBhopCapReplication();
	UFUNCTION(NetMulticast, Reliable)
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
		float FrameTime = 1.f;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis") // the direction the ground acceleration impulse is applied
		FVector GroundAccelDir = FVector::Zero();
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis") // Whether they're applying ground acceleration
		bool bApplyingGroundAccel = false;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis") // The new ground acceleration based on the current bhop buildup
		float CalcMaxWalkSpeed = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis") // the direction the air acceleration impulse is applied
		FVector AirAccelDir = FVector::Zero();
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis") // Whether they're applying air acceleration
		bool bApplyingAirAccel = false;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis") // The new air acceleration based on the current bhop buildup
		float CalcMaxAirSpeed = 0.f;
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Bhop_Analysis")
		FVector InputDirection = FVector::Zero();
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		FVector InputForwardVector = FVector::Zero();
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float InputForwardAxis = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		FVector InputSideVector = FVector::Zero();
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		float InputSideAxis = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		bool bJumpPressed = false;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		bool bIsRampSliding = false;
	UPROPERTY(VisibleAnywhere, Category = "Bhop_Analysis")
		int32 NumberOfTimesRampSlided = 0;
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Bhop_Analysis")
		float DefaultMaxWalkSpeed = 800.f; // CharacterMovement->MaxWalkSpeed
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
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Bhop_Analysis")
		bool bApplyingBhopCap = false;

	UPROPERTY(EditAnywhere, Category = "Bhop_AirAccel")
		bool bEnableCustomAirAccel = true;
	UPROPERTY(EditAnywhere, Category = "Bhop_AirAccel")
		float AirAccelerate = 10.f;

	UPROPERTY(EditAnywhere, Category = "Bhop_Bhop")
		bool bEnablePogo = false; // This is buggy, and is not reliable in multiplayer. Tldr, work on it
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

	UPROPERTY(EditAnywhere, Category = "Bhop_Cap")
		FVector BhopCapVector = FVector::Zero();
	UPROPERTY(EditAnywhere, Category = "Bhop_Cap")
		float bhopCapNewSpeed = 0.f;

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
		float JumpSoundCooldownTotal = 0.f;
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


public:
	/** Returns CharacterMovement subobject **/
	FORCEINLINE class UBhopCharacterMovementComponent* GetBhopCharacterMovement() const { return BhopCharacterMovement; }


//////////////////////////////////////////////////////////////////////////
// Gameplay Ability System												//
//////////////////////////////////////////////////////////////////////////
public:
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void PossessedBy(AController* NewController) override; // Initialize ability system on Server call
	virtual void OnRep_PlayerState() override; // Initialize ability system on Client call


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute Gas")
		class UProtoASC* AbilitySystemComponent;
	UPROPERTY()
		class UProtoAttributeSet* Attributes;


protected:
	virtual void InitializeAttributes();
	virtual void GiveBaseAbilities();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Attribute Gas")
		TSubclassOf<class UGameplayEffect> DefaultAttributeEffect;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Attribute Gas")
		TArray<TSubclassOf<class UProtoGasGameplayAbility>> DefaultAbilties;	


//////////////////////////////////////////////////////////////////////////
// Animations and Montages												//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Getters and Setters													//
//////////////////////////////////////////////////////////////////////////
public:
	void InitCharacterMovement();
	FORCEINLINE float GetSpeedometer() { return XYspeedometer; };
	float GetDefaultMaxWalkSpeed();
	float GetFriction();
	void PrintToScreen(FColor color, FString message);


};
