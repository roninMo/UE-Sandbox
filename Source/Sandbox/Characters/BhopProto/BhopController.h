// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BhopController.generated.h"

/**
 * 
 */
UCLASS()
class SANDBOX_API ABhopController : public APlayerController
{
	GENERATED_BODY()
	
		
public:
	virtual void Tick(float DeltaTime) override;

	void SetHUDSpeedometer();
	void SetHUDefaultMaxWalkSpeed();
	void SetHUDFricton();


protected:
	virtual void BeginPlay() override;


private:
	UPROPERTY()
		class ABhopHud* BhopHUD;


};
