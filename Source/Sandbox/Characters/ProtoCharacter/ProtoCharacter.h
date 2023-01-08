// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// Character stuff
#include "Sandbox/Characters/BaseConfiguration/BaseCharacterConfiguration.h"
#include "GameFramework/Character.h"

// Gameplay ability system stuff
#include "AbilitySystemInterface.h"
#include <GameplayEffectTypes.h>

#include "ProtoCharacter.generated.h"

// Gameplay Ability System documentation: https://github.com/tranek/GASDocumentation
/*
Ability system debugging:
	- showdebug abilitysystem
	- AbilitySystem.Debug.NextCategory
	- page up and page down to go between targets
*/ 


UCLASS()
class SANDBOX_API AProtoCharacter : public ABaseCharacterConfiguration, public IAbilitySystemInterface
{
	GENERATED_BODY()


//////////////////////////////////////////////////////////////////////////
// TODO (Keep this stuff organized baby)								//
//////////////////////////////////////////////////////////////////////////
/* 
	* Create our own character movement component
	* Create an animation instance (a kinematic rig to adopt animations with ease (use the animation starter pack)
	* Create a hud and learn how to make it spiffy
	* Create an inventory component as a friend class of the character component
	* Learn AI, and how to implement it through C++ (Start with the Utility Ai with replicated functionality (This should be easier since AI is just basic movement, everything is handled on the server anyways))
	* Basic combat (learn from the ai), Create lock on, hit animations, and projectile based weapons (bow) 

	* Create a animation instance for an animation blueprint that grabs all the information needed for the anim blueprint
	* Create a blend pose for walking/running/crouching, and blend two animations together, one for walking and running, and the other for a mix between that and actions that the character does like holding a weapon, etc.
	* Pully the ue mannequin into blender, or create one and make some animations for the character (this is gonna be super important later and also sounds super fun for creating all the stuff)


	* Later
	* Fancy animations and characters, play around in blender
*/ 





//////////////////////////////////////////////////////////////////////////
// Base functions and components										//
//////////////////////////////////////////////////////////////////////////
public:
	AProtoCharacter(const FObjectInitializer& ObjectInitializer);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//virtual void Destroyed() override; // This is a replicated function, handle logic pertaining to character death in here for free
	//virtual void OnRep_ReplicatedMovement() override; // overriding this to replicate simulated proxies movement: https://www.udemy.com/course/unreal-engine-5-cpp-multiplayer-shooter/learn/lecture/31515548#questions
	virtual void Tick(float DeltaTime) override;


protected:
	virtual void BeginPlay() override;




//////////////////////////////////////////////////////////////////////////
// Input functions														//
//////////////////////////////////////////////////////////////////////////
protected:
	// Movement
	virtual void StartJump() override;
	virtual void StopJump() override;
	virtual void StartSprint() override;
	virtual void StopSprint() override;
	virtual void CrouchButtonPressed() override;
	virtual void CrouchButtonReleased() override;


private:

	


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gameplay Ability System				// https://github.com/tranek/GASDocumentation#concepts								//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const;
	virtual void PossessedBy(AController* NewController) override; // Initialize ability system on Server call
	virtual void OnRep_PlayerState() override; // Initialize ability system on Client call, or do this in the player controller on the AcknowledgePossession function


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attribute Gas")
		class UProtoASC* AbilitySystemComponent;
	UPROPERTY()
		class UProtoAttributeSet* Attributes;


	virtual void InitializeAttributes();
	virtual void GiveBaseAbilities();
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Attribute Gas")
		TSubclassOf<class UGameplayEffect> DefaultAttributeSet;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Attribute Gas")
		TArray<TSubclassOf<class UProtoGasGameplayAbility>> DefaultAbilities;


//////////////////////////////////////////////////////////////////////////
// Animations and Montages												//
//////////////////////////////////////////////////////////////////////////
public:



//////////////////////////////////////////////////////////////////////////
// Getters and Setters													//
//////////////////////////////////////////////////////////////////////////
public:




};
