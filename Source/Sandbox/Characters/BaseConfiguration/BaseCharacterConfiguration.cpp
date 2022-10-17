// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacterConfiguration.h"

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

// Gameplay Ability System plugin
#include "Sandbox/GAS/ProtoASC.h"
#include "Sandbox/GAS/ProtoAttributeSet.h"
#include "Sandbox/GAS/ProtoGasGameplayAbility.h"

// Bhop Character Movement Component
#include "CMCBaseConfiguration.h"


#pragma region Constructors
ABaseCharacterConfiguration::ABaseCharacterConfiguration(const FObjectInitializer& ObjectInitializer) // This super initializer is how you set the character movement component 
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCMCBaseConfiguration>(ACharacter::CharacterMovementComponentName))
{
	// Define reference for the bhop character movement component
	BaseCharacterMovement = Cast<UCMCBaseConfiguration>(GetCharacterMovement());

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
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // To stop the capsule component from colliding with our camera
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // same here
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	//GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh); // For when you create your own collision channels

	//////////////////////// Configure the character movement ////////////////////////
	// Movement component configuration (configure the movement (the movement for the character (the character's movement)))
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of the movement input
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 640.0f, 0.0f); // I think this is nulled out by our acceleration forces
	bUseControllerRotationYaw = true; // This off with OrientRotationMovement on lets the character rotation be independent from walking direction (keep it on for bhopping) -> set it to false when standing still perhaps

	//////////////////////// Replication stuff (Server/Client rendering) 
	NetUpdateFrequency = 66.f; // default update character on other machines 66 times a second (general fps defaults)
	MinNetUpdateFrequency = 33.f; // To help with bandwidth and lagginess, allow a minNetUpdateFrequency, which is generally 33 in fps games
	// The other important value is the server config tick rate, which is in the project defaultEngine.ini -> [/Script/OnlineSubsystemUtils.IpNetDriver] NetServerMaxTickRate = 60
	// also this which is especially crucial for implementing the gameplay ability system [SystemSettings] net.UseAdaptiveNetUpdateFrequency = 1
}


void ABaseCharacterConfiguration::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Base Movements
	PlayerInputComponent->BindAxis("MoveForward", this, &ABaseCharacterConfiguration::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABaseCharacterConfiguration::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABaseCharacterConfiguration::Turn);
	PlayerInputComponent->BindAxis("Lookup", this, &ABaseCharacterConfiguration::Lookup);

	// Auxillery movements
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABaseCharacterConfiguration::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ABaseCharacterConfiguration::StopJump);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ABaseCharacterConfiguration::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ABaseCharacterConfiguration::StopSprint);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABaseCharacterConfiguration::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ABaseCharacterConfiguration::CrouchButtonReleased);
}


void ABaseCharacterConfiguration::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); // Net/UnrealNetwork is required to declare rep lifetimes

	//DOREPLIFETIME(ABhopCharacter, InputDirection);
}


void ABaseCharacterConfiguration::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}
#pragma endregion


#pragma region Main Functions
void ABaseCharacterConfiguration::BeginPlay()
{
	Super::BeginPlay();
}


void ABaseCharacterConfiguration::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
#pragma endregion



#pragma region Utility
void ABaseCharacterConfiguration::PrintToScreen(FColor color, FString message)
{
	if (HasAuthority() && IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("server %s:: %s"), *GetNameSafe(this), *message);
	}

	if (!HasAuthority() && IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("client:: %s"), *GetNameSafe(this), *message);
	}
}
#pragma endregion

#pragma region Input Functions
void ABaseCharacterConfiguration::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}


void ABaseCharacterConfiguration::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// Find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}


void ABaseCharacterConfiguration::Turn(float Value)
{
	AddControllerYawInput(Value);
}


void ABaseCharacterConfiguration::Lookup(float Value)
{
	AddControllerPitchInput(Value);
}


void ABaseCharacterConfiguration::StartJump()
{
	Jump();
}


void ABaseCharacterConfiguration::StopJump()
{
	StopJumping();
}


void ABaseCharacterConfiguration::StartSprint()
{
	if (GetBaseCharacterMovement())
	{
		GetBaseCharacterMovement()->SprintPressed();
	}
}


void ABaseCharacterConfiguration::StopSprint()
{
	if (GetBaseCharacterMovement())
	{
		GetBaseCharacterMovement()->SprintReleased();
	}
}


void ABaseCharacterConfiguration::CrouchButtonPressed()
{
	Crouch();
}


void ABaseCharacterConfiguration::CrouchButtonReleased()
{
	UnCrouch();
}
#pragma endregion

