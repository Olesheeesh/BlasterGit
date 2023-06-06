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
#include "Net/UnrealNetwork.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());//ref to chracter
	bRelativeHandIsSet = false;
}

void UBlasterAnimInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBlasterAnimInstance, bRelativeHandIsSet);
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
	EquippedScope = BlasterCharacter->GetEquippedScope();
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
	SetSightTransform();

	if (bWeaponEquipped)
	{
		if (!bRelativeHandIsSet)
		{
			UE_LOG(LogTemp, Error, TEXT("BEFORE bRelativeHandIsSet is: %d"), bRelativeHandIsSet);//false
			SetRelativeHandTransform();
			UE_LOG(LogTemp, Error, TEXT("AFTER bRelativeHandIsSet is: %d"), bRelativeHandIsSet);//false
		}
		if (bAiming)
		{
			InterpAiming(DeltaTime, 1.f);
			if (bInterpRelativeHand)
			{
				InterpRelativeHand(DeltaTime);
			}
		}
		else
		{
			InterpAiming(DeltaTime, 0.f);
		}
	}

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

	if(bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;

		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		
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

		//�� ���� -> ����������� ����
		DrawDebugLine(
			GetWorld(),
			MuzzleTipTransform.GetLocation(),//������ ���������� �����
			MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f,//���� ���������
			FColor::Red
		);
	}
	bUseFabric = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading;//when not reloading, we can use fabric
	bUseAimOffsets = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading && !BlasterCharacter->GetDisableGameplay();
	bTransfromRightHand = BlasterCharacter->GetCombatState() != ECombatState::ECS_Reloading && !BlasterCharacter->GetDisableGameplay();
}


void UBlasterAnimInstance::SetSightTransform()
{
	if (BlasterCharacter && BlasterCharacter->GetFollowCamera() && BlasterCharacter->GetMesh()) {
		FTransform FollowCameraTransform = BlasterCharacter->GetFollowCamera()->GetComponentTransform();
		FTransform FPSMeshTransfrom = BlasterCharacter->GetMesh()->GetComponentTransform();
		FTransform CameraRelativeToArms = FollowCameraTransform.GetRelativeTransform(FPSMeshTransfrom);//make camera relative to arms
		FVector SightForwardVector = CameraRelativeToArms.GetRotation().GetForwardVector();
		FVector SightLocation = SightForwardVector * DistanceToSight + CameraRelativeToArms.GetLocation();//���������� �� �������
		SightTransform.SetLocation(SightLocation);
		SightTransform.SetRotation(CameraRelativeToArms.GetRotation());
	}
}

void UBlasterAnimInstance::SetRelativeHandTransform()
{
	if (BlasterCharacter->GetCombatComponent()->GetCurrentScope() == nullptr) UE_LOG(LogTemp, Error, TEXT("Current scope is null:"));
	if (EquippedScope == nullptr) UE_LOG(LogTemp, Error, TEXT("EquippedScope is null:"));
	if (EquippedScope && BlasterCharacter->GetMesh() == nullptr) UE_LOG(LogTemp, Error, TEXT("Mesh is null:"));//not null

	if (BlasterCharacter->GetCombatComponent()->GetCurrentScope() && EquippedScope && BlasterCharacter->GetMesh())
	{
		FTransform AimSocketTransform = BlasterCharacter->GetCombatComponent()->GetCurrentScope()->GetScope()->GetSocketTransform("AimSocket", ERelativeTransformSpace::RTS_World);
		FTransform HandSocketTransform = BlasterCharacter->GetMesh()->GetSocketTransform("hand_r", ERelativeTransformSpace::RTS_World);
		RelativeHandTransform = AimSocketTransform.GetRelativeTransform(HandSocketTransform);
		bRelativeHandIsSet = true;
		UE_LOG(LogTemp, Error, TEXT("bRelativeHandIsSet is: %d"), bRelativeHandIsSet);
	}
}

void UBlasterAnimInstance::SetFinalHandTransform()
{
	FTransform AimSocketTransform = BlasterCharacter->GetCombatComponent()->GetCurrentScope()->GetScope()->GetSocketTransform("AimSocket", ERelativeTransformSpace::RTS_World);
	FTransform HandSocketTransform = BlasterCharacter->GetMesh()->GetSocketTransform("hand_r", ERelativeTransformSpace::RTS_World);
	FinalHandTransform = AimSocketTransform.GetRelativeTransform(HandSocketTransform);
}

void UBlasterAnimInstance::InterpAiming(float DeltaTime, float Target)
{
	AimAlpha = FMath::FInterpTo(AimAlpha, Target, DeltaTime, EquippedWeapon->AimInterpSpeed);
}

void UBlasterAnimInstance::InterpRelativeHand(float DeltaTime)
{
	RelativeHandTransform = UKismetMathLibrary::TInterpTo(RelativeHandTransform, FinalHandTransform, DeltaTime, 10.f);
	if (RelativeHandTransform.Equals(FinalHandTransform))
	{
		bInterpRelativeHand = false;//�������� �������� bInterpRelativeHand � ���������� CombatComponent, ������� ��������� �� ������� �������
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Here is a problem"));
	}
}


