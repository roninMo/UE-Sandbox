// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BhopHud.generated.h"

/**
 * 
 */
UCLASS()
class SANDBOX_API ABhopHud : public AHUD
{
	GENERATED_BODY()
	

public:
	virtual void DrawHUD() override;

	// To add this class in a blueprint, implement it with a TSubClassOf
	UPROPERTY()
		class UCharacterOverlay* CharacterOverlay;
	UPROPERTY(EditAnywhere, Category = "Configuration")
		TSubclassOf<class UUserWidget> CharacterOverlayClass;
	void AddCharacterOverlay();


protected:
	virtual void BeginPlay() override;


private:
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);


};
