// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseConfigurationAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SANDBOX_API UBaseConfigurationAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	

public:
	virtual void NativeInitializeAnimation() override; // This is much like BeginPlay
	virtual void NativeUpdateAnimation(float DeltaTime) override; // This is a lot like the tick function and it's called every frame 


private:
	// Everywhere blueprint vars
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		class ABaseCharacterConfiguration* Character;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float Speed;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsAccelerating;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsInAir;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsCrouched;

	// Blendspace vars
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float YawOffset;
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float Lean;

	// Aim offset vars
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bLocallyControlled;


	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	// This is to fix interpolation errors when you reach the bounds. if it interpolates from -180 to 180, it will go through all the animations making a really jerky animation while playing...
	FRotator DeltaRotation; // Using unreal's interpolations will go from -180 > 180 kinda like a clock, instead of through all the numbers


};
