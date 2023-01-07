// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CMCBaseConfiguration.generated.h"




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


// Error handling
// "p.NetShowCorrections 1"
// "emulationPkLag 500"


// Defining the base movespeeds here because they're referenced across multiple classes
#define MAX_WALK_SPEED 840.f
#define JUMP_Z_VELOCITY 969.f
#define GROUND_FRICTION 8.f


/**
 * 
 */
UCLASS()
class SANDBOX_API UCMCBaseConfiguration : public UCharacterMovementComponent
{
	GENERATED_BODY()
	

public:
	//////////////////////////////////////////////
	// Custom FSavedMove (Bhop Implementation)	//
	//////////////////////////////////////////////
	class CMCB_FSavedMove_Character : public FSavedMove_Character
	{
	public:
		typedef FSavedMove_Character Super;

		// Other values values we want to pass into the saved moves
		uint8 Saved_bWantsToSprnt : 1;

		// Functions 
		/** Returns true if this move can be combined with NewMove for replication without changing any behavior */
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

		/** Combine this move with an older move and update relevant state. */
		//virtual void CombineWith(const FSavedMove_Character* OldMove, ACharacter* InCharacter, APlayerController* PC, const FVector& OldStartLocation) override;

		/** Clear saved move properties, so it can be re-used. */
		virtual void Clear() override;

		/** Returns a byte containing encoded special movement information (jumping, crouching, etc.)	 */
		virtual uint8 GetCompressedFlags() const override;

		/** Called to set up this saved move (when initially created) to make a predictive correction. */
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

		/** Called before ClientUpdatePosition uses this SavedMove to make a predictive correction	 */
		virtual void PrepMoveFor(ACharacter* Character) override;
	};



	//////////////////////////////////////////////////////////////////
	// Custom FNetworkPredictionData_Client (Bhop Implementation)	//
	//////////////////////////////////////////////////////////////////
	// Indicate to the cmc that we're using our custom move FSavedMove_Bhop
	class CMCB_FNetworkPredictionData_Client_Character : public FNetworkPredictionData_Client_Character
	{
	public:
		typedef FNetworkPredictionData_Client_Character Super;
		CMCB_FNetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement);
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
	class CMCB_FCharacterNetworkMoveData : public FCharacterNetworkMoveData
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
	class CMCB_CharacterNetworkMoveDataContainer : public FCharacterNetworkMoveDataContainer
	{
		//typedef FCharacterNetworkMoveDataContainer Super;
		CMCB_CharacterNetworkMoveDataContainer();
		CMCB_FCharacterNetworkMoveData CMCBDefaultMoveData[3];
	};





	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Bhop Character Movement Component Stuff																															 						 //
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	UCMCBaseConfiguration(); // Constructor

	/** Get prediction data for a client game. Should not be used if not running as a client. Allocates the data on demand and can be overridden to allocate a custom override if desired. Result must be a FNetworkPredictionData_Client_Character. */
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	/** Called after MovementMode has changed. Base implementation does special handling for starting certain modes, then notifies the CharacterOwner. */
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	/* Process a move at the given time stamp, given the compressed flags representing various events that occurred (ie jump). */
	virtual void MoveAutonomous(float ClientTimeStamp, float DeltaTime, uint8 CompressedFlags, const FVector& NewAccel) override;

	/* Get's the max speed base on the movement mode you're in */
	virtual float GetMaxSpeed() const override;


protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;


	////////// Additional implementations to the original UCharacterMovement class ////////// 
public:
	// Action functions and stuff
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();

	// Movement variables that are hoisted to the network
	bool Safe_bWantsToSprnt = false;

	// Defaults configuration for the component
	UPROPERTY() float DefaultMaxWalkSpeed = MAX_WALK_SPEED;
	UPROPERTY() float DefaultMaxSprintSpeed = MAX_WALK_SPEED * 2;

};
