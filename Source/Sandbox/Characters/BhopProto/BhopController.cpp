// Fill out your copyright notice in the Description page of Project Settings.


#include "BhopController.h"
#include "Sandbox/Characters/BhopProto/BhopCharacter.h"
#include "Components/TextBlock.h"

#include "Sandbox/HUDs/BhopHud.h"
#include "Sandbox/HUDs/CharacterOverlay.h"


void ABhopController::BeginPlay()
{
	Super::BeginPlay();

	BhopHUD = Cast<ABhopHud>(GetHUD());
}


void ABhopController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDSpeedometer();
	SetHUDefaultMaxWalkSpeed();
	SetHUDFricton();
}


void ABhopController::SetHUDSpeedometer()
{
	// Set the hud values from the character's calculated speed
	ABhopCharacter* BhopCharacter = Cast<ABhopCharacter>(GetPawn());
	if (BhopCharacter)
	{
		BhopHUD = BhopHUD == nullptr ? Cast<ABhopHud>(GetHUD()) : BhopHUD;
		if (BhopHUD && BhopHUD->CharacterOverlay && BhopHUD->CharacterOverlay->SpeedometerValue)
		{
			FString SpeedText = FString::Printf(TEXT("%d"), FMath::CeilToInt(BhopCharacter->GetSpeedometer()));
			BhopHUD->CharacterOverlay->SpeedometerValue->SetText(FText::FromString(SpeedText));
		}
	}
	else UE_LOG(LogTemp, Warning, TEXT("CharacterOverlay::SetHUDSpeedometer: An error occured while trying to get the character controller"));
}

void ABhopController::SetHUDefaultMaxWalkSpeed()
{
	// Set the hud values from the character's calculated speed
	ABhopCharacter* BhopCharacter = Cast<ABhopCharacter>(GetPawn());
	if (BhopCharacter)
	{
		BhopHUD = BhopHUD == nullptr ? Cast<ABhopHud>(GetHUD()) : BhopHUD;
		if (BhopHUD && BhopHUD->CharacterOverlay && BhopHUD->CharacterOverlay->DefaultMaxWalkSpeedValue)
		{
			FString SpeedText = FString::Printf(TEXT("%d"), FMath::CeilToInt(BhopCharacter->GetDefaultMaxWalkSpeed()));
			BhopHUD->CharacterOverlay->DefaultMaxWalkSpeedValue->SetText(FText::FromString(SpeedText));
		}
	}
	else UE_LOG(LogTemp, Warning, TEXT("CharacterOverlay::SetHUDefaultMaxWalkSpeed: An error occured while trying to get the character controller"));
}

void ABhopController::SetHUDFricton()
{
	// Set the hud values from the character's calculated speed
	ABhopCharacter* BhopCharacter = Cast<ABhopCharacter>(GetPawn());
	if (BhopCharacter)
	{
		BhopHUD = BhopHUD == nullptr ? Cast<ABhopHud>(GetHUD()) : BhopHUD;
		if (BhopHUD && BhopHUD->CharacterOverlay && BhopHUD->CharacterOverlay->FrictionValue)
		{
			FString FrictionText = FString::Printf(TEXT("%d"), FMath::CeilToInt(BhopCharacter->GetFriction()));
			BhopHUD->CharacterOverlay->FrictionValue->SetText(FText::FromString(FrictionText));
		}
	}
	else UE_LOG(LogTemp, Warning, TEXT("CharacterOverlay::SetHUDefaultMaxWalkSpeed: An error occured while trying to get the character controller"));
}
