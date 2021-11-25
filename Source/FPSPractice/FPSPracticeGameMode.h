// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSPracticeGameMode.generated.h"

UCLASS(minimalapi)
class AFPSPracticeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSPracticeGameMode();
	UUserWidget* U_CrossWidget;
	UFUNCTION(BlueprintCallable)
	void ShowCross();
	UFUNCTION(BlueprintCallable)
	void HideCross();
};



