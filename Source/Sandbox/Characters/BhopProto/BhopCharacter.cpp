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
#include "Components/PrimitiveComponent.h"

// Types
#include "AI/Navigation/NavigationTypes.h"
#include "UObject/Class.h"

// Gameplay Ability System plugin
#include "Sandbox/GAS/ProtoASC.h"
#include "Sandbox/GAS/ProtoAttributeSet.h"
#include "Sandbox/GAS/ProtoGasGameplayAbility.h"

// Bhop Character Movement Component
#include "BhopCharacterMovementComponent.h"


#pragma region Constructors
ABhopCharacter::ABhopCharacter(const FObjectInitializer& ObjectInitializer) // This super initializer is how you set the character movement component 
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBhopCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Define reference for the bhop character movement component
	BhopCharacterMovement = Cast<UBhopCharacterMovementComponent>(GetCharacterMovement());

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
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f); // I think this is nulled out by our acceleration forces
	bUseControllerRotationYaw = true; // This off with OrientRotationMovement on lets the character rotation be independent from walking direction (keep it on for bhopping) -> set it to false when standing still perhaps

	// Configure the bhop defaults (the bhopping configuration (the configuration for bhopping (bhopping configuration))) 
	DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	DefaultFriction = GetCharacterMovement()->GroundFriction;
	DefaultJumpVelocity = GetCharacterMovement()->JumpZVelocity;

	//////////////////////// Replication stuff (Server/Client rendering) 
	NetUpdateFrequency = 66.f; // default update character on other machines 66 times a second (general fps defaults)
	MinNetUpdateFrequency = 33.f; // To help with bandwidth and lagginess, allow a minNetUpdateFrequency, which is generally 33 in fps games
	// The other important value is the server config tick rate, which is in the project defaultEngine.ini -> [/Script/OnlineSubsystemUtils.IpNetDriver] NetServerMaxTickRate = 60
	// also this which is especially crucial for implementing the gameplay ability system [SystemSettings] net.UseAdaptiveNetUpdateFrequency = 1


	//////////////////////// Ability System Component ////////////////////////
	AbilitySystemComponent = CreateDefaultSubobject<UProtoASC>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	Attributes = CreateDefaultSubobject<UProtoAttributeSet>("Attributes");

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
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ABhopCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ABhopCharacter::StopSprint);

	// Actions
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABhopCharacter::EquipButtonPress);
	PlayerInputComponent->BindAction("UnEquip", IE_Released, this, &ABhopCharacter::UnEquipButtonPress);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABhopCharacter::CrouchButtonPressed);

	// Bind ability inputs
	if (AbilitySystemComponent && InputComponent) // Sometimes the ability system is valid but the input wouldn't be and vice versa
	{
		const FGameplayAbilityInputBinds Binds("Confirm", "Cancel", "EGASAbilityInputID", static_cast<int32>(EGASAbilityInputID::Confirm), static_cast<int32>(EGASAbilityInputID::Cancel)); // Sync the enums with the ability names
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, Binds);
	}
}


void ABhopCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); // Net/UnrealNetwork is required to declare rep lifetimes

	DOREPLIFETIME(ABhopCharacter, InputDirection);

	DOREPLIFETIME(ABhopCharacter, FrameTime);
	DOREPLIFETIME(ABhopCharacter, RampMomentumFactor);
	DOREPLIFETIME(ABhopCharacter, DefaultMaxWalkSpeed);
	DOREPLIFETIME(ABhopCharacter, bApplyingBhopCap);
	// ImpulseVector
}


void ABhopCharacter::InitializeAttributes()
{
	if (AbilitySystemComponent && DefaultAttributeEffect)
	{
		// This is a context handle
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1, EffectContext);

		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle GEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}


 void ABhopCharacter::GiveBaseAbilities()
{
	 if (HasAuthority() && AbilitySystemComponent)
	 {
		 for (TSubclassOf<UProtoGasGameplayAbility>& Ability : DefaultAbilties)
		 {
			 AbilitySystemComponent->GiveAbility(
				 FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this)
			 );
		 }
	 }
}
#pragma endregion


#pragma region UE Main Functions
void ABhopCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitCharacterMovement();
}


void ABhopCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// This function is the earliest you can access an actor component from the character clas
}


void ABhopCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Get the character component if it was deleted when unpossessed
	BhopCharacterMovement = Cast<UBhopCharacterMovementComponent>(GetCharacterMovement());

	// Gameplay ability system init
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeAttributes();
	GiveBaseAbilities();
}


void ABhopCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeAttributes();
	if (AbilitySystemComponent && InputComponent) // Sometimes the ability system is valid but the input wouldn't be and vice versa
	{
		const FGameplayAbilityInputBinds Binds("Confirm", "Cancel", "EGASAbilityInputID", static_cast<int32>(EGASAbilityInputID::Confirm), static_cast<int32>(EGASAbilityInputID::Cancel)); // Sync the enums with the ability names
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, Binds);
	}
}


void ABhopCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//// Bhop logic //// Calculate frameTime (DeltaTime), PrevVelocity, and the XYspeedometer
	FrameTime = DeltaTime;
	PrevVelocity = GetVelocity();
	XYspeedometer = PrevVelocity.Length();
	//UE_LOG(LogTemp, Warning, TEXT("Time: %f, Tick::PrevVel: %s"), UKismetSystemLibrary::GetGameTimeInSeconds(this), *PrevVelocity.ToCompactString());
}
#pragma endregion


#pragma region bhop (movement) logic
void ABhopCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);
	InitCharacterMovement();

	// Debugging (print the movement mode information)
	const EMovementMode NewMovementMode = CachedCharacterMovement->MovementMode;
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
				if (LegBonkSound) UGameplayStatics::PlaySoundAtLocation(this, LegBonkSound, GetActorLocation());
			}
		}

		// Set the land sound cooldown variable to be up to date with the game time
		LandingSoundCooldownTotal = LandSoundCooldown + UKismetSystemLibrary::GetGameTimeInSeconds(this);

		// bhop easy mode (good for ue debugging)
		if (bEnablePogo && bJumpPressed)
		{
			BhopAndTrimpLogic();
		}
		else
		{
			// time window upon landing before friction applied (frame delay allows maintaining speed while bhopping) Increasing this delay more than a few frames may result in undesired effects.Default = 1 frame
			FLatentActionInfo StuffTodoOnComplete; // https://www.reddit.com/r/unrealengine/comments/b1t1d8/how_to_wait_for/
			StuffTodoOnComplete.CallbackTarget = this;
			StuffTodoOnComplete.ExecutionFunction = FName("ResetFrictionDelay");
			StuffTodoOnComplete.Linkage = 0;
			StuffTodoOnComplete.UUID = NumberOfTimesPogoResetFrictionUUID;
			UKismetSystemLibrary::Delay(GetWorld(), FrameTime, StuffTodoOnComplete);
		}
	}

	Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);
}


void ABhopCharacter::BhopAndTrimpLogic()
{
	RampCheck();
	ApplyTrimp();

	bool handleJumpAndBhopCap = false;

	// play sound if jump sound is not on cooldown
	if (JumpSoundCooldownTotal < UKismetSystemLibrary::GetGameTimeInSeconds(this))
	{
		Jump();
		if (JumpSound) UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation());
		handleJumpAndBhopCap = true;
	}
	else if (GetCharacterMovement() && GetCharacterMovement()->IsWalking())
	{
		Jump();
		handleJumpAndBhopCap = true;
	}

	// Only do this logic if jumping
	if (handleJumpAndBhopCap)
	{
		JumpSoundCooldownTotal = JumpSoundCooldown + UKismetSystemLibrary::GetGameTimeInSeconds(this);

		// is bunny hop cap enabled?
		/*if (bEnableBunnyHopCap)
		{
			ApplyBhopCap();
		}*/
	}
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

	// Apply the trimp impulse
	if (GetBhopCharacterMovement())
	{
		GetBhopCharacterMovement()->SetBhopJumpZVelocity(TrimpJumpImpulse);
		GetBhopCharacterMovement()->SetBhopMaxWalkSpeed(TrimpLateralImpulse);
		AddMovementInput(TrimpImpulse);
	}
}
#pragma endregion


#pragma region Bhop Cap
void ABhopCharacter::ApplyBhopCap()
{
	// caps speed to: default maxwalk speed* bunnyhop Cap Factor. Adjust the bunnyhop Cap factor to alter the cap
	float BhopCapSpeed = DefaultMaxWalkSpeed * BunnyHopCapFactor;
	      
	// Is the current speed more than bunnyhop cap?
	if (XYspeedometer > BhopCapSpeed && GetCharacterMovement())
	{
		// find how much we are over cap, and reduce this excess speed by Bhop Bleed factor
		float SpeedOverCap = (BhopCapSpeed - XYspeedometer) * BhopBleedFactor;

		// predict our new speed after reduction
		bhopCapNewSpeed = SpeedOverCap + BhopCapSpeed;
		bApplyingBhopCap = true;
		BhopCapVector = UKismetMathLibrary::Multiply_VectorFloat(PrevVelocity.GetSafeNormal(), SpeedOverCap);

		// [Not used for multiplayer] apply impulse in opposite direction of our movement
		//GetCharacterMovement()->AddImpulse(BhopCapVector, true);
		//GetBhopCharacterMovement()->SetBhopMaxWalkSpeed(bhopCapNewSpeed);
	}
	else
	{
		bApplyingBhopCap = false;
	}

	// Apply the bhop cap
	if (GetBhopCharacterMovement())
	{
		GetBhopCharacterMovement()->SetBhopMaxWalkSpeed(bhopCapNewSpeed);
		AddMovementInput(BhopCapVector);
	}
}
#pragma endregion


#pragma region Handling Friction
// Remove friction in air and for the coyote frames on landing
void ABhopCharacter::RemoveFriction()
{
	if (GetBhopCharacterMovement())
	{
		GetBhopCharacterMovement()->SetBhopGroundFriction(0.f);
	}
}


// When the character lands, add the base movement speed limit and friction
void ABhopCharacter::ResetFriction()
{
	if (GetBhopCharacterMovement())
	{
		GetBhopCharacterMovement()->SetBhopMaxWalkSpeed(DefaultMaxWalkSpeed);
		GetBhopCharacterMovement()->SetBhopGroundFriction(DefaultFriction);
	}
}


// Delay function for OnMovementModeChanged function
void ABhopCharacter::ResetFrictionDelay()
{
	ResetFriction();
	NumberOfTimesPogoResetFrictionUUID++;
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
		//UE_LOG(LogTemp, Warning, TEXT("RampCheck::GroundAngleDotproduct: %f, angle of ramp: %f \n"), RampCheckGroundAngleDotproduct, AngleOfRamp - 90.f);

		// Check if the angle is greater than 2 degrees (90 is a flat surface)
		if (AngleOfRamp > 92.f) return true;
	}

	return false;
}
#pragma endregion



#pragma region Acceleration Functions
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

	// Apply the ground acceleration
	UE_LOG(LogTemp, Warning, TEXT("Time: %f, Direction::Gnd: %s, MaxWalkSpeed: %f"), UKismetSystemLibrary::GetGameTimeInSeconds(this), *GroundAccelDir.ToCompactString(), CalcMaxWalkSpeed);
	GetBhopCharacterMovement()->SetBhopMaxWalkSpeed(CalcMaxWalkSpeed);
	AddMovementInput(GroundAccelDir); // add movement input node must be used for proper multiplayer replication as it utilizes predictionand network history.
}


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

	// Apply the acceleration
	//UE_LOG(LogTemp, Warning, TEXT("Time: %f, CalcMaxSpeed::Air: %s, MaxWalkSpeed: %f"), UKismetSystemLibrary::GetGameTimeInSeconds(this), *AirAccelDir.ToCompactString(), CalcMaxAirSpeed);
	GetBhopCharacterMovement()->SetBhopMaxWalkSpeed(CalcMaxAirSpeed);
	AddMovementInput(AirAccelDir); // add movement input node must be used for proper multiplayer replication as it utilizes predictionand network history.
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
		AccelerateAir();
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
		}
		else
		{
			// In the case that we just got out of rampsliding, then we add a momentum force to prevent stickiness when exiting the rampslide
			if (bIsRampSliding)
			{
				UE_LOG(LogTemp, Warning, TEXT("Adding Ramp momentum"));
				if (GetBhopCharacterMovement())
				{
					GetBhopCharacterMovement()->SetBhopJumpZVelocity(DefaultJumpVelocity * RampMomentumFactor);
					Jump();
				}

				// Then create a delay before applying the friction again
				FLatentActionInfo StuffTodoOnComplete; // https://www.reddit.com/r/unrealengine/comments/b1t1d8/how_to_wait_for/
				StuffTodoOnComplete.CallbackTarget = this;
				StuffTodoOnComplete.ExecutionFunction = FName("MovementDelayLogic");
				StuffTodoOnComplete.Linkage = 0;
				StuffTodoOnComplete.UUID = NumberOfTimesRampSlided;
				UKismetSystemLibrary::Delay(GetWorld(), FrameTime, StuffTodoOnComplete);
			}
			else // If we weren't rampsliding, then handle the base movement logic
			{
				BaseMovementLogic();
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
	}
	else // Apply the standard movement logic
	{
		AddMovementInput(InputForwardVector, InputForwardAxis);
		AddMovementInput(InputSideVector, InputSideAxis); // add movement input node must be used for proper multiplayer replication as it utilizes predictionand network history.
	}
}


void ABhopCharacter::MovementDelayLogic()
{

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
	Jump();

	bJumpPressed = true;
	// Are we walking when we press jump? (this prevents jump sound spamming when pressing jump key in midair)
	if (GetCharacterMovement() && GetCharacterMovement()->IsWalking())
	{
		BhopAndTrimpLogic();
	}

}


void ABhopCharacter::StopJump()
{
	bJumpPressed = false;
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
	if (GetBhopCharacterMovement()) GetBhopCharacterMovement()->SprintPressed();
}


void ABhopCharacter::StopSprint()
{
	if (GetBhopCharacterMovement()) GetBhopCharacterMovement()->SprintReleased();
}


void ABhopCharacter::CrouchButtonPressed()
{
	if (bIsCrouched) UnCrouch();
	else Crouch();
}


void ABhopCharacter::EquipButtonPress()
{
	DebugCharacterName = 0;
	UE_LOG(LogTemp, Warning, TEXT("TESTING THE SERVER BOI"));
}


void ABhopCharacter::UnEquipButtonPress()
{
	DebugCharacterName = 1;
	UE_LOG(LogTemp, Warning, TEXT("TESTING THE CLIENTS"));
}
#pragma endregion




#pragma region Getters and Setters
void ABhopCharacter::InitCharacterMovement()
{
	// Get the character movement component
	CachedCharacterMovement = GetCharacterMovement();
	if (CachedCharacterMovement == nullptr) UE_LOG(LogTemp, Error, TEXT("ERROR: Character movement component not found, exiting OnMovementModeChanged!"));
}


inline float ABhopCharacter::GetDefaultMaxWalkSpeed()
{
	if (GetCharacterMovement()) return GetCharacterMovement()->MaxWalkSpeed;
	else return DefaultMaxWalkSpeed;
}


inline float ABhopCharacter::GetFriction()
{
	if (GetCharacterMovement()) return GetCharacterMovement()->GroundFriction;
	else return DefaultFriction;
}


UAbilitySystemComponent* ABhopCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
#pragma endregion



void ABhopCharacter::PrintToScreen(FColor color, FString message)
{
	if (DebugCharacterName == 0) // Server
	{
		if (HasAuthority() && IsLocallyControlled())
		{
			UE_LOG(LogTemp, Warning, TEXT("server %s:: %s"), *GetNameSafe(this), *message);
		}
	}
	else // Client
	{
		if (!HasAuthority() && IsLocallyControlled())
		{
			UE_LOG(LogTemp, Warning, TEXT("client:: %s"), *GetNameSafe(this), *message);
		}
	}
}


#pragma region Random Noteworthy stuff I found for coding and learning how to debug and whatever else tickles my fancy
/* 
Grabbing and printing enums, also how to get the name of teh character
	const UEnum* MovementModeEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EMovementMode"));
	UE_LOG(LogTemp, Warning, TEXT("THIS: %s, Movement mode: %s")
		, *GetNameSafe(this)
		, *(MovementModeEnum ? MovementModeEnum->GetNameStringByIndex(PreviousMovementMode) : TEXT("<Invalid Enum>")));
*/
#pragma endregion
