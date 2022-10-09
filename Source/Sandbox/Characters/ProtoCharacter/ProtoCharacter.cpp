// Fill out your copyright notice in the Description page of Project Settings.


#include "ProtoCharacter.h"

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
#include "Components/PrimitiveComponent.h"

// Types
#include "AI/Navigation/NavigationTypes.h"
#include "UObject/Class.h"



#pragma region Constructors
AProtoCharacter::AProtoCharacter()
{
	// Base configuration and initialization of components
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Create the camera arm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh()); // Attach this to the mesh because if we attach this to the root, whenever we crouch the springArm/Camera will move along with it, which is not intended
	CameraBoom->TargetArmLength = 200; // Distance from the character
	CameraBoom->SocketOffset = FVector(0, 74, 74); // Align the camera to the side of the character
	CameraBoom->bUsePawnControlRotation = true; // Allows us to rotate the camera boom along with our controller when we're adding mouse input'
	CameraBoom->AddLocalOffset(FVector(0, 0, 100));

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
	GetCharacterMovement()->AirControl = 100.f; // pertains to bhop
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

	DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	
	//////////////////////// Replication stuff (Server/Client rendering) ////////////////////////
	NetUpdateFrequency = 66.f; // default update character on other machines 66 times a second (general fps defaults)
	MinNetUpdateFrequency = 33.f; // To help with bandwidth and lagginess, allow a minNetUpdateFrequency, which is generally 33 in fps games
	// The other important value is the server config tick rate, which is in the project defaultEngine.ini -> [/Script/OnlineSubsystemUtils.IpNetDriver] NetServerMaxTickRate = 60
}


void AProtoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Base Movements
	PlayerInputComponent->BindAxis("MoveForward", this, &AProtoCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProtoCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AProtoCharacter::Turn);
	PlayerInputComponent->BindAxis("Lookup", this, &AProtoCharacter::Lookup);

	// Auxillery movements
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AProtoCharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AProtoCharacter::StopJump);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AProtoCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AProtoCharacter::StopSprint);

	// Actions
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AProtoCharacter::EquipButtonPress);
	PlayerInputComponent->BindAction("UnEquip", IE_Released, this, &AProtoCharacter::UnEquipButtonPress);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AProtoCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &AProtoCharacter::LockOn);
}


void AProtoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); // Net/UnrealNetwork is required to declare rep lifetimes
}
#pragma endregion


#pragma region UE Main Functions
void AProtoCharacter::BeginPlay()
{
	Super::BeginPlay();

}


void AProtoCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// This function is the earliest you can access an actor component from the character clas
}


void AProtoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("Time: %f, Tick::PrevVel: %s"), UKismetSystemLibrary::GetGameTimeInSeconds(this), *PrevVelocity.ToCompactString());
}
#pragma endregion


#pragma region Base Movement Functions
void AProtoCharacter::ServerHandleMovementReplication_Implementation(FVector Direction, float Value)
{
	ClientHandleMovementReplication(Direction, Value);
}


void AProtoCharacter::ClientHandleMovementReplication_Implementation(FVector Direction, float Value)
{
	UE_LOG(LogTemp, Warning, TEXT("CalcDirection::Air: %s, MaxWalkSpeed: %f"), *Direction.ToCompactString(), GetCharacterMovement()->MaxWalkSpeed);
	AddMovementInput(Direction, Value);
}


void AProtoCharacter::HandleMovement(float Value, EAxis::Type Axis)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Axis);
		//AddMovementInput(Direction, Value);

		// Replication
		if (HasAuthority())
		{
			ServerHandleMovementReplication(Direction, Value);
		}
		else
		{
			ClientHandleMovementReplication(Direction, Value);
		}
	}
}
#pragma endregion


#pragma region Input Functions
void AProtoCharacter::MoveForward(float Value)
{
	HandleMovement(Value, EAxis::X);
}


void AProtoCharacter::MoveRight(float Value)
{
	HandleMovement(Value, EAxis::Y);
}


void AProtoCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}


void AProtoCharacter::Lookup(float Value)
{
	AddControllerPitchInput(Value);
}


void AProtoCharacter::StartJump()
{
	Jump();
}


void AProtoCharacter::StopJump()
{
	StopJumping();
}


void AProtoCharacter::StartSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed * 2;
}


void AProtoCharacter::StopSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed;
}


void AProtoCharacter::CrouchButtonPressed()
{
	if (!bIsCrouched) Crouch();
	else UnCrouch();
}


void AProtoCharacter::EquipButtonPress()
{
	UE_LOG(LogTemp, Warning, TEXT("Equip button pressed!"));
}


void AProtoCharacter::UnEquipButtonPress()
{
	UE_LOG(LogTemp, Warning, TEXT("Equip button released!"));
}


void AProtoCharacter::LockOn()
{
	UE_LOG(LogTemp, Warning, TEXT("LockOn button pressed!"));
}
#pragma endregion

