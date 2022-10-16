// Fill out your copyright notice in the Description page of Project Settings.


#include "BhopCharacterMovementComponent.h"
#include "GameFramework/Character.h"



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bhop FSavedMove Implementation																																		 					 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region FSavedMove (Bhop)
// Prevent discrepencies between the network and client through direct assertions of replication
bool UBhopCharacterMovementComponent::FSavedMove_Bhop::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_Bhop* NewBhopMove = static_cast<FSavedMove_Bhop*>(NewMove.Get());

	// (Sprinting scenario) Check if was sprinting (if they pressed the sprint button on the client)
	if (Saved_bWantsToSprnt != NewBhopMove->Saved_bWantsToSprnt)
	{
		return false;
	}

	// Bhop implementations
	if (!FMath::IsNearlyEqual(Saved_BhopMaxWalkSpeed, NewBhopMove->Saved_BhopMaxWalkSpeed, MaxSpeedThresholdCombine))
	{
		return false;
	}
	if ((Saved_BhopMaxWalkSpeed == 0.0f) != (NewBhopMove->Saved_BhopMaxWalkSpeed == 0.0f))
	{
		return false;
	}

	if (!FMath::IsNearlyEqual(Saved_BhopGroundFriction, NewBhopMove->Saved_BhopGroundFriction, MaxSpeedThresholdCombine))
	{
		return false;
	}
	if ((Saved_BhopGroundFriction == 0.0f) != (NewBhopMove->Saved_BhopGroundFriction == 0.0f))
	{
		return false;
	}

	if (!FMath::IsNearlyEqual(Saved_BhopJumpZVelocity, NewBhopMove->Saved_BhopJumpZVelocity, MaxSpeedThresholdCombine))
	{
		return false;
	}
	if ((Saved_BhopJumpZVelocity == 0.0f) != (NewBhopMove->Saved_BhopJumpZVelocity == 0.0f))
	{
		return false;
	}

	// Run base logic
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}


void UBhopCharacterMovementComponent::FSavedMove_Bhop::CombineWith(const FSavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC, const FVector& OldStartLocation)
{
	Super::CombineWith(OldMove, InCharacter, PC, OldStartLocation);

	UCharacterMovementComponent* CharMovement = InCharacter->GetCharacterMovement();
	const FSavedMove_Bhop* OldBhopMove = static_cast<const FSavedMove_Bhop*>(OldMove);

	//// Pass in the bhop specific movement variables to the network
	//CharMovement->MaxWalkSpeed = OldBhopMove->Saved_BhopMaxWalkSpeed;
	//CharMovement->GroundFriction = OldBhopMove->Saved_BhopGroundFriction;
	//CharMovement->JumpZVelocity = OldBhopMove->Saved_BhopJumpZVelocity;
}


void UBhopCharacterMovementComponent::FSavedMove_Bhop::Clear()
{
	FSavedMove_Character::Clear();

	// Reset our logic
	Saved_bWantsToSprnt = 0;
	Saved_BhopMaxWalkSpeed = MAX_WALK_SPEED;
	Saved_BhopGroundFriction = JUMP_Z_VELOCITY;
	Saved_BhopJumpZVelocity = GROUND_FRICTION;
}


// This is the minimal movement information that's sent to the server every frame for replication
uint8 UBhopCharacterMovementComponent::FSavedMove_Bhop::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags(); // Base flags

	if (Saved_bWantsToSprnt) Result |= FLAG_Custom_0; // Flip custom flag 0 (Sprint channel), if is sprinting

	return Result;
}


// Captures the state data of the character movement component. Grabs all the safe variables in the CMC and set their respective save variables. (Sets the saved moves for the current snapshot of the CMC)
// This is where you set the saved move in case a packet is dropped containing this to minimize corrections
void UBhopCharacterMovementComponent::FSavedMove_Bhop::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	// Set our saved cmc values to the current(safe) values of the cmc
	UBhopCharacterMovementComponent* CharacterMovement = Cast<UBhopCharacterMovementComponent>(C->GetCharacterMovement());
	if (CharacterMovement)
	{
		Saved_bWantsToSprnt = CharacterMovement->Safe_bWantsToSprnt;
		Saved_BhopMaxWalkSpeed = CharacterMovement->Safe_BhopMaxWalkSpeed;
		Saved_BhopGroundFriction = CharacterMovement->Safe_BhopGroundFriction;
		Saved_BhopJumpZVelocity = CharacterMovement->Safe_BhopJumpZVelocity;
	}
}


// Take the data in the save move and apply it to the current state of the CMC
// This is called usually when a packet is dropped and resets the compressed flag to its saved state
void UBhopCharacterMovementComponent::FSavedMove_Bhop::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	// set our state data on the cmc equal to the saved values in the saved move snapshot
	UBhopCharacterMovementComponent* CharacterMovement = Cast<UBhopCharacterMovementComponent>(C->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->Safe_bWantsToSprnt = Saved_bWantsToSprnt;
		CharacterMovement->Safe_BhopMaxWalkSpeed = Saved_BhopMaxWalkSpeed;
		CharacterMovement->Safe_BhopGroundFriction = Saved_BhopGroundFriction;
		CharacterMovement->Safe_BhopJumpZVelocity = Saved_BhopJumpZVelocity;
	}
}
#pragma endregion


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bhop FNetworkPredicitonData_Client Implementation																															 			 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region FNetworkPredictionData_Client_Character (Bhop)
UBhopCharacterMovementComponent::FNetworkPredictionData_Client_BhopCharacter::FNetworkPredictionData_Client_BhopCharacter(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement) {}


FSavedMovePtr UBhopCharacterMovementComponent::FNetworkPredictionData_Client_BhopCharacter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Bhop());
} 
#pragma endregion


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bhop CharacterNetworkMoveData																																	 						 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region FCharacterNetworkMoveData (Bhop)
UBhopCharacterMovementComponent::FBhopCharacterNetworkMoveDataContainer::FBhopCharacterNetworkMoveDataContainer()
{
	// Override this with our own saved move objects (FBhopCharacterNetworkMoveData BhopDefaultMoveData)
	NewMoveData = &BhopDefaultMoveData[0];
	PendingMoveData = &BhopDefaultMoveData[1];
	OldMoveData = &BhopDefaultMoveData[2];
}


void UBhopCharacterMovementComponent::FBhopCharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(ClientMove, MoveType);

	// Send the bhop specific implementations across the network
	const FSavedMove_Bhop& BhopClientMove = static_cast<const FSavedMove_Bhop&>(ClientMove);
	Saved_BhopMaxWalkSpeed = BhopClientMove.Saved_BhopMaxWalkSpeed;
	Saved_BhopGroundFriction = BhopClientMove.Saved_BhopGroundFriction;
	Saved_BhopJumpZVelocity = BhopClientMove.Saved_BhopJumpZVelocity;
}


bool UBhopCharacterMovementComponent::FBhopCharacterNetworkMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType);

	// Serialize all the information to be sent across the network (to and from)
	//bool bLocalSuccess = true;
	SerializeOptionalValue<float>(Ar.IsSaving(), Ar, Saved_BhopMaxWalkSpeed, MAX_WALK_SPEED);
	SerializeOptionalValue<float>(Ar.IsSaving(), Ar, Saved_BhopGroundFriction, GROUND_FRICTION);
	SerializeOptionalValue<float>(Ar.IsSaving(), Ar, Saved_BhopJumpZVelocity, JUMP_Z_VELOCITY);
	//Acceleration.NetSerialize(Ar, PackageMap, bLocalSuccess);
	//Acceleration.NetSerialize(Ar, PackageMap, bLocalSuccess);
	//Acceleration.NetSerialize(Ar, PackageMap, bLocalSuccess);
	
	return !Ar.IsError();
}
#pragma endregion






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bhop Character Movement Component Stuff																															 						 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UBhopCharacterMovementComponent::UBhopCharacterMovementComponent()
{
	#pragma region Character movement compendium
	// CharacterMovement (General Settings)
	GravityScale = 2.8f; // pertains to bhop
	MaxAcceleration = 6009.f; // pertains to bhop
	BrakingFrictionFactor = 2.f;
	BrakingFriction = 0.f;
	bUseSeparateBrakingFriction = false;
	Mass = 100.f;
	DefaultLandMovementMode = EMovementMode::MOVE_Walking;
	DefaultWaterMovementMode = EMovementMode::MOVE_Swimming;

	// Character Movement: Walking
	MaxStepHeight = 45.f;
	SetWalkableFloorAngle(44.765309);
	SetWalkableFloorZ(0.71);
	GroundFriction = GROUND_FRICTION;
	MaxWalkSpeed = MAX_WALK_SPEED; // pertains to bhop
	MaxWalkSpeedCrouched = 300.f;
	MinAnalogWalkSpeed = 0.f;
	BrakingDecelerationWalking = 200.f; // pertains to bhop
	bSweepWhileNavWalking = true;
	bCanWalkOffLedges = true;
	bCanWalkOffLedgesWhenCrouching = false;
	bMaintainHorizontalGroundVelocity = false; // pertains to bhop
	bIgnoreBaseRotation = false;
	PerchRadiusThreshold = 0.f;
	PerchAdditionalHeight = 40.f;
	LedgeCheckThreshold = 4.f;
	bAlwaysCheckFloor = false;
	bUseFlatBaseForFloorChecks = false;

	// Character Movement: Jumping/Falling
	JumpZVelocity = JUMP_Z_VELOCITY; // pertains to bhop
	BrakingDecelerationFalling = 0.f;
	AirControl = 100.f; // pertains to bhop
	AirControlBoostMultiplier = 2.f;
	AirControlBoostVelocityThreshold = 25.f;
	FallingLateralFriction = 0.f;
	bImpartBaseVelocityX = true;
	bImpartBaseVelocityY = true;
	bImpartBaseVelocityZ = true;
	bImpartBaseAngularVelocity = true;
	bNotifyApex = false;
	JumpOffJumpZFactor = 0.5f;
	bApplyGravityWhileJumping = true;

	// Character Movement (Networking)
	NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
	bNetworkSkipProxyPredictionOnNetUpdate = false;
	NetworkMaxSmoothUpdateDistance = 256.f;
	NetworkNoSmoothUpdateDistance = 384.f;
	NetworkMinTimeBetweenClientAckGoodMoves = 0.1f;
	NetworkMinTimeBetweenClientAdjustments = 0.1f;
	NetworkMinTimeBetweenClientAdjustmentsLargeCorrection = 0.05f;
	NetworkLargeClientCorrectionDistance = 15.f;
	bNetworkAlwaysReplicateTransformUpdateTimestamp = false;
	NetworkSimulatedSmoothLocationTime = 0.1f;
	NetworkSimulatedSmoothRotationTime = 0.05f;
	ListenServerNetworkSimulatedSmoothLocationTime = 0.04f;
	ListenServerNetworkSimulatedSmoothRotationTime = 0.033f;
	NetProxyShrinkRadius = 0.01f;
	NetProxyShrinkHalfHeight = 0.01f;

	// Movement Capabilities
	NavAgentProps.AgentHeight = 48.f; // pertains to bhop (i thunk) TODO: Fix these (These are not being set properly, do it manualy)
	NavAgentProps.AgentRadius = 192.f; // pertains to bhop (i thunk) TODO: Fix these (These are not being set properly, do it manualy)
	NavAgentProps.AgentStepHeight = -1.f;
	NavAgentProps.NavWalkingSearchHeightScale = 0.5f;
	NavAgentProps.bCanCrouch = true;
	NavAgentProps.bCanJump = true;
	NavAgentProps.bCanWalk = true;
	NavAgentProps.bCanSwim = true;
	NavAgentProps.bCanFly = false;

	// Movement Component
	bUpdateOnlyIfRendered = false;
	bAutoUpdateTickRegistration = true;
	bTickBeforeOwner = true;
	bAutoRegisterUpdatedComponent = true;
	bAutoRegisterPhysicsVolumeUpdates = true;
	bComponentShouldUpdatePhysicsVolume = true;
#pragma endregion

	// These are the saved default values of the bhop variables
	DefaultMaxWalkSpeed = MaxWalkSpeed;
	DefaultMaxSprintSpeed = MaxWalkSpeed * 2;
	DefaultGroundFriction = GroundFriction;
	DefaultJumpZVelocity = JumpZVelocity;
}


FNetworkPredictionData_Client* UBhopCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (ClientPredictionData == nullptr)
	{
		UBhopCharacterMovementComponent* MutableThis = const_cast<UBhopCharacterMovementComponent*>(this); // This is a workaround of const (in the case the prediction data is undefined and we have to create it)
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_BhopCharacter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}


void UBhopCharacterMovementComponent::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel)
{
	FBhopCharacterNetworkMoveData* MoveData = static_cast<FBhopCharacterNetworkMoveData*>(GetCurrentNetworkMoveData());
	if (MoveData != nullptr)
	{
		Safe_BhopMaxWalkSpeed = MoveData->Saved_BhopMaxWalkSpeed;
		Safe_BhopGroundFriction = MoveData->Saved_BhopGroundFriction;
		Safe_BhopJumpZVelocity = MoveData->Saved_BhopJumpZVelocity;
	}

	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAccel);
}


void UBhopCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	// Sprint logic
	if (MovementMode == MOVE_Walking)
	{
		MaxWalkSpeed = Safe_BhopMaxWalkSpeed;
		GroundFriction = Safe_BhopGroundFriction;
		JumpZVelocity = Safe_BhopJumpZVelocity;

		if (Safe_bWantsToSprnt)
		{
			if (MaxWalkSpeed < DefaultMaxSprintSpeed) MaxWalkSpeed = DefaultMaxSprintSpeed;
		}
		else
		{
			MaxWalkSpeed = DefaultMaxWalkSpeed;
		}
	}
}



void UBhopCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprnt = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;

}


UFUNCTION(BlueprintCallable) void UBhopCharacterMovementComponent::SprintPressed()
{
	Safe_bWantsToSprnt = true;
}

UFUNCTION(BlueprintCallable) void UBhopCharacterMovementComponent::SprintReleased()
{
	Safe_bWantsToSprnt = false;
}

UFUNCTION(BlueprintCallable) void UBhopCharacterMovementComponent::SetBhopMaxWalkSpeed(float Value)
{
	Safe_BhopMaxWalkSpeed = Value;
}

UFUNCTION(BlueprintCallable) void UBhopCharacterMovementComponent::SetBhopGroundFriction(float Value)
{
	Safe_BhopGroundFriction = Value;
}

UFUNCTION(BlueprintCallable) void UBhopCharacterMovementComponent::SetBhopJumpZVelocity(float Value)
{
	Safe_BhopJumpZVelocity = Value;
}
