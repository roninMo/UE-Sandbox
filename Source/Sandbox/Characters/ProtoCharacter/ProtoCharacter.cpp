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

// Bhop Character Movement Component
#include "Sandbox/Characters/BaseConfiguration/CMCBaseConfiguration.h"

// Types
#include "AI/Navigation/NavigationTypes.h"
#include "UObject/Class.h"

// Gameplay Ability System plugin
#include "Sandbox/GAS/ProtoASC.h" // GameplayAbilitySystemComponent
#include "Sandbox/GAS/ProtoAttributeSet.h" // AttributeSet
#include "Sandbox/GAS/ProtoGasGameplayAbility.h" // GameplayAbility
#include <GameplayEffectTypes.h> // Gameplay effect types



#pragma region Constructors
AProtoCharacter::AProtoCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) 
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UProtoASC>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	Attributes = CreateDefaultSubobject<UProtoAttributeSet>("Attributes");
}


void AProtoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); // Net/UnrealNetwork is required to declare rep lifetimes

	//DOREPLIFETIME(ABhopCharacter, InputDirection);
}


void AProtoCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}


void AProtoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

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

	// Bind the inputs to the abilities (Sometimes the player state isn't valid when this is called, and vice versa (so doing it in both solves this potential error))
	if (AbilitySystemComponent && InputComponent)
	{
		const FGameplayAbilityInputBinds Binds("Confirm", "Cancel", "EGASAbilityInputID", static_cast<int32>(EGASAbilityInputID::Confirm), static_cast<int32>(EGASAbilityInputID::Cancel));
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, Binds);
	}
}
#pragma endregion


#pragma region Main Functions
void AProtoCharacter::BeginPlay()
{
	Super::BeginPlay();


}


void AProtoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
#pragma endregion


#pragma region Gameplay Ability System Configuration
void AProtoCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Initialize the ASC on the server
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
	SetOwner(NewController); // ASC MixedMode replication requires that the ASC Owner's Owner be the Controller.

	// Initialize base attributes and abilities
	InitializeAttributes();
	GiveBaseAbilities();
}


void AProtoCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Initialize the ASC on the client
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// Bind the inputs to the abilities (Sometimes the player state isn't valid when this is called, and vice versa (so doing it in both solves this potential error))
		if (InputComponent)
		{
			const FGameplayAbilityInputBinds Binds("Confirm", "Cancel", "EGASAbilityInputID", static_cast<int32>(EGASAbilityInputID::Confirm), static_cast<int32>(EGASAbilityInputID::Cancel));
			AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, Binds);
		}
	}

}


void AProtoCharacter::InitializeAttributes()
{
	if (AbilitySystemComponent && DefaultAttributeSet)
	{
		// Use context handles to apply effects to characters/abilitySystemComponents
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeSet, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			// Take the ability system component and apply the gameplay effect to it (function names are ApplyGameplayEffectSpecToSelf ApplyGameplayEffectSpecToTarget, etc.)
			FActiveGameplayEffectHandle GEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void AProtoCharacter::GiveBaseAbilities()
{
	if (HasAuthority() && AbilitySystemComponent)
	{
		for (TSubclassOf<UProtoGasGameplayAbility>& StartupAbility : DefaultAbilities)
		{
			AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(StartupAbility, 1, static_cast<int32>(StartupAbility.GetDefaultObject()->AbilityInputID), this)
			);
		}
	}
}


UAbilitySystemComponent* AProtoCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
#pragma endregion



#pragma region Input Functions
void AProtoCharacter::StartJump()
{
	Super::StartJump();
}


void AProtoCharacter::StopJump()
{
	Super::StopJump();
}


void AProtoCharacter::StartSprint()
{
	Super::StartSprint();
}


void AProtoCharacter::StopSprint()
{
	Super::StopSprint();
}


void AProtoCharacter::CrouchButtonPressed()
{
	Super::CrouchButtonPressed();
}


void AProtoCharacter::CrouchButtonReleased()
{
	Super::CrouchButtonPressed();
}
#pragma endregion

