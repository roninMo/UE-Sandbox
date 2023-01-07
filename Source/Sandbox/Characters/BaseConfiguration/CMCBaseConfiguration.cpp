// Fill out your copyright notice in the Description page of Project Settings.


#include "CMCBaseConfiguration.h"
#include "GameFramework/Character.h"


// CMC network breakdown
// First on tick the perform move function is called, which executes all the movement logic
// Then it creates a saved move, and uses SetMoveFor to read the safe values and store them in the saved values
// Calls canCombineMoveWith to check if there are any identical moves that can be combined to save bandwidth if necessary
// Then Calls getCompressed flags to reduce the saved move into the small networkable packet that it sends to the server
// Then when the server receives this move it will call updateFromCompressedFlags, and update the state variables with the values sent to it
// Then everything should be replicated and there shouldn't be any weird rubber banding conflicts with the code. God sweet baby jesus please for once

// NOTE:
// You can only call moves that alter safe variables on the client
// You can alter movement safe variables in non safe momement functions on the client
// You can never utilize non movement safe variables in a movement safe function
// And you can't call non movement safe functions that alter movement safe variables on the server




// CMC physics breakdown
// TickComponent grabs your input vectors to determine your direction
// Then it either performs the move on the server, or replicates it to the server through PerformMovement(Auth), or ReplciateMovetoServer(Client)
// The server immediately performs the move, and the client ReplicateMoveToServer replicates the movement then calls performMovement.
// Once that's done it calls PostUpdate, which is records every server created instance that it wants to create with the client.
// It then pushes it to an array of saved moves that it keeps track of to test against the client's saved moves. This is a very nice way of knocking out the client and server discrepancies by creating these arrays.
// Until a move is acknowledged, it stays in that array and is kept track of. It then runs the CallServerMove function to do the same thing.
// On the client side we've moved, and now it's time for the server to checkout our move. It does some timestamp stuff to prevent speed hacking, and then calls MoveAutonomous, which configures things like acceleration before calling performMove
// After MoveAutonomous the server has moved in the same way that your client has. Then it checks if there was an issue through the ServerMoveHandleClientError function
// If there was an error, it creates an adjustment to circumvent the error that occurred. Otherwise if there wasn't an error, it just sends back acknowledges it as a good move
// 
// If there was an error and a correction needed to be made, it sets them in UNetDriver.ServerReplicateActors, and this is were it gets complicated
// Inside of ServerReplicateActors it grabs the playercontroller and sends the client adjustment.
// 
// StartNewPhysics that determines what state your character is in, and then implements the physics for that specific phys function




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bhop FSavedMove Implementation																																		 					 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region FSavedMove (Bhop)
// Prevent discrepencies between the network and client through direct assertions of replication
bool UCMCBaseConfiguration::CMCB_FSavedMove_Character::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	CMCB_FSavedMove_Character* NewBhopMove = static_cast<CMCB_FSavedMove_Character*>(NewMove.Get());

	// (Sprinting scenario) Check if was sprinting (if they pressed the sprint button on the client)
	if (Saved_bWantsToSprnt != NewBhopMove->Saved_bWantsToSprnt)
	{
		return false;
	}

	// Run base logic
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}


void UCMCBaseConfiguration::CMCB_FSavedMove_Character::Clear()
{
	Super::Clear();

	// Reset our logic
	Saved_bWantsToSprnt = 0;
}


// This is the minimal movement information that's sent to the server every frame for replication
uint8 UCMCBaseConfiguration::CMCB_FSavedMove_Character::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags(); // Base flags

	if (Saved_bWantsToSprnt) Result |= FLAG_Custom_0; // Flip custom flag 0 (Sprint channel), if is sprinting

	return Result;
}


// Captures the state data of the character movement component. Grabs all the safe variables in the CMC and set their respective save variables. (Sets the saved moves for the current snapshot of the CMC)
// This is where you set the saved move in case a packet is dropped containing this to minimize corrections
void UCMCBaseConfiguration::CMCB_FSavedMove_Character::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	// Set our saved cmc values to the current(safe) values of the cmc
	UCMCBaseConfiguration* CharacterMovement = Cast<UCMCBaseConfiguration>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		Saved_bWantsToSprnt = CharacterMovement->Safe_bWantsToSprnt;
	}
}


// Take the data in the save move and apply it to the current state of the CMC
// This is called usually when a packet is dropped and resets the compressed flag to its saved state
void UCMCBaseConfiguration::CMCB_FSavedMove_Character::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	// set our state data on the cmc equal to the saved values in the saved move snapshot
	UCMCBaseConfiguration* CharacterMovement = Cast<UCMCBaseConfiguration>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->Safe_bWantsToSprnt = Saved_bWantsToSprnt;
	}
}
#pragma endregion


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bhop FNetworkPredicitonData_Client Implementation																															 			 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region FNetworkPredictionData_Client_Character (Bhop)
UCMCBaseConfiguration::CMCB_FNetworkPredictionData_Client_Character::CMCB_FNetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement) {}


FSavedMovePtr UCMCBaseConfiguration::CMCB_FNetworkPredictionData_Client_Character::AllocateNewMove()
{
	return FSavedMovePtr(new CMCB_FSavedMove_Character());
}
#pragma endregion


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bhop CharacterNetworkMoveData																																	 						 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region FCharacterNetworkMoveData (Bhop)
UCMCBaseConfiguration::CMCB_CharacterNetworkMoveDataContainer::CMCB_CharacterNetworkMoveDataContainer()
{
	// Override this with our own saved move objects (FBhopCharacterNetworkMoveData BhopDefaultMoveData)
	NewMoveData = &CMCBDefaultMoveData[0];
	PendingMoveData = &CMCBDefaultMoveData[1];
	OldMoveData = &CMCBDefaultMoveData[2];
}


void UCMCBaseConfiguration::CMCB_FCharacterNetworkMoveData::ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(ClientMove, MoveType);

	// Send the bhop specific implementations across the network
	const CMCB_FSavedMove_Character& BhopClientMove = static_cast<const CMCB_FSavedMove_Character&>(ClientMove);
}


bool UCMCBaseConfiguration::CMCB_FCharacterNetworkMoveData::Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType)
{
	Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType);

	// Serialize all the information to be sent across the network (to and from)
	//bool bLocalSuccess = true;
	//SerializeOptionalValue<float>(Ar.IsSaving(), Ar, Saved_BhopMaxWalkSpeed, MAX_WALK_SPEED);
	//SerializeOptionalValue<float>(Ar.IsSaving(), Ar, Saved_BhopGroundFriction, GROUND_FRICTION);
	//SerializeOptionalValue<float>(Ar.IsSaving(), Ar, Saved_BhopJumpZVelocity, JUMP_Z_VELOCITY);
	//Acceleration.NetSerialize(Ar, PackageMap, bLocalSuccess);
	//Acceleration.NetSerialize(Ar, PackageMap, bLocalSuccess);
	//Acceleration.NetSerialize(Ar, PackageMap, bLocalSuccess);

	return !Ar.IsError();
}
#pragma endregion






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bhop Character Movement Component Stuff																															 						 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UCMCBaseConfiguration::UCMCBaseConfiguration()
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
}


FNetworkPredictionData_Client* UCMCBaseConfiguration::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (ClientPredictionData == nullptr)
	{
		UCMCBaseConfiguration* MutableThis = const_cast<UCMCBaseConfiguration*>(this); // This is a workaround of const (in the case the prediction data is undefined and we have to create it)
		MutableThis->ClientPredictionData = new CMCB_FNetworkPredictionData_Client_Character(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}


void UCMCBaseConfiguration::MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel)
{
	Super::MoveAutonomous(ClientTimeStamp, DeltaTime, CompressedFlags, NewAccel);
}


void UCMCBaseConfiguration::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprnt = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}


void UCMCBaseConfiguration::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	// Sprint logic
	if (MovementMode == MOVE_Walking)
	{
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


float UCMCBaseConfiguration::GetMaxSpeed() const
{
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		return IsCrouching() ? MaxWalkSpeedCrouched : MaxWalkSpeed;
	case MOVE_Falling:
		return MaxWalkSpeed;
	case MOVE_Swimming:
		return MaxSwimSpeed;
	case MOVE_Flying:
		return MaxFlySpeed;
	case MOVE_Custom:
		return MaxCustomMovementSpeed;
	case MOVE_None:
	default:
		return 0.f;
	}
}



UFUNCTION(BlueprintCallable) void UCMCBaseConfiguration::SprintPressed()
{
	Safe_bWantsToSprnt = true;
}

UFUNCTION(BlueprintCallable) void UCMCBaseConfiguration::SprintReleased()
{
	Safe_bWantsToSprnt = false;
}
