// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSPracticeCharacter.generated.h"

UCLASS(config=Game)
class AFPSPracticeCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AFPSPracticeCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;
	
	UPROPERTY(BlueprintReadWrite)
	bool isAltDown=0;
	UFUNCTION(BlueprintCallable)
	FHitResult BulletTrance(const FVector& FromLocation);
	UFUNCTION(BlueprintCallable)
	void DrawPreviewLine(const FVector& FromLocation, const FVector& Velocity, float gravity,int n=980);
	UFUNCTION(BlueprintImplementableEvent)
	void OnFireDown();
	UFUNCTION(BlueprintImplementableEvent)
	void OnJumpDown();
	UFUNCTION(BlueprintImplementableEvent)
	void OnBoomDown();
	UFUNCTION(BlueprintImplementableEvent)
	void OnFireUp();
	UFUNCTION(BlueprintImplementableEvent)
	void OnJumpUp();
	UFUNCTION(BlueprintImplementableEvent)
	void OnCamTurnDown();
	UFUNCTION(BlueprintImplementableEvent)
	void OnCamTurnUp();
	void ProcessTouch(ETouchIndex::Type FingerIndex, FVector Location,bool bDown);
	FVector2D ScreenSize;
	void BeginPlay() override;
	struct FingerRecord
	{
		ETouchIndex::Type Fire_Index;
		ETouchIndex::Type Jump_Index;
		ETouchIndex::Type Boom_Index;
		ETouchIndex::Type CamTurn_Index;
		bool bFireDown=0;
		bool bJumpDown=0;
		bool bBoomDown=0;
		bool bCamTurnDown=0;
	}fingerRecord;
	struct FTouchItem
	{
		bool bIsPressed;
		bool bMoved;
		FVector Location;
	}TouchItem;
	FVector2D CamTurn;
protected:

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	void TouchUpdate(ETouchIndex::Type FingerIndex, FVector Location);
	void CamTurnAtRate(float Rate);
	void CamLookUpAtRate(float Rate);
	UFUNCTION(BlueprintImplementableEvent)
	void LetCamBack();
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

inline void AFPSPracticeCharacter::BeginPlay()
{
	Super::BeginPlay();
	GWorld->GetGameViewport()->GetViewportSize(ScreenSize);
}
