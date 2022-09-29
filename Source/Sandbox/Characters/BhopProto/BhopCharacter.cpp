// Fill out your copyright notice in the Description page of Project Settings.


#include "BhopCharacter.h"

// Essentials
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Misc/App.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Components 
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
//#include "Components/WidgetComponent.h"

// Types
#include "AI/Navigation/NavigationTypes.h"
#include "UObject/Class.h"

#pragma region Constructors
ABhopCharacter::ABhopCharacter()
{
 	// Base configuration and initialization of components
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Create the camera arm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh()); // Attach this to the mesh because if we attach this to the root, whenever we crouch the springArm/Camera will move along with it, which is not intended
	CameraBoom->TargetArmLength = 200; // Distance from the character
	CameraBoom->SocketOffset = FVector(0, 74, 74); // Align the camera to the side of the character
	CameraBoom->bUsePawnControlRotation = true; // Allows us to rotate the camera boom along with our controller when we're adding mouse input

	CharacterCam = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	CharacterCam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attaches the camera to the camera's spring arm socket
	CharacterCam->bUsePawnControlRotation = false; // The follow camera should use the pawn control rotation as it's attached to the camera boom

	// Player collision
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // To stop the capsule component from colliding with our camera
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // same here
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	//GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh); // For when you create your own collision channels

	//////////////////////// Configure the character movement ////////////////////////
	// Movement component configuration (configure the movement (the movement for the character (the character's movement)))
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of the movement input
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f); // I think this is nulled out by our acceleration forces
	bUseControllerRotationYaw = true; // This off with OrientRotationMovement on lets the character rotation be independent from walking direction (keep it on for bhopping) -> set it to false when standing still perhaps

	#pragma region Get character movement compendium
	// CharacterMovement (General Settings)
	GetCharacterMovement()->GravityScale = 2.8f; // pertains to bhop
	GetCharacterMovement()->MaxAcceleration = 6009.f; // pertains to bhop
	GetCharacterMovement()->BrakingFrictionFactor = 2.f;
	GetCharacterMovement()->BrakingFriction = 0.f;
	GetCharacterMovement()->bUseSeparateBrakingFriction = false;
	GetCharacterMovement()->Mass = 100.f;
	GetCharacterMovement()->DefaultLandMovementMode = EMovementMode::MOVE_Walking;
	GetCharacterMovement()->DefaultWaterMovementMode = EMovementMode::MOVE_Swimming;

	// Character Movement: Walking
	GetCharacterMovement()->MaxStepHeight = 45.f;
	GetCharacterMovement()->SetWalkableFloorAngle(44.765309);
	GetCharacterMovement()->SetWalkableFloorZ(0.71);
	GetCharacterMovement()->GroundFriction = 8.f;
	GetCharacterMovement()->MaxWalkSpeed = 800.f; // pertains to bhop
	GetCharacterMovement()->MaxWalkSpeedCrouched = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 0.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 200.f; // pertains to bhop
	GetCharacterMovement()->bSweepWhileNavWalking = true;
	GetCharacterMovement()->bCanWalkOffLedges = true;
	GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = false;
	GetCharacterMovement()->bMaintainHorizontalGroundVelocity = false; // pertains to bhop
	GetCharacterMovement()->bIgnoreBaseRotation = false;
	GetCharacterMovement()->PerchRadiusThreshold = 0.f;
	GetCharacterMovement()->PerchAdditionalHeight = 40.f;
	GetCharacterMovement()->LedgeCheckThreshold = 4.f;
	GetCharacterMovement()->bAlwaysCheckFloor = false;
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = false;

	// Character Movement: Jumping/Falling
	GetCharacterMovement()->JumpZVelocity = 1000.f; // pertains to bhop
	GetCharacterMovement()->BrakingDecelerationFalling = 0.f;
	GetCharacterMovement()->AirControl = 1.f; // pertains to bhop
	GetCharacterMovement()->AirControlBoostMultiplier = 2.f;
	GetCharacterMovement()->AirControlBoostVelocityThreshold = 25.f;
	GetCharacterMovement()->FallingLateralFriction = 0.f;
	GetCharacterMovement()->bImpartBaseVelocityX = true;
	GetCharacterMovement()->bImpartBaseVelocityY = true;
	GetCharacterMovement()->bImpartBaseVelocityZ = true;
	GetCharacterMovement()->bImpartBaseAngularVelocity = true;
	GetCharacterMovement()->bNotifyApex = false;
	GetCharacterMovement()->JumpOffJumpZFactor = 0.5f;
	GetCharacterMovement()->bApplyGravityWhileJumping = true;

	// Character Movement (Networking)
	GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
	GetCharacterMovement()->bNetworkSkipProxyPredictionOnNetUpdate = false;
	GetCharacterMovement()->NetworkMaxSmoothUpdateDistance = 256.f;
	GetCharacterMovement()->NetworkNoSmoothUpdateDistance = 384.f;
	GetCharacterMovement()->NetworkMinTimeBetweenClientAckGoodMoves = 0.1f;
	GetCharacterMovement()->NetworkMinTimeBetweenClientAdjustments = 0.1f;
	GetCharacterMovement()->NetworkMinTimeBetweenClientAdjustmentsLargeCorrection = 0.05f;
	GetCharacterMovement()->NetworkLargeClientCorrectionDistance = 15.f;
	GetCharacterMovement()->bNetworkAlwaysReplicateTransformUpdateTimestamp = false;
	GetCharacterMovement()->NetworkSimulatedSmoothLocationTime = 0.1f;
	GetCharacterMovement()->NetworkSimulatedSmoothRotationTime = 0.05f;
	GetCharacterMovement()->ListenServerNetworkSimulatedSmoothLocationTime = 0.04f;
	GetCharacterMovement()->ListenServerNetworkSimulatedSmoothRotationTime = 0.033f;
	GetCharacterMovement()->NetProxyShrinkRadius = 0.01f;
	GetCharacterMovement()->NetProxyShrinkHalfHeight = 0.01f;

	// Movement Capabilities
	GetCharacterMovement()->NavAgentProps.AgentHeight = 48.f; // pertains to bhop (i thunk) TODO: Fix these (These are not being set properly, do it manualy)
	GetCharacterMovement()->NavAgentProps.AgentRadius = 192.f; // pertains to bhop (i thunk) TODO: Fix these (These are not being set properly, do it manualy)
	GetCharacterMovement()->NavAgentProps.AgentStepHeight = -1.f;
	GetCharacterMovement()->NavAgentProps.NavWalkingSearchHeightScale = 0.5f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->NavAgentProps.bCanJump = true;
	GetCharacterMovement()->NavAgentProps.bCanWalk = true;
	GetCharacterMovement()->NavAgentProps.bCanSwim = true;
	GetCharacterMovement()->NavAgentProps.bCanFly = false;

	// Movement Component
	GetCharacterMovement()->bUpdateOnlyIfRendered = false;
	GetCharacterMovement()->bAutoUpdateTickRegistration = true;
	GetCharacterMovement()->bTickBeforeOwner = true;
	GetCharacterMovement()->bAutoRegisterUpdatedComponent = true;
	GetCharacterMovement()->bAutoRegisterPhysicsVolumeUpdates = true;
	GetCharacterMovement()->bComponentShouldUpdatePhysicsVolume = true;
	#pragma endregion
	
	// Configure the bhop defaults (the bhopping configuration (the configuration for bhopping (bhopping configuration))) 
	DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	DefaultBraking = GetCharacterMovement()->BrakingDecelerationWalking;
	DefaultFriction = GetCharacterMovement()->GroundFriction;
	DefaultJumpVelocity = GetCharacterMovement()->JumpZVelocity;

	//////////////////////// Replication stuff (Server/Client rendering) ////////////////////////
	NetUpdateFrequency = 66.f; // default update character on other machines 66 times a second (general fps defaults)
	MinNetUpdateFrequency = 33.f; // To help with bandwidth and lagginess, allow a minNetUpdateFrequency, which is generally 33 in fps games
	// The other important value is the server config tick rate, which is in the project defaultEngine.ini -> [/Script/OnlineSubsystemUtils.IpNetDriver] NetServerMaxTickRate = 60
}


void ABhopCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Base Movements
	PlayerInputComponent->BindAxis("MoveForward", this, &ABhopCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABhopCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABhopCharacter::Turn);
	PlayerInputComponent->BindAxis("Lookup", this, &ABhopCharacter::Lookup);

	// Auxillery movements
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABhopCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ABhopCharacter::StopJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ABhopCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ABhopCharacter::StopSprint);

	// Actions
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABhopCharacter::EquipButtonPress);
	PlayerInputComponent->BindAction("UnEquip", IE_Released, this, &ABhopCharacter::UnEquipButtonPress);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABhopCharacter::CrouchButtonPressed);
}


void ABhopCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); // Net/UnrealNetwork is required to declare rep lifetimes

}
#pragma endregion


#pragma region UE Main Functions
void ABhopCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}


void ABhopCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// This function is the earliest you can access an actor component from the character clas
}


void ABhopCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//// Bhop logic //// Calculate frameTime (DeltaTime), PrevVelocity, and the XYspeedometer
	FrameTime = DeltaTime;
	PrevVelocity = GetVelocity();
	XYspeedometer = PrevVelocity.Length();

}
#pragma endregion


#pragma region bhop (movement) logic
void ABhopCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
	// This is being completely overrided because it's doing the broadcast call before we implement our own physics.

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Super OnMonkeyModeChange Logic																	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	CachedCharacterMovement = GetCharacterMovement();
	if (CachedCharacterMovement == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ERROR: Character movement component not found, exiting OnMovementModeChanged!"));
		return;
	}
	const EMovementMode NewMovementMode = CachedCharacterMovement->MovementMode;

	// Original Functionality
	if (!bPressedJump || !CachedCharacterMovement->IsFalling())
	{
		ResetJumpState();
	}

	// Record jump force start time for proxies. Allows us to expire the jump even if not continually ticking down a timer.
	if (bProxyIsJumpForceApplied && CachedCharacterMovement->IsFalling())
	{
		ProxyJumpForceStartedTime = GetWorld()->GetTimeSeconds();
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//// Bunny hoppin functionality																	  ////
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Debugging (print the movement mode information)
	const UEnum* MovementModeEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EMovementMode"));
	UE_LOG(LogTemp, Warning, TEXT("CharacterName: %s, Movement mode: %s, Prev Monke mode: %s")
		, *GetNameSafe(this)
		, *(MovementModeEnum ? MovementModeEnum->GetNameStringByIndex(NewMovementMode) : TEXT("<Invalid Enum>"))
		, *(MovementModeEnum ? MovementModeEnum->GetNameStringByIndex(PrevMovementMode) : TEXT("<Invalid Enum>")));


	// if we're in the Walking state
	if (NewMovementMode == EMovementMode::MOVE_Walking)
	{
		// Is the landing sound not on cooldown?
		if ( !(LandingSoundCooldownTotal > UKismetSystemLibrary::GetGameTimeInSeconds(this)) )
		{
			// Calculate the impact speed for which sound to play (the leg crack or the land hop)
			if (GetCharacterMovement())
			{
				// We need to get the Z velocity when we are walking up ramps. Regular get velocity function shows 0 when walking so cannot be used.
				// TODO: Implement condition bhop sound based on z velocity impulse

				if (LegBonkSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						LegBonkSound,
						GetActorLocation()
					);
				}
			}
		}

		// Set the land sound cooldown variable to be up to date with the game time
		LandingSoundCooldownTotal = LandSoundCooldown + UKismetSystemLibrary::GetGameTimeInSeconds(this);

		// bhop easy mode (good for ue debugging)
		if (bEnablePogo && bPressedJump)
		{
			// TODO: BunnyHopAndTrimpLogic
		}
		else
		{
			// Create a delay for pogo reseting the friction 
			FLatentActionInfo StuffTodoOnComplete; // https://www.reddit.com/r/unrealengine/comments/b1t1d8/how_to_wait_for/
			StuffTodoOnComplete.CallbackTarget = this;
			StuffTodoOnComplete.ExecutionFunction = FName("ResetFrictionDelay");
			StuffTodoOnComplete.Linkage = 0;
			StuffTodoOnComplete.UUID = NumberOfTimesPogoResetFrictionUUID;
			UKismetSystemLibrary::Delay(GetWorld(), FrameTime * 2.f, StuffTodoOnComplete);
		}
	}


	// Bhop/Ramp/Trimp logic (trimp is when they fly up through the air after hopping on an edge at an angle)


	// Broadcast this to replicate the movement! (This is the final piece of the original function
	MovementModeChangedDelegate.Broadcast(this, PrevMovementMode, PrevCustomMode);
}


void ABhopCharacter::BhopAndTrimpLogic()
{
	RampCheck();
	ApplyTrimp();

	// Trimp replication

	// play sound if jump sound is not on cooldown

	// apply bunn hop cap and it's replication
}

#pragma region Apply Trimp
void ABhopCharacter::ApplyTrimp()
{
	// Indicates down sloping ramp
	if (RampCheckGroundAngleDotproduct > 0.05f) // Downward trimp logic
	{
		// limit the amount of vertical reduction when jumping down ramps to ensure we can always jump
		float ReduceJumpHeightZ = FMath::Clamp(
			(RampCheckGroundAngleDotproduct * (-1.f / TrimpDownMultiplier) * XYspeedometer),
			TrimpDownVertCap * DefaultJumpVelocity * -1,
			0.f
		);
		FVector ReduceJumpHeight = FVector(0.f, 0.f, ReduceJumpHeightZ);

		// Combine the vertical and lateral impulses
		TrimpImpulse = UKismetMathLibrary::Add_VectorVector(
			UKismetMathLibrary::Multiply_VectorFloat(PrevVelocity, RampCheckGroundAngleDotproduct * TrimpDownMultiplier), // Add lateral speed
			ReduceJumpHeight
		);
	}
	else if (RampCheckGroundAngleDotproduct < -0.05f) // Upward trimp logic
	{
		// determine how much lateral speed to reduce when jumping up ramp
		FVector ReduceJumpHeight = FVector(0.f, 0.f, RampCheckGroundAngleDotproduct * TrimpUpMultiplier * XYspeedometer);

		TrimpImpulse = UKismetMathLibrary::Add_VectorVector(
			UKismetMathLibrary::Multiply_VectorFloat(PrevVelocity, TrimpUpLateralSlow * RampCheckGroundAngleDotproduct),
			ReduceJumpHeight
		);
	}
	else // Neutral trimp logic
	{
		TrimpImpulse = FVector::Zero();
	}

	// Set trimp lateral impulse
	TrimpLateralImpulse = FVector(TrimpImpulse.X, TrimpImpulse.Y, 0.f).Length() + XYspeedometer;
	
	// Set trimp jump impulse
	TrimpJumpImpulse = TrimpImpulse.Z + DefaultJumpVelocity;
}


void ABhopCharacter::ApplyTrimpReplication()
{
	if (HasAuthority())
	{
		HandleApplyTrimpReplication();
	}
	else
	{
		ServerApplyTrimpReplication();
	}
}


void ABhopCharacter::ServerApplyTrimpReplication_Implementation()
{
	HandleApplyTrimpReplication();
}


void ABhopCharacter::HandleApplyTrimpReplication()
{

}
#pragma endregion


#pragma region Bhop Cap
void ABhopCharacter::BhopCap()
{
}


void ABhopCharacter::BhopCapReplication()
{
}


void ABhopCharacter::ServerBhopCapReplication_Implementation()
{
}


void ABhopCharacter::HandleBhopCapReplication()
{
}
#pragma endregion


#pragma region Reset Friction
void ABhopCharacter::ResetFriction()
{
	// I wonder if this causes lagginess when the client and server implementations don't align (This makes it so the server runs the same code for keeping track of the client's movements
	// and it drops the movement functionality specific to that character everywhere else). So only the server keeps track of the details while the character's replicated (rpcs) movements are rendered across all clients
	// If there's lagginess maybe creating a multicast that the server calls on this is the go forward decision. later edit (OnMovementChanged has a broadcast event that multicasts to prevent this (I thunk))
	if (HasAuthority()) // On Server
	{
		HandleResetFriction();
	}
	else
	{
		ServerResetFriction();
	}
}


void ABhopCharacter::ServerResetFriction_Implementation()
{
	HandleResetFriction();
}


void ABhopCharacter::HandleResetFriction()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed;
		GetCharacterMovement()->GroundFriction = DefaultFriction;
		GetCharacterMovement()->BrakingDecelerationWalking = DefaultBraking;
		GetCharacterMovement()->MovementMode;
	}
}


// Delay function for OnMovementModeChanged function
void ABhopCharacter::ResetFrictionDelay()
{
	ResetFriction();
	NumberOfTimesPogoResetFrictionUUID++;
}
#pragma endregion


#pragma region Remove Friction
void ABhopCharacter::RemoveFriction()
{
	// Same as what's said in ResetFriction
	if (HasAuthority()) // On Server
	{
		HandleRemoveFriction();
	}
	else
	{
		ServerRemoveFriction();
	}
}


void ABhopCharacter::ServerRemoveFriction_Implementation()
{
	HandleRemoveFriction();
}


void ABhopCharacter::HandleRemoveFriction()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GroundFriction = 0.f;
		GetCharacterMovement()->BrakingDecelerationWalking = 0.f;
	}
}
#pragma endregion


#pragma region Ram Stuff
// Slide on ramp if passed the min ramp slide speed AND on a slideable ramp (found through RampCheck)
bool ABhopCharacter::RamSlide()
{
	if (XYspeedometer > (DefaultMaxWalkSpeed * RampslideThresholdFactor) && RampCheck()) return true;
	return false;
}


// Check if the surface (ramp) is a slideable ramp (surface angle < 90 degress)
bool ABhopCharacter::RampCheck()
{
	UWorld* World = GetWorld();
	if (World)
	{
		// Trace a line from the actor to the ground
		FHitResult BreakHitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);
		World->LineTraceSingleByChannel(
			BreakHitResult,
			GetActorLocation(),
			GetActorLocation() - UKismetMathLibrary::Multiply_VectorFloat(GetActorUpVector(), 100.f),
			ECollisionChannel::ECC_Visibility,
			CollisionParams
		);

		// get the normalized vectors of the previous and current directions we're traveling on the xy plane to find out whether we're sloping up or down a hill
		RampCheckGroundAngleDotproduct = FVector::DotProduct(BreakHitResult.ImpactNormal, PrevVelocity.GetSafeNormal(0.0001));
		float AngleOfRamp = UKismetMathLibrary::DegAcos(RampCheckGroundAngleDotproduct);
		UE_LOG(LogTemp, Warning, TEXT("RampCheck::GroundAngleDotproduct: %f, angle of ramp: %f \n"), RampCheckGroundAngleDotproduct, AngleOfRamp);

		// Check if the angle is greater than 2 degrees (90 is a flat surface)
		if (AngleOfRamp > 92.f) return true;
	}

	return false;
}
#pragma endregion


#pragma region Add Ramp Momentum
// A jump force is needed to prevent sticking to the ground when exiting a rampslide
void ABhopCharacter::AddRampMomentum()
{
	if (HasAuthority())
	{
		HandleAddRampMomentum();
	}
	else 
	{
		ServerAddRampMomentum();
	}
}


void ABhopCharacter::ServerAddRampMomentum_Implementation()
{
	HandleAddRampMomentum();
}


void ABhopCharacter::HandleAddRampMomentum()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->JumpZVelocity = DefaultJumpVelocity * RampMomentumFactor;
		Jump();
	}
}
#pragma endregion


#pragma region Ground Acceleration
// Apply Ground acceleration when turning
void ABhopCharacter::AccelerateGround()
{
	// normalized vector indicating the desired movement direction based on the currently pressed keys
	FVector RawInputDirection = UKismetMathLibrary::Multiply_VectorFloat(InputForwardVector, InputForwardAxis) + UKismetMathLibrary::Multiply_VectorFloat(InputSideVector, InputSideAxis);
	InputDirection = RawInputDirection.GetSafeNormal(0.0001f); 

	// Takes the projection of the current velocity along the input direction - this is used to allow some acceleration to take place when turning in the same direction of strafe
	float ProjectedVelocity = FVector::DotProduct(FVector(GetVelocity().X, GetVelocity().Y, 0.f), InputDirection);

	// Take the length of our input vector (desired input speed)
	float InputSpeed = UKismetMathLibrary::Multiply_VectorFloat(InputDirection, DefaultMaxWalkSpeed).Length();

	// When a movement key is pressed we subtract the velocity projection from the accel speed cap to set a max acceleration value
	float MaxAccelSpeed = InputSpeed - ProjectedVelocity;

	// Applying acceleration?
	if (MaxAccelSpeed > 0.f)
	{
		bApplyingGroundAccel = true;
		float Accelspeed = FMath::Clamp((FrameTime * InputSpeed * GroundAccelerate), 0.f, MaxAccelSpeed);

		// Calculate the impulse we will apply for acceleration
		FVector ImpulseVector = UKismetMathLibrary::Multiply_VectorFloat(InputDirection, Accelspeed);
		GroundAccelDir = InputDirection;

		// Determine what new maxWalkSpeed should be to allow acceleration impulses to take effect
		// Also the clamp is to prevent the default maxWalkSpeed from being set less than the normal walking speed
		CalcMaxWalkSpeed = FMath::Clamp(UKismetMathLibrary::Add_VectorVector(PrevVelocity, ImpulseVector).Length(), DefaultMaxWalkSpeed, MaxSeaDemonSpeed);
	}
	else
	{
		bApplyingGroundAccel = false;
	}
}


// Replicate that applied ground acceleration
void ABhopCharacter::AccelerateGroundReplication()
{
	if (HasAuthority())
	{
		if (bApplyingGroundAccel || UKismetSystemLibrary::IsDedicatedServer(this)) HandleAccelerateGroundReplication();
	}
	else if (bApplyingGroundAccel)
	{
		ServerAccelerateGroundReplication();
	}
}


void ABhopCharacter::ServerAccelerateGroundReplication_Implementation()
{
	HandleAccelerateGroundReplication();
}


void ABhopCharacter::HandleAccelerateGroundReplication()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = CalcMaxWalkSpeed;
		AddMovementInput(GroundAccelDir); // add movement input node must be used for proper multiplayer replication as it utilizes predictionand network history.
	}
}
#pragma endregion


#pragma region AirAcceleration
void ABhopCharacter::AccelerateAir()
{
	// normalized vector indicating the desired movement direction based on the currently pressed keys
	FVector RawInputDirection = UKismetMathLibrary::Multiply_VectorFloat(InputForwardVector, InputForwardAxis) + UKismetMathLibrary::Multiply_VectorFloat(InputSideVector, InputSideAxis);
	InputDirection = RawInputDirection.GetSafeNormal(0.0001f);

	// Takes the projection of the current velocity along the input direction - this is used to allow some acceleration to take place when turning in the same direction of strafe
	float ProjectedVelocity = FVector::DotProduct(GetVelocity(), InputDirection);

	// Take the length of our input vector (desired input speed)
	float InputSpeed = UKismetMathLibrary::Multiply_VectorFloat(InputDirection, DefaultMaxWalkSpeed).Length();

	// Cap the amount of acceleration in air from a standstill when under the cap. Without this cap you will accelerate from the standstill up to the max walkspeed
	float AccelSpeedCap = 80.f;

	// When a movement key is pressed we subtract the velocity projection from the accel speed cap to set a max acceleration value
	float MaxAccelSpeed = FMath::Clamp(InputSpeed, 0.f, AccelSpeedCap) - ProjectedVelocity;

	// Applying acceleration?
	if (MaxAccelSpeed > 0.f)
	{
		bApplyingAirAccel = true;
		float Accelspeed = FMath::Clamp((FrameTime * InputSpeed * AirAccelerate), 0.f, MaxAccelSpeed);

		// Calculate the impulse we will apply for acceleration
		FVector ImpulseVector = UKismetMathLibrary::Multiply_VectorFloat(InputDirection, Accelspeed);
		AirAccelDir = InputDirection;

		// prevent the default maxWalkSpeed from being set less than the normal walking speed
		CalcMaxAirSpeed = FMath::Clamp(ImpulseVector.Length() + PrevVelocity.Length(), DefaultMaxWalkSpeed, MaxSeaDemonSpeed);
	}
	else
	{
		bApplyingAirAccel = false;
	}
}


void ABhopCharacter::AccelerateAirReplication()
{
	if (HasAuthority())
	{
		if (bApplyingAirAccel || UKismetSystemLibrary::IsDedicatedServer(this)) HandleAccelerateAirReplication();
	}
	else if (bApplyingAirAccel)
	{
		ServerAccelerateGroundReplication();
	}
}


void ABhopCharacter::ServerAccelerateAirReplication_Implementation()
{
	HandleAccelerateAirReplication();
}


void ABhopCharacter::HandleAccelerateAirReplication()
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = CalcMaxAirSpeed;
		AddMovementInput(AirAccelDir); // add movement input node must be used for proper multiplayer replication as it utilizes predictionand network history.
	}
}
#pragma endregion


#pragma region Movement Logic
void ABhopCharacter::HandleMovement()
{
	if (!GetWorld()) return;

	if (CachedCharacterMovement->IsFalling()) // In air
	{
		RemoveFriction();
		bIsRampSliding = false; 

		// Is custom air acceleration enabled?
		if (bEnableCustomAirAccel)
		{
			AccelerateAir();
			AccelerateAirReplication();
		}
		else
		{
			BaseMovementLogic();
		}
	}
	else // On the ground like a scrub
	{
		bool bOnRampSlide = RamSlide();

		// Is the character rampsliding?
		if (bOnRampSlide)
		{
			RemoveFriction(); // When rampsliding we want to remove friction and allow "air control" for steering
			bIsRampSliding = true; // This is set after rampsliding decision to understand when we EXIT a rampslide

			AccelerateAir();
			AccelerateAirReplication();
		}
		else
		{
			// In the case that we just got out of rampsliding, then we add a momentum force to prevent stickiness when exiting the rampslide
			if (bIsRampSliding)
			{
				AddRampMomentum();

				// Then create a delay before applying the friction again
				FLatentActionInfo StuffTodoOnComplete; // https://www.reddit.com/r/unrealengine/comments/b1t1d8/how_to_wait_for/
				StuffTodoOnComplete.CallbackTarget = this;
				StuffTodoOnComplete.ExecutionFunction = FName("MovementDelayLogic");
				StuffTodoOnComplete.Linkage = 0;
				StuffTodoOnComplete.UUID = NumberOfTimesRampSlided;

				UKismetSystemLibrary::Delay(GetWorld(), FrameTime * 2.f, StuffTodoOnComplete);
			}
			else // If we weren't rampsliding, then handle the base movement logic
			{
				if (bEnableGroundAccel)
				{
					BaseMovementLogic();
				}
			}
		}
	}
}


void ABhopCharacter::BaseMovementLogic()
{
	// Apply ground acceleration when turning (and replicate it)
	if (bEnableGroundAccel)
	{
		AccelerateGround();
		AccelerateGroundReplication();
	}
	else // Apply the standard movement logic
	{
		AddMovementInput(InputForwardVector, InputForwardAxis);
		AddMovementInput(InputSideVector, InputSideAxis); // add movement input node must be used for proper multiplayer replication as it utilizes predictionand network history.
	}
}


void ABhopCharacter::MovementDelayLogic()
{
	UE_LOG(LogTemp, Warning, TEXT("Calling the movement delay logic function!"));

	// Reset the friction and set wasRampSliding to false
	bIsRampSliding = false;
	ResetFriction();
	NumberOfTimesRampSlided++; // For the next time we call the rampslide delay

	// Run the base implementation
	BaseMovementLogic();
}
#pragma endregion


// end of bhop region
#pragma endregion




#pragma region Input Functions
void ABhopCharacter::MoveForward(float Value)
{
	InputForwardAxis = Value;
	InputForwardVector = GetActorForwardVector();

	HandleMovement();
}


void ABhopCharacter::MoveRight(float Value)
{
	InputSideAxis = Value;
	InputSideVector = GetActorRightVector();

	HandleMovement();
}


void ABhopCharacter::StartJump()
{
	bPressedJump = true;
	//if (bIsCrouched) UnCrouch();
	BhopAndTrimpLogic();
}


void ABhopCharacter::StopJump()
{
	bPressedJump = false;
	StopJumping();
}










void ABhopCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}


void ABhopCharacter::Lookup(float Value)
{
	AddControllerPitchInput(Value);
}


void ABhopCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed * 2;
}


void ABhopCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed;
}


void ABhopCharacter::CrouchButtonPressed()
{
	if (bIsCrouched) UnCrouch();
	else Crouch();
}


void ABhopCharacter::EquipButtonPress()
{
	UE_LOG(LogTemp, Warning, TEXT("You pressed the equip button"));
}


void ABhopCharacter::UnEquipButtonPress()
{
	UE_LOG(LogTemp, Warning, TEXT("You pressed the unequip button"));
}
#pragma endregion







#pragma region Random Noteworthy stuff I found for coding and learning how to debug and whatever else tickles my fancy
/* 
Grabbing and printing enums, also how to get the name of teh character
	const UEnum* MovementModeEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EMovementMode"));
	UE_LOG(LogTemp, Warning, TEXT("THIS: %s, Movement mode: %s")
		, *GetNameSafe(this)
		, *(MovementModeEnum ? MovementModeEnum->GetNameStringByIndex(PreviousMovementMode) : TEXT("<Invalid Enum>")));
*/
#pragma endregion
