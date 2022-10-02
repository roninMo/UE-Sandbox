// Fill out your copyright notice in the Description page of Project Settings.


#include "BhopHud.h"
#include "GameFramework/PlayerController.h"
#include "Sandbox/HUDs/CharacterOverlay.h"


void ABhopHud::BeginPlay()
{
	Super::BeginPlay();

	// Add the bhop widget
	AddCharacterOverlay();
}


void ABhopHud::DrawHUD()
{
	Super::DrawHUD();
}


void ABhopHud::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		if (CharacterOverlay) CharacterOverlay->AddToViewport();
	}
}


void ABhopHud::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
}
