// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "Blaster/BlaserComponents/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Weapon/Scopes/Scope.h"
#include "Camera/CameraComponent.h"
#include "Curves/CurveVector.h"
#include "Net/UnrealNetwork.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());//ref to chracter
	
}

void UBlasterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	if (BlasterCharacter )
	{
		OldRotation = BlasterCharacter->GetControlRotation();
	}
}
void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)//tick
{
	Super::NativeUpdateAnimation(DeltaTime);

	if(BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = BlasterCharacter->isWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	if(EquippedWeapon)EquippedScope = EquippedWeapon->EquippedScope;
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bAiming = BlasterCharacter->isAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimmed = BlasterCharacter->isElimmed();
	//CurrentScope = BlasterCharacter->GetCombatComponent()->GetCurrentScope();
	//AimSpeed = EquippedWeapon->AimInterpSpeed;
	/*
	 * Youtube tutorial
	 */

	if (bWeaponEquipped && BlasterCharacter->IsLocallyControlled())
	{
		UpdateMovingCurve(DeltaTime);
		UpdateTurningSway(DeltaTime);
		if (bAiming)
		{
			SetSightTransform();
			InterpAiming(DeltaTime, 1.f);
			if (AimAlpha != 0.0f)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Green, FString::Printf(TEXT("AimAlpha = %f"), AimAlpha));
			}
			if (bInterpRelativeHand)
			{
				InterpRelativeHand(DeltaTime);
			}
		}
		else
		{
			if (AimAlpha != 0.0f)
			{
				InterpAiming(DeltaTime, 0.f);
			}
			//GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Yellow, FString::Printf(TEXT("AimAlpha = %f"), AimAlpha));
		}
	}

	/*
	 * Curve
	 */


	//offset yaw for strafing
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw; 

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if(bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->FPSMesh)
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;

		//выполняет преобразование позиции LeftHandTransform.GetLocation() из мирового пространства в локальное пространство кости "hand_r" на меше персонажа BlasterCharacter->GetMesh(). Результат преобразования сохраняется в переменных OutPosition (локальная позиция) и OutRotation (локальный поворот).
		//преобразуем в пространство hand_r потому, что оружие прикреплено к этой же кости и в таком случае, LeftHandSocket будет передвигаться вместе с оружием
		BlasterCharacter->FPSMesh->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		/*if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = BlasterCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}*/

		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X)); //Vector(x)

		//из дула -> направление дула
		DrawDebugLine(
			GetWorld(),
			MuzzleTipTransform.GetLocation(),//откуда начинается линия
			MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f,//куда стремится
			FColor::Red
		);
	}
	bUseFabric = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;//when not reloading, we can use fabric
	bUseAimOffsets = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
	bTransfromRightHand = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
}


void UBlasterAnimInstance::SetSightTransform()
{
	if (EquippedWeapon && BlasterCharacter && BlasterCharacter->GetFollowCamera() && BlasterCharacter->FPSMesh)
	{
		FTransform FollowCameraTransform = BlasterCharacter->GetFollowCamera()->GetComponentTransform();
		FTransform FPSMeshTransfrom = BlasterCharacter->FPSMesh->GetComponentTransform();
		FTransform CameraRelativeToArms = FollowCameraTransform.GetRelativeTransform(FPSMeshTransfrom);//make camera relative to arms
		FVector SightForwardVector = CameraRelativeToArms.GetRotation().GetForwardVector();
		FVector SightLocation = SightForwardVector * EquippedWeapon->GetDistanceToSight() + CameraRelativeToArms.GetLocation();
		SightTransform.SetLocation(SightLocation);
		SightTransform.SetRotation(CameraRelativeToArms.GetRotation());
		//GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Red, FString::Printf(TEXT("SightLocation.X: %f"), SightLocation.X));
		//GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Red, FString::Printf(TEXT("SightLocation.Y: %f"), SightLocation.Y));
		//GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Red, FString::Printf(TEXT("SightLocation.Z: %f"), SightLocation.Z));
	}
}

void UBlasterAnimInstance::SetRelativeHandTransform()
{
	if(EquippedWeapon && BlasterCharacter->FPSMesh && EquippedScope == nullptr)
	{
		FTransform SightSocketTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform("SightSocket", ERelativeTransformSpace::RTS_World);
		FTransform HandSocketTransform = BlasterCharacter->FPSMesh->GetSocketTransform("hand_r", ERelativeTransformSpace::RTS_World);
		RelativeHandTransform = SightSocketTransform.GetRelativeTransform(HandSocketTransform);
		bRelativeHandIsSet = true;
		if (EquippedScope == nullptr) UE_LOG(LogTemp, Warning, TEXT("Am I in?"));

	}
	else if (EquippedScope && EquippedWeapon && EquippedWeapon->GetCurrentScope())
	{
		FTransform AimSocketTransform = EquippedWeapon->GetCurrentScope()->GetScope()->GetSocketTransform("AimSocket", ERelativeTransformSpace::RTS_World);
		FTransform HandSocketTransform = BlasterCharacter->FPSMesh->GetSocketTransform("hand_r", ERelativeTransformSpace::RTS_World);
		RelativeHandTransform = AimSocketTransform.GetRelativeTransform(HandSocketTransform);
		bRelativeHandIsSet = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SRHT FUCK"));
	}
}

void UBlasterAnimInstance::SetFinalHandTransform()
{
	if (EquippedWeapon && BlasterCharacter->FPSMesh && EquippedScope == nullptr)
	{
		FTransform SightSocketTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform("SightSocket", ERelativeTransformSpace::RTS_World);
		FTransform HandSocketTransform = BlasterCharacter->FPSMesh->GetSocketTransform("hand_r", ERelativeTransformSpace::RTS_World);
		FinalHandTransform = SightSocketTransform.GetRelativeTransform(HandSocketTransform);
		bRelativeHandIsSet = true;
	}
	else if(EquippedWeapon && EquippedScope && EquippedWeapon->GetCurrentScope())
	{
		FTransform AimSocketTransform = EquippedWeapon->GetCurrentScope()->GetScope()->GetSocketTransform("AimSocket", ERelativeTransformSpace::RTS_World);
		FTransform HandSocketTransform = BlasterCharacter->FPSMesh->GetSocketTransform("hand_r", ERelativeTransformSpace::RTS_World);
		FinalHandTransform = AimSocketTransform.GetRelativeTransform(HandSocketTransform);
	}
}

void UBlasterAnimInstance::InterpAiming(float DeltaTime, float Target)
{
	AimAlpha = FMath::FInterpTo(AimAlpha, Target, DeltaTime, EquippedWeapon->AimInterpSpeed);
	//GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Green, FString(TEXT("Calling?")));
}

void UBlasterAnimInstance::InterpRelativeHand(float DeltaTime)
{
	RelativeHandTransform = UKismetMathLibrary::TInterpTo(RelativeHandTransform, FinalHandTransform, DeltaTime, 10.f);
	if (RelativeHandTransform.Equals(FinalHandTransform))
	{
		bInterpRelativeHand = false;
		SetRelativeHandTransform();
		
	}
}

void UBlasterAnimInstance::ChangeOptic()
{
	SetFinalHandTransform();
	bInterpRelativeHand = true;
}

void UBlasterAnimInstance::UpdateMovingCurve(float DeltaTime)
{
	if(MovingCurve)
	{
		float MaxSpeed = BlasterCharacter->GetSprintSpeed();
		Speed = UKismetMathLibrary::NormalizeToRange(Speed, (MaxSpeed / DivideCurveMinRange * -1.0f), MaxSpeed);//изначально NormilizedSpeed = 0, растёт до 1 вместе со скоростью
		FVector NewVector = MovingCurve->GetVectorValue(BlasterCharacter->GetGameTimeSinceCreation());//получаем значение "key" из VectorCurve, исходя из времени
		SwayLocation = UKismetMathLibrary::VInterpTo(SwayLocation, NewVector, DeltaTime, 1.8f);//сдвигаем SwayLocation к значению Curve
		SwayLocation *= Speed;
	}
}

void UBlasterAnimInstance::UpdateTurningSway(float DeltaTime)
{
	FRotator CurrentRotation = BlasterCharacter->GetControlRotation();
	TurnRotation = UKismetMathLibrary::RInterpTo(TurnRotation, CurrentRotation - OldRotation, DeltaTime, 3.5f);//10 - 6 = 4 == CurrentRotation - OldRotation = 4
	TurnRotation.Roll = TurnRotation.Pitch * -1.0f;

	TurnRotation.Yaw = FMath::Clamp(TurnRotation.Yaw, -7.f, 7.f);
	TurnRotation.Roll = FMath::Clamp(TurnRotation.Roll, -3.f, 3.f);

	OldRotation = CurrentRotation;
	//UE_LOG(LogTemp, Warning, TEXT("TurnRotation.Roll: %f"), TurnRotation.Roll);
	//UE_LOG(LogTemp, Warning, TEXT("TurnRotation.Yaw: %f"), TurnRotation.Yaw);
}
