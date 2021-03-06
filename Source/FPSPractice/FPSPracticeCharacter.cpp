// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSPracticeCharacter.h"

#include "DrawDebugHelpers.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// AFPSPracticeCharacter

AFPSPracticeCharacter::AFPSPracticeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; 
}
void AFPSPracticeCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSPracticeCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSPracticeCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AFPSPracticeCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AFPSPracticeCharacter::LookUpAtRate);
	//PlayerInputComponent->BindAxis("CamTurnRate",this,&AFPSPracticeCharacter::CamTurnAtRate);
	//PlayerInputComponent->BindAxis("CamLookUpRate",this,&AFPSPracticeCharacter::CamLookUpAtRate);
	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AFPSPracticeCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AFPSPracticeCharacter::TouchStopped);
	PlayerInputComponent->BindTouch(IE_Repeat, this, &AFPSPracticeCharacter::TouchUpdate);
	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AFPSPracticeCharacter::OnResetVR);
}


FHitResult AFPSPracticeCharacter::BulletTrance(const FVector& FromLocation)
{
	const FVector CamFromLocation=GetFollowCamera()->GetComponentLocation();
	const FRotator CamRotator=GetFollowCamera()->GetComponentRotation();
	FVector ToLocation=CamFromLocation+CamRotator.RotateVector(FVector(1,0,0))*5000;
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult,CamFromLocation,ToLocation,ECollisionChannel::ECC_Visibility);
	const FVector TargetLocation=HitResult.Location;
	ToLocation=FromLocation+(TargetLocation-FromLocation)*1.1;
	GetWorld()->LineTraceSingleByChannel(HitResult,FromLocation,ToLocation,ECollisionChannel::ECC_Visibility);
	//DrawDebugLine(GetWorld(),FromLocation,ToLocation,FColor::Red,0,-1,0,3);
	return HitResult;
}

void AFPSPracticeCharacter::DrawPreviewLine(const FVector& FromLocation, const FVector& Velocity, float gravity, int n)
{
	const float ZAxisVelocity=Velocity.Z;
	const FVector2D XYAxisVelocity=FVector2D(Velocity.X,Velocity.Y);
	//FVector2D XYDrection;
	//float XYLength;
	//XYAxisVelocity.ToDirectionAndLength(XYDrection,XYLength);
	constexpr float Dt=0.05f;
	for(float t=0;t<=n*Dt;t+=Dt)
	{
		bool End=0;
		const float Z0=ZAxisVelocity*t-0.5*gravity*t*t;
		const FVector2D XY0=XYAxisVelocity*t;
		const float Z1=ZAxisVelocity*(t+Dt)-0.5*gravity*(t+Dt)*(t+Dt);
		const FVector2D XY1=XYAxisVelocity*(t+Dt);
		FVector Point0=FVector(XY0,Z0)+FromLocation;
		FVector Point1=FVector(XY1,Z1)+FromLocation;
		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult,Point0,Point1,ECC_Visibility);
		if(HitResult.GetActor())
		{
			End=true;
			Point1=HitResult.Location;
			GEngine->AddOnScreenDebugMessage(2,3.0f,FColor::Blue,HitResult.ToString());
		}
		DrawDebugLine(GetWorld(),Point0,Point1,FColor::Red,false,-1,0,3);
		if(End)
		{
			DrawDebugSphere(GetWorld(),Point1,5.0f,10,FColor::Blue);
			break;
		}
	}
}

void AFPSPracticeCharacter::ProcessTouch(ETouchIndex::Type FingerIndex, FVector Location, bool bDown)
{
	GWorld->GetGameViewport()->GetViewportSize(ScreenSize);
	if(bDown)
	{
		if ( Location.X>ScreenSize.X*0.8 && Location.Y>ScreenSize.Y*0.7 )
		{
			//GEngine->AddOnScreenDebugMessage(5,3.0f,FColor::Red,Location.ToString()+ScreenSize.ToString());
			if(!fingerRecord.bFireDown)
			{
				fingerRecord.Fire_Index=FingerIndex;
				fingerRecord.bFireDown=1;
				OnFireDown();
			}
			return;
		}
		float TurnButtonPosX=ScreenSize.X*0.8;
		float TurnButtonPosY=ScreenSize.Y*0.2;
		if((TurnButtonPosX-Location.X)*(TurnButtonPosX-Location.X)+(TurnButtonPosY-Location.Y)*(TurnButtonPosY-Location.Y)<1000)
		{
			if(!fingerRecord.bCamTurnDown)
			{
				fingerRecord.CamTurn_Index=FingerIndex;
				fingerRecord.bCamTurnDown=1;
				BaseTurnRate*=5;
				BaseLookUpRate*=5;
				OnCamTurnDown();
			}
			return;
		}
		float JumpButtonPosX=ScreenSize.X*0.9;
		float JumpButtonPosY=ScreenSize.Y*0.6;
		if((JumpButtonPosX-Location.X)*(JumpButtonPosX-Location.X)+(JumpButtonPosY-Location.Y)*(JumpButtonPosY-Location.Y)<1000)
		{
			if(!fingerRecord.bJumpDown)
			{
				fingerRecord.Jump_Index=FingerIndex;
				fingerRecord.bJumpDown=1;
				OnJumpDown();
			}
			return;
		}
		if ( Location.X>ScreenSize.X*0.65 && Location.Y>ScreenSize.Y*0.7 )
		{
			if(!fingerRecord.bBoomDown)
			{
				fingerRecord.Boom_Index=FingerIndex;
				fingerRecord.bBoomDown=1;
			}
			OnBoomDown();
			return;
		}
	}else
	{
		if(fingerRecord.bBoomDown&&fingerRecord.Boom_Index==FingerIndex)
		{
			fingerRecord.bBoomDown=0;
		}
		if(fingerRecord.bFireDown&&fingerRecord.Fire_Index==FingerIndex)
		{
			fingerRecord.bFireDown=0;
			OnFireUp();
		}
		if(fingerRecord.bCamTurnDown&&fingerRecord.CamTurn_Index==FingerIndex)
		{
			fingerRecord.bCamTurnDown=0;
			BaseTurnRate/=5;
			BaseLookUpRate/=5;
			OnCamTurnUp();
		}
		if(fingerRecord.bJumpDown&&fingerRecord.Jump_Index==FingerIndex)
		{
			fingerRecord.bJumpDown=0;
			OnJumpUp();
		}
	}
}

void AFPSPracticeCharacter::OnResetVR()
{
	// If FPSPractice is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in FPSPractice.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AFPSPracticeCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	TouchItem.bIsPressed = true;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
	ProcessTouch(FingerIndex,Location,1);
}

void AFPSPracticeCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
	TouchItem.Location=Location;
	ProcessTouch(FingerIndex,Location,0);
}

void AFPSPracticeCharacter::TouchUpdate(ETouchIndex::Type FingerIndex, FVector Location)
{
	const FVector2D Move=FVector2D(Location.X-TouchItem.Location.X,Location.Y-TouchItem.Location.Y);
	if(FMath::Abs(Move.X)>0||FMath::Abs(Move.Y)>0)
	{
	    if(FMath::Abs(Move.X)<4&&FMath::Abs(Move.Y)<4)
		TouchItem.bMoved=true;
		LookUpAtRate(Move.Y*0.05);
		TurnAtRate(Move.X*0.05);
	}else
	{
		TouchItem.bMoved=false;
	}
	TouchItem.Location=Location;
}

void AFPSPracticeCharacter::CamTurnAtRate(float Rate)
{
	if(Rate!=0)
	{
		bUseControllerRotationYaw=true;
		isAltDown=1;
	}
	CamTurn.X=Rate;
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
	//bUseControllerRotationYaw=false;
	/*if(CamTurn.Equals(FVector2D(0,0)))
	{
		LetCamBack();
	}*/
}

void AFPSPracticeCharacter::CamLookUpAtRate(float Rate)
{
	if(Rate!=0)
	{
		bUseControllerRotationYaw=true;
		isAltDown=1;
	}
	CamTurn.Y=Rate;
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	//bUseControllerRotationYaw=false;
	/*if(CamTurn.Equals(FVector2D(0,0)))
	{
		LetCamBack();
	}*/
}

void AFPSPracticeCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AFPSPracticeCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AFPSPracticeCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = GetActorRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AFPSPracticeCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = GetActorRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
