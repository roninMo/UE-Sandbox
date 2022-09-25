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

// Components 
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
//#include "Components/WidgetComponent.h"

// Types
#include "AI/Navigation/NavigationTypes.h"

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
	bUseControllerRotationYaw = false; // This off with OrientRotationMovement on lets the character rotation be independent from the orient rotation

	#pragma region Get character movement compendium
	// CharacterMovement (General Settings)
	GetCharacterMovement()->GravityScale = 2.8f; // pertains to bhop
	GetCharacterMovement()->MaxAcceleration = 6009.f; // pertains to bhop
	GetCharacterMovement()->BrakingFrictionFactor = 2.f;
	GetCharacterMovement()->BrakingFriction = 0.f;
	GetCharacterMovement()->CrouchedHalfHeight = 40.f;
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

	PlayerInputComponent->BindAxis("MoveForward", this, &ABhopCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABhopCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABhopCharacter::Turn);
	PlayerInputComponent->BindAxis("Lookup", this, &ABhopCharacter::Lookup);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABhopCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ABhopCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ABhopCharacter::StopSprint);

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

	// Bhop logic
	// Calculate frameTime (DeltaTime), PrevVelocity, and the XYspeedometer
	FrameTime = DeltaTime;
	PrevVelocity = GetVelocity();
	XYspeedometer = PrevVelocity.Length();

}
#pragma endregion


#pragma region bhop (movement) logic
// THE
#pragma endregion


#pragma region Input Functions
void ABhopCharacter::MoveTheCharacter(float Value, bool isForward)
{
	if (Controller != nullptr && Value != 0.f) {
		const auto Axis = isForward ? EAxis::X : EAxis::Y;
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f); // The yaw helps us determine the direction
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(Axis)); // This is a function to grab the direction from a specific rotation, in this case we're grabbing the yaw

		FString RotationString = YawRotation.ToCompactString();
		FString DirectionString = Direction.ToCompactString();
		UE_LOG(LogTemp, Warning, TEXT("YawRotation: %s, Direction: %s"), *RotationString, *DirectionString);

		if (GetCharacterMovement()->IsFalling()) // Air
		{

		}
		else // Ground
		{

		}

		// Add a force to the character with the move functions
		AddMovementInput(Direction, Value);
	}
}


void ABhopCharacter::MoveForward(float Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Moving the character forwards?"));
	MoveTheCharacter(Value, true);
}


void ABhopCharacter::MoveRight(float Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Moving the character sideways?"));
	MoveTheCharacter(Value, false);
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