// Fill out your copyright notice in the Description page of Project Settings.


#include "ProtoAttributeSet.h"
#include "Net/UnrealNetwork.h"


UProtoAttributeSet::UProtoAttributeSet()
{
}


void UProtoAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UProtoAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UProtoAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UProtoAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UProtoAttributeSet, Mana, COND_None, REPNOTIFY_Always);
}


void UProtoAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UProtoAttributeSet, Health, OldHealth);
}


void UProtoAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UProtoAttributeSet, Stamina, OldStamina);
}


void UProtoAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UProtoAttributeSet, AttackPower, OldAttackPower);
}


void UProtoAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UProtoAttributeSet, Mana, OldMana);
}
