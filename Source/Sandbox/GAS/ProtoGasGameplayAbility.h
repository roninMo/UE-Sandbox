// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include <Sandbox/Sandbox.h>
#include "ProtoGasGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class SANDBOX_API UProtoGasGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	

public:
	UProtoGasGameplayAbility();

	// Abilities with this set will automatically activate when the input is pressed
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
		EGASAbilityInputID AbilityInputID = EGASAbilityInputID::None; 


};
