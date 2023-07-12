// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Weapon/Weapon.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	UPROPERTY()
	class ABlasterPlayerController* Controller;
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;
	virtual void NativeBeginPlay() override;

private:
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	ABlasterCharacter* BlasterCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Scope, meta = (AllowPrivateAccess = "true"))
	class AScope* EquippedScope;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Lean;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))//access потому что BlueprintReadOnly нельзя использовать в private
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bElimmed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseFabric;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseAimOffsets;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bTransfromRightHand;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseProceduralAiming;

public:
	/*
	 * True Fps Tutorial
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK | Transform");
	FTransform SightTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK | Transform");
	FTransform RelativeHandTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK | Transform");
	FTransform FinalHandTransform;

	UFUNCTION(BlueprintCallable)
	void SetSightTransform();
	UFUNCTION(BlueprintCallable)
	void SetRelativeHandTransform();
	UFUNCTION(BlueprintCallable)
	void SetFinalHandTransform();

	UPROPERTY(BlueprintReadOnly, Category = Aiming, meta = (AllowPrivateAccess = "true"))
	float AimAlpha = 0.f;

	void InterpAiming(float DeltaTime, float Target);

	void InterpRelativeHand(float DeltaTime);

	void ChangeOptic();

	UPROPERTY()
	AScope* CurrentScope;

	bool bInterpRelativeHand = false;

	bool bRelativeHandIsSet = false;

	const USkeletalMeshSocket* SightSocket;

	bool bScopeIsEquipped;

	UPROPERTY(EditAnywhere)
	class UCurveVector* MovingCurve;//curve

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK | Transform");
	FVector SwayLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK | Transform")
	float SwayInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK | Transform")
	FRotator TurnRotation;

	FRotator OldRotation;

	void UpdateMovingCurve(float DeltaTime);

	void UpdateTurningSway(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK | Transform")
	float DivideCurveMinRange;
//public:

	/*
	 *References
	 */

	

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims");
	FIKProperties IKProperties;

	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	USkeletalMeshComponent* Mesh;*/

	/*
	 * IK VARS
	 */

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims");
	//FTransform CameraTransform;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims");
	//FTransform RelativeCameraTransform;
	
//protected:
	//virtual void SetVars(const float DeltaTime);
	//virtual void CalculateWeaponSway(const float DeltaTime);
};

