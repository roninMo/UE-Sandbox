// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ProtoAttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(ClassName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(ClassName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(ClassName)

/**
 * 
 */
UCLASS()
class SANDBOX_API UProtoAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
		
public:
	UProtoAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Health
	UFUNCTION() virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
		FGameplayAttributeData Health;

	// Stamina
	UFUNCTION() virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Stamina)
		FGameplayAttributeData Stamina;

	// Attack power
	UFUNCTION() virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_AttackPower)
		FGameplayAttributeData AttackPower;

	// Mana 
	UFUNCTION() virtual void OnRep_Mana(const FGameplayAttributeData& OldMana);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Mana)
		FGameplayAttributeData Mana;


protected:



};
