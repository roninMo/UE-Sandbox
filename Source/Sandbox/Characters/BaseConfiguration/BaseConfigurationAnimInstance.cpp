// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseConfigurationAnimInstance.h"
#include "Sandbox/Characters/BaseConfiguration/BaseCharacterConfiguration.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void UBaseConfigurationAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ABaseCharacterConfiguration>(TryGetPawnOwner());
}


void UBaseConfigurationAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (Character == nullptr) Character = Cast<ABaseCharacterConfiguration>(TryGetPawnOwner());
	if (Character == nullptr) return;

	//// Grab information of the character for the animations ////
	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f; // We only want the lateral speed, so zero out the z index (upward movement)

	Speed = Velocity.Size(); // The magnitude is the speed of something
	bIsAccelerating = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bIsInAir = Character->GetCharacterMovement()->IsFalling();
	bIsCrouched = Character->bIsCrouched;

	// Offset yaw for strafing on the server
	FRotator AimRotation = Character->GetBaseAimRotation(); // The current direction the character is facing in the world // GetBaseAimRotation: built in function to grab the offset of where the character is aiming
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Velocity); // The direction (offset) of the character while moving
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation); 
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 4.5f); // Unreal's magical interpolation function to avoid the bad interpolation animation vibes It interps the ranges like a clock, not like a number
	YawOffset = DeltaRotation.Yaw;

	// Character lean for the server
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = Character->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame); // The delta between the current and last lean (angles baby)
	const float Target = Delta.Yaw / DeltaTime; // This scales it up and makes it proportionate to delta time
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 4.5f); // To remove the jitteriness in the lean we do an interp here
	Lean = FMath::Clamp(Interp, -90.f, 90.f); // Clamp this value so it doesn't break the character's back 

	if (Character->IsLocallyControlled()) bLocallyControlled = true;
}
