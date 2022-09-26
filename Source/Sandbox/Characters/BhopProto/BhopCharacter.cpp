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
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 850.0f, 0.0f); // ...at this rotation rate
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
	GetCharacterMovement()->NetworkSimulatedSmoothLocationTime = 0.04f;
	GetCharacterMovement()->NetworkSimulatedSmoothRotationTime = 0.033f;
	GetCharacterMovement()->NetProxyShrinkRadius = 0.01f;
	GetCharacterMovement()->NetProxyShrinkHalfHeight = 0.01f;

	// Movement Capabilities
	GetCharacterMovement()->NavAgentProps.AgentHeight = 48.f; // pertains to bhop (i thunk)
	GetCharacterMovement()->NavAgentProps.AgentRadius = 192.f; // pertains to bhop (i thunk)
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
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABhopCharacter::Jump);
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
void ABhopCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	// This is being completely overrided because it's doing the broadcast call before we implement our own physics.
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

	/*
	** //// Bunny hoppin functionality ////
	*/
	const UEnum* MovementModeEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EMovementMode"));
	UE_LOG(LogTemp, Warning, TEXT("CharacterName: %s, Movement mode: %s, Prev Monke mode: %s")
		, *GetNameSafe(this)
		, *(MovementModeEnum ? MovementModeEnum->GetNameStringByIndex(NewMovementMode) : TEXT("<Invalid Enum>"))
		, *(MovementModeEnum ? MovementModeEnum->GetNameStringByIndex(PrevMovementMode) : TEXT("<Invalid Enum>")));


	// Walking state

	// If the character just landed, calculate the force from the speed and direction, and apply a sound for whether they bopped the ground or cracked their legs on the dang ol cement based on the impact speed

	// Create a time window upon landing before friction is applied for bhopping (blueprint configurable)

	// Bhop/Ramp/Trimp logic (trimp is when they fly up through the air after hopping on an edge at an angle)
}


#pragma region Reset Friction
void ABhopCharacter::ServerResetFriction_Implementation()
{
	HandleResetFriction();
}


void ABhopCharacter::ResetFriction()
{
	// I wonder if this causes lagginess when the client and server implementations don't align (This makes it so the server runs the same code for keeping track of the client's movements
	// and it drops the movement functionality specific to that character everywhere else). So only the server keeps track of the details while the character's replicated (rpcs) movements are rendered across all clients
	// If there's lagginess maybe creating a multicast that the server calls on this is the go forward decision. later edit (OnMovementChanged has a broadcast event that multicasts to prevent this (I thunk))
	if (HasAuthority()) // On Server
	{
		ServerResetFriction();
	}
	else
	{
		HandleResetFriction();
	}
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
#pragma endregion


#pragma region Remove Friction
void ABhopCharacter::RemoveFriction()
{
	// Same as what's said in ResetFriction
	if (HasAuthority()) // On Server
	{
		ServerRemoveFriction();
	}
	else
	{
		HandleRemoveFriction();
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
void ABhopCharacter::RamSlide()
{

}

void ABhopCharacter::RampCheck()
{
	UWorld* World = GetWorld();
	if (World)
	{
		FHitResult BreakHitResult;

		World->LineTraceSingleByChannel(
			BreakHitResult,
			GetActorLocation(),
			GetActorLocation() - GetActorUpVector(),
			ECollisionChannel::ECC_Visibility
		);
	}
}
#pragma endregion



#pragma region Add Ramp Momentum
void ABhopCharacter::AddRampMomentum()
{
	if (HasAuthority())
	{
		ServerAddRampMomentum();
	}
	else 
	{
		HandleAddRampMomentum();
	}
}


void ABhopCharacter::ServerAddRampMomentum_Implementation()
{
	HandleAddRampMomentum();
}


void ABhopCharacter::HandleAddRampMomentum()
{
	// A jump force is needed to prevent sticking to the ground when exiting a rampslide
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->JumpZVelocity = DefaultJumpVelocity * RampMomentumFactor;
		Jump();
	}
}
#pragma endregion


// end of bhop region
#pragma endregion


#pragma region Input Functions
void ABhopCharacter::HandleMovement()
{
	if (CachedCharacterMovement->IsFalling()) // In air
	{
		RemoveFriction();
	}
	else // On the ground like a scrub
	{

	}
}


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












void ABhopCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}


void ABhopCharacter::Lookup(float Value)
{
	AddControllerPitchInput(Value);
}


void ABhopCharacter::Jump()
{
	if (bIsCrouched) UnCrouch();
	Super::Jump();
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
