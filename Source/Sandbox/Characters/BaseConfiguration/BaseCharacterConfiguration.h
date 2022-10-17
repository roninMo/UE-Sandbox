// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacterConfiguration.generated.h"

UCLASS()
class SANDBOX_API ABaseCharacterConfiguration : public ACharacter
{
	GENERATED_BODY()


//////////////////////////////////////////////////////////////////////////
// Base functions and components										//
//////////////////////////////////////////////////////////////////////////
public:
	/** Returns CharacterMovement subobject **/
	FORCEINLINE class UCMCBaseConfiguration* GetBaseCharacterMovement() const { return BaseCharacterMovement; }
	ABaseCharacterConfiguration(const FObjectInitializer& ObjectInitializer);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	//virtual void Destroyed() override; // This is a replicated function, handle logic pertaining to character death in here for free

	//virtual void OnRep_ReplicatedMovement() override; // overriding this to replicate simulated proxies movement: https://www.udemy.com/course/unreal-engine-5-cpp-multiplayer-shooter/learn/lecture/31515548#questions


protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement) class UCMCBaseConfiguration* BaseCharacterMovement;


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
	virtual void MoveForward(float Value);
	virtual void MoveRight(float Value);
	virtual void Turn(float Value);
	virtual void Lookup(float Value);
	virtual void StartJump();
	virtual void StopJump();
	virtual void StartSprint();
	virtual void StopSprint();
	virtual void CrouchButtonPressed();
	virtual void CrouchButtonReleased();


private:
	UPROPERTY()
		bool bIsSprinting = false;


//////////////////////////////////////////////////////////////////////////
// Animations and Montages												//
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
// Getters and Setters													//
//////////////////////////////////////////////////////////////////////////
public:
	void PrintToScreen(FColor color, FString message);


};
