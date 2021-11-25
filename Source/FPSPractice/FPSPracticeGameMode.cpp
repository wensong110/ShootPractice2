// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSPracticeGameMode.h"
#include "FPSPracticeCharacter.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"

AFPSPracticeGameMode::AFPSPracticeGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	ConstructorHelpers::FClassFinder<UUserWidget> Cross(TEXT("/Game/ThirdPersonCPP/Blueprints/WBP_Cross"));
	const auto CrossWidgit=CreateWidget(GetWorld(),Cross.Class,FName(TEXT("Cross")));
	if(CrossWidgit)
	{
		CrossWidgit->AddToViewport();
		U_CrossWidget=CrossWidgit;
	}
}

void AFPSPracticeGameMode::ShowCross()
{
	U_CrossWidget->SetVisibility(ESlateVisibility::Visible);
}

void AFPSPracticeGameMode::HideCross()
{
	U_CrossWidget->SetVisibility(ESlateVisibility::Hidden);
}

