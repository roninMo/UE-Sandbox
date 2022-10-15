// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BhopCharacterMovementComponent.generated.h"

/*
Tips for handling the network replication errorsstuff

Here are some suggestions:
1. Make sure it is an actual desync, not something else (frame drop or performance issue): do this by using the command: "p.NetShowCorrections 1" and also use the command emulationPkLag 500 to simulate a ping of 500. 
	If the problem is actually rubberbanding, you will see many green and red capsules.

2. Given that the problem is a desync, this is strange but make sure you exhaust all cases to find out exactly where your problem is, this is kinda obvious but its crucial to understanding your problem.
	Test jumping, jumping while walking, jumping while sprinting, walking off a ledge, sprinting off a ledge, ect. You want to eliminate as many cases as possible.

3. Assuming the desync only happens when jumping while sprinting, I'm not sure off the top of my head why this would be but here are some things I would check:

a) You aren't using the same compressed flag for jumping and sprinting.
b) You have correctly implemented CanCombineWith on the SavedMove class.
c) All of the usages of Saved_bWantsToSprint are correctly implemented.

Sorry these suggestions aren't great, I just don't have enough info on your specific problem but it does seem like a movement safety error. A good way that I debug movement safety is to create a log message that prints 
" {Server or Client}   {TimeStamp}    {Safe_bWantsToSprint}  "
TimeStamp can be read from the NetworkPredictionData.

Your goal here is to make sure that the value of Safe_bWantsToSprint is the same on the client and server when the TimeStamp is the same.
Hope this helps!


"p.NetShowCorrections 1"
"emulationPkLag 500"
*/


/*
*@Documentation Extending Saved Move Data

To add new data, first extend FSavedMove_Character to include whatever information your Character Movement Component needs.
Next, extend FCharacterNetworkMoveData and add the custom data you want to send across the network; in most cases, this mirrors the data added to FSavedMove_Character.
You will also need to extend FCharacterNetworkMoveDataContainer so that it can serialize your FCharacterNetworkMoveData for network transmission, and deserialize it upon receipt. When this setup is finised, configure the system as follows:

	- Modify your Character Movement Component to use the FCharacterNetworkMoveDataContainer subclass you created with the SetNetworkMoveDataContainer function.
		The simplest way to accomplish this is to add an instance of your FCharacterNetworkMoveDataContainer to your Character Movement Component child class, and call SetNetworkMoveDataContainer from the constructor.

	- Since your FCharacterNetworkMoveDataContainer needs its own instances of FCharacterNetworkMoveData, point it (typically in the constructor) to instances of your FCharacterNetworkMoveData subclass.
		See the base constructor for more details and an example.

	- In your extended version of FCharacterNetworkMoveData, override the ClientFillNetworkMoveData function to copy or compute data from the saved move.
		Override the Serialize function to read and write your data using an FArchive; this is the bit stream that RPCs require.

	- To extend the server response to clients, which can acknowledges a good move or send correction data, extend FCharacterMoveResponseData, FCharacterMoveResponseDataContainer,
		and override your Character Movement Component's version of the SetMoveResponseDataContainer.
*/


/**
 * 
 */


// Defining the base movespeeds here because they're referenced across multiple classes
#define MAX_WALK_SPEED 880.f
#define JUMP_Z_VELOCITY 999.f
#define GROUND_FRICTION 8.f


UCLASS()
class SANDBOX_API UBhopCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()


public:
	UBhopCharacterMovementComponent();


	//////////////////////////////////////////////
	// Custom FSavedMove (Bhop Implementation)	//
	//////////////////////////////////////////////
	class FSavedMove_Bhop : public FSavedMove_Character
	{
	public:
		typedef FSavedMove_Character Super;

		// Other values values we want to pass into the saved moves
		uint8 Saved_bWantsToSprnt : 1;
		float Saved_BhopMaxWalkSpeed = MAX_WALK_SPEED;
		float Saved_BhopGroundFriction = JUMP_Z_VELOCITY;
		float Saved_BhopJumpZVelocity = GROUND_FRICTION;

		// Functions 
		/** Returns true if this move can be combined with NewMove for replication without changing any behavior */
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

		/** Combine this move with an older move and update relevant state. */
		virtual void CombineWith(const FSavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC, const FVector& OldStartLocation) override;

		/** Clear saved move properties, so it can be re-used. */
		virtual void Clear() override;

		/** Returns a byte containing encoded special movement information (jumping, crouching, etc.)	 */
		virtual uint8 GetCompressedFlags() const override;

		/** Called to set up this saved move (when initially created) to make a predictive correction. */
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

		/** Called before ClientUpdatePosition uses this SavedMove to make a predictive correction	 */
		virtual void PrepMoveFor(ACharacter* C) override;
	};


	//////////////////////////////////////////////////////////////////
	// Custom FNetworkPredictionData_Client (Bhop Implementation)	//
	//////////////////////////////////////////////////////////////////
	// Indicate to the cmc that we're using our custom move FSavedMove_Bhop
	class FNetworkPredictionData_Client_BhopCharacter : public FNetworkPredictionData_Client_Character
	{
	public:
		typedef FNetworkPredictionData_Client_Character Super;
		FNetworkPredictionData_Client_BhopCharacter(const UCharacterMovementComponent& ClientMovement);
		/* Creates a copy of the new move */
		virtual FSavedMovePtr AllocateNewMove() override;
	};


	//////////////////////////////////////////////////////////////////
	// Custom FNetworkPredictionData_Client (Bhop Implementation)	//
	//////////////////////////////////////////////////////////////////
	/**
	 * FCharacterNetworkMoveData encapsulates a client move that is sent to the server for UCharacterMovementComponent networking.
	 *
	 * Adding custom data to the network move is accomplished by deriving from this struct, adding new data members, implementing ClientFillNetworkMoveData(), implementing Serialize(),
	 * and setting up the UCharacterMovementComponent to use an instance of a custom FCharacterNetworkMoveDataContainer (see that struct for more details).
	 *
	 * @see FCharacterNetworkMoveDataContainer
	 */
	class FBhopCharacterNetworkMoveData : public FCharacterNetworkMoveData
	{
	public:
		typedef FCharacterNetworkMoveData Super;
		/**
		 * Given a FSavedMove_Character from UCharacterMovementComponent, fill in data in this struct with relevant movement data.
		 * Note that the instance of the FSavedMove_Character is likely a custom struct of a derived struct of your own, if you have added your own saved move data.
		 * @see UCharacterMovementComponent::AllocateNewMove()
		 */
		virtual void ClientFillNetworkMoveData(const FSavedMove_Character& ClientMove, ENetworkMoveType MoveType) override;
		/**
		 * Serialize the data in this struct to or from the given FArchive. This packs or unpacks the data in to a variable-sized data stream that is sent over the
		 * network from client to server.
		 * @see UCharacterMovementComponent::CallServerMovePacked
		 */
		virtual bool Serialize(UCharacterMovementComponent& CharacterMovement, FArchive& Ar, UPackageMap* PackageMap, ENetworkMoveType MoveType) override; // Data compression for efficient transfer across the network

		// Other information we want to send across the network (since it's being updated every frame)
		float Saved_BhopMaxWalkSpeed = MAX_WALK_SPEED;
		float Saved_BhopGroundFriction = JUMP_Z_VELOCITY;
		float Saved_BhopJumpZVelocity = GROUND_FRICTION;
	};
	
	/**
	* Struct used for network RPC parameters between client/server by ACharacter and UCharacterMovementComponent.
	* To extend network move data and add custom parameters, you typically override this struct with a custom derived struct and set the CharacterMovementComponent
	* to use your container with UCharacterMovementComponent::SetNetworkMoveDataContainer(). Your derived struct would then typically (in the constructor) replace the
	* NewMoveData, PendingMoveData, and OldMoveData pointers to use your own instances of a struct derived from FCharacterNetworkMoveData, where you add custom fields
	* and implement custom serialization to be able to pack and unpack your own additional data.
	*
	* @see UCharacterMovementComponent::SetNetworkMoveDataContainer()
	*/
	class FBhopCharacterNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
	{
		//typedef FCharacterNetworkMoveDataContainer Super;
		FBhopCharacterNetworkMoveDataContainer();
		FBhopCharacterNetworkMoveData BhopDefaultMoveData[3];
	};





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bhop Character Movement Component Stuff																															 						 //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	/** Get prediction data for a client game. Should not be used if not running as a client. Allocates the data on demand and can be overridden to allocate a custom override if desired. Result must be a FNetworkPredictionData_Client_Character. */
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	/** Called after MovementMode has changed. Base implementation does special handling for starting certain modes, then notifies the CharacterOwner. */
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	/* Process a move at the given time stamp, given the compressed flags representing various events that occurred (ie jump). */
	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel) override;


protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;


////////// Additional implementations to the original UCharacterMovement class ////////// 
public:
	// Action functions and stuff
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();
	UFUNCTION(BlueprintCallable) void SetBhopMaxWalkSpeed(float Value);
	UFUNCTION(BlueprintCallable) void SetBhopGroundFriction(float Value);
	UFUNCTION(BlueprintCallable) void SetBhopJumpZVelocity(float Value);

	// Movement variables that are hoisted to the network
	bool Safe_bWantsToSprnt = false;
	float Safe_BhopMaxWalkSpeed = MAX_WALK_SPEED;
	float Safe_BhopGroundFriction = GROUND_FRICTION;
	float Safe_BhopJumpZVelocity = JUMP_Z_VELOCITY;

	// Defaults configuration for the component
	UPROPERTY() float DefaultMaxWalkSpeed = MAX_WALK_SPEED;
	UPROPERTY() float DefaultMaxSprintSpeed = MAX_WALK_SPEED * 2;
	UPROPERTY() float DefaultGroundFriction = GROUND_FRICTION;
	UPROPERTY() float DefaultJumpZVelocity = JUMP_Z_VELOCITY;


};
