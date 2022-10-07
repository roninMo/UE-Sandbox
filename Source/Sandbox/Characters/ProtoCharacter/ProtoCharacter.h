// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProtoCharacter.generated.h"

UCLASS()
class SANDBOX_API AProtoCharacter : public ACharacter
{
	GENERATED_BODY()


//////////////////////////////////////////////////////////////////////////
// TODO (Keep this stuff organized baby)								//
//////////////////////////////////////////////////////////////////////////
/* 
	* Create our own character movement component
	* Create an animation instance (a kinematic rig to adopt animations with ease (use the animation starter pack)
	* Create a hud and learn how to make it spiffy
	* Create an inventory component as a friend class of the character component
	* Learn AI, and how to implement it through C++ (Start with the Utility Ai with replicated functionality (This should be easier since AI is just basic movement, everything is handled on the server anyways))
	* Basic combat (learn from the ai), Create lock on, hit animations, and projectile based weapons (bow) 

	* Later
	* Fancy animations and characters, play around in blender
*/ 





//////////////////////////////////////////////////////////////////////////
// Base functions and components										//
//////////////////////////////////////////////////////////////////////////
public:
	// Sets default values for this character's properties
	AProtoCharacter();
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
	void LockOn();


private:
	UPROPERTY()
		bool bIsSprinting = false;
	UPROPERTY()
		float DefaultMaxWalkSpeed = 0.f;


//////////////////////////////////////////////////////////////////////////
// Bunny Hopping Stuff													//
//////////////////////////////////////////////////////////////////////////

	UFUNCTION(Server, Reliable)
		void ServerHandleMovementReplication(FVector Direction, float Value);
	UFUNCTION(NetMulticast, Reliable)
		void ClientHandleMovementReplication(FVector Direction, float Value);
	UFUNCTION()
		void HandleMovement(float Value, EAxis::Type Axis);

};
