
#include "GrappleComponent.h"

#include "CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Grappling/GrappleTarget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blaster/Blaster.h"
#include "Blaster/Grappling/GrapplingRope.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/SplineMeshActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

UGrappleComponent::UGrappleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	GrappleRopeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("RopeTimeLineComponent"));
	
}

void UGrappleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME(UGrappleComponent, GrappleState); будет OnRep
	DOREPLIFETIME(UGrappleComponent, BestTarget);
	DOREPLIFETIME_CONDITION(UGrappleComponent, CharacterLocation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UGrappleComponent, GrappleSocketLocation, COND_SkipOwner);
}

void UGrappleComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character && Character->ChildActor)
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Character Target: %s"), *Character->ChildActor->GetChildActor()->GetName()));
		IgnoreActors.Add(Character->ChildActor->GetChildActor());
		for (AActor* IgnoreTarget : IgnoreActors)
		{
			//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Ignore Targets: %s"), *IgnoreTarget->GetName()));
			BestTarget = nullptr;
		}
	}
}

void UGrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickRetracted();
	/*switch (GrappleState)
	{
	case EGrappleState::EGS_Retracted:
		TickRetracted();
	case EGrappleState::EGS_Firing:
		TickFiring();
	case EGrappleState::EGS_NearingTarget:
		TickNearingTarget();
	case EGrappleState::EGS_OnTarget:
		TickOnTarget();
	}*/
}

void UGrappleComponent::TickRetracted()
{
	if (!bShouldLookForTarget) return;
	if (Character && Character->IsLocallyControlled())
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString("Tick is calling"));
		FHitResult HitResult;
		TraceHitResult = HitResult.ImpactPoint;

		if (Character->GetEquippedWeapon() && Character->GetEquippedWeapon()->GetWeaponMesh() && Character->GetEquippedWeapon()->GetWeaponType() == EWeaponType::EWT_GrapplingHook)
		{
			USkeletalMeshComponent* GrappleWeaponMesh = Character->GetEquippedWeapon()->GetWeaponMesh();

			FVector StartLocation = GrappleWeaponMesh->GetSocketLocation(TEXT("GrappleSocket"));

			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel3));//находить только ECC_GrappleTarget

			TArray<AActor*> OverlappingActors;

			UKismetSystemLibrary::SphereOverlapActors(
				GetWorld(),
				StartLocation,
				MaxGrappleDistance,
				ObjectTypes,
				AGrappleTarget::StaticClass(),
				IgnoreActors,
				OverlappingActors
			);

			for (auto Target : OverlappingActors)
			{
				CurrentTarget = Cast<AGrappleTarget>(Target);

				GrappleTargets.Add(CurrentTarget);
				if (CurrentTarget)
				{

					GetWorld()->LineTraceSingleByChannel(
						HitResult,
						StartLocation,
						CurrentTarget->GetActorLocation(),
						ECollisionChannel::ECC_Visibility
					);
					if (HitResult.bBlockingHit)
					{
						/*DrawDebugLine(
							GetWorld(),
							StartLocation,
							CurrentTarget->GetActorLocation(),
							FColor::Red,
							false,
							.25f
						);*/
						FVector NormalizedDistanceToTarget = (CurrentTarget->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
						FVector CameraFwdVector = Character->GetFollowCamera()->GetForwardVector();
						float Angle = FMath::Acos(FVector::DotProduct(NormalizedDistanceToTarget, CameraFwdVector));

						CurrentAngle = FMath::RadiansToDegrees(Angle);//current angle to the target

						//UE_LOG(LogTemp, Error, TEXT("Current angle: %f"), CurrentAngle);

						if (CurrentAngle < BestAngle || BestTarget == nullptr)//if angle < then current angle - change target
						{
							BestTarget = CurrentTarget;
							//BestAngle = CurrentAngle;//скорее всего не нужно
							if (!Character->HasAuthority())
							{
								ServerSetBestTarget(BestTarget);
							}
						}

						UE_LOG(LogTemp, Warning, TEXT("Best angle: %f"), BestAngle);
						SetCurrentTarget(BestTarget);
						if (BestTarget == nullptr)
						{
							//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Best target is null"));
						}
					}
				}
			}
		}
		else
		{
			return;
		}
	}
}

void UGrappleComponent::ServerSetBestTarget_Implementation(class AGrappleTarget* NewTarget)
{
	BestTarget = NewTarget;
}

void UGrappleComponent::SetCurrentTarget(class AGrappleTarget* NewTarget)//локалько задаёт какая из целей будет видна
{
	if (NewTarget == CurrentTarget)
	{
		BestTarget->SetActive(true);
		BestAngle = CurrentAngle;
	}
	else
	{
		CurrentTarget->SetActive(false);
		CurrentTarget = nullptr;
	}
	if (BestAngle > MaxTargetScanAngle)
	{
		NewTarget->SetActive(false);
		BestTarget = nullptr;
	}
}


void UGrappleComponent::StartHook()
{
	if (BestTarget == nullptr)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("BestTarget is null"));
		return;
	}
	if (Character && BestTarget && bCanGrapple)
	{
		bCanGrapple = false;

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString::Printf(TEXT("StartHook bShouldLookForTarget: %d"), bShouldLookForTarget));
		if (BestTarget == nullptr) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Target is null"));
		if (GrapplingRope)
		{
			GrapplingRope->Destroy();
			GrapplingRope = nullptr;
		}
		bShouldLookForTarget = false;
		
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString::Printf(TEXT("bShouldLookForTarget: %d"), bShouldLookForTarget));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString("StartHook is calling"));

		if (Character->GetEquippedWeapon()->GetWeaponType() == EWeaponType::EWT_GrapplingHook && Character->GetEquippedWeapon() && Character->GetEquippedWeapon()->GetWeaponMesh())
		{
			USkeletalMeshComponent* GrappleWeaponMesh = Character->GetEquippedWeapon()->GetWeaponMesh();
			const USkeletalMeshSocket* GrappleWeaponSocket = GrappleWeaponMesh->GetSocketByName("GrappleSocket");

			if (GrappleWeaponSocket == nullptr || BestTarget == nullptr)
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Pizda naxyi"));
				return;
			}

			if (GrappleWeaponSocket && BestTarget)
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString("Server2 is calling"));
				FTransform GrappleSocketTransform = GrappleWeaponSocket->GetSocketTransform(GrappleWeaponMesh);

				FVector StartLocation = GrappleSocketTransform.GetLocation();
				FVector TargetLocation = BestTarget->GetActorLocation();
				FVector StartTangent = FVector(0, 1, 0);
				FVector EndTangent = FVector(0, 1, 0);
				FRotator StartRotation = GrappleSocketTransform.GetRotation().Rotator();

				FVector FwdVector = Character->GetActorForwardVector() * (CapsuleRadius);

				DrawDebugSphere(GetWorld(), StartLocation, 30.f, 15, FColor::Purple, false, 1.f);
				GrapplingRope = GetWorld()->SpawnActor<AGrapplingRope>(GrapplingRopeClass, StartLocation, StartRotation);
				//GrapplingRope->AttachToActor(Character, FAttachmentTransformRules::KeepWorldTransform);

				GrapplingRope->SetPoints(TargetLocation, StartLocation, StartTangent, EndTangent);

				StartGrappling();
				ServerStartHook(StartLocation, TargetLocation, StartTangent, EndTangent, StartRotation);

				Character->GetWorldTimerManager().SetTimer(
					GrapplingTimer,
					this,
					&UGrappleComponent::GrapplingTimerFinished,
					GrapplingDelay
				);
			}
		}
	}
}

void UGrappleComponent::ServerStartHook_Implementation(FVector InStartLoc, FVector InTargetLoc, FVector InStartTarngent, FVector InEndTangent, FRotator InStartRotation)
{
	MulticastStartHook(InStartLoc, InTargetLoc, InStartTarngent, InEndTangent, InStartRotation);//сюда передать переменные и удалить структуру
}

void UGrappleComponent::MulticastStartHook_Implementation(FVector InStartLoc, FVector InTargetLoc, FVector InStartTarngent, FVector InEndTangent, FRotator InStartRotation)
{
	if (Character && BestTarget)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString("multicast is calling"));
		if (GrapplingRope)
		{
			GrapplingRope->Destroy();
			GrapplingRope = nullptr;
		}

		if (RopeSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				RopeSound,
				InStartLoc
			);
		}

		//DrawDebugSphere(GetWorld(), InStartLoc, 30.f, 15, FColor::Purple, false, 1.f);
		GrapplingRope = GetWorld()->SpawnActor<AGrapplingRope>(GrapplingRopeClass, InStartLoc, InStartRotation);
		//GrapplingRope->AttachToActor(Character, FAttachmentTransformRules::KeepWorldTransform);

		GrapplingRope->SetPoints(InTargetLoc, InStartLoc, InStartTarngent, InEndTangent);

		FVector FwdVector = Character->GetActorForwardVector() * (CapsuleRadius);
		FVector FinalTargetLocation = InTargetLoc + FwdVector;

		GrappleData.StartLocation = InStartLoc;
		GrappleData.TargetLocation = InTargetLoc;
		GrappleData.FinalTargetLocation = FinalTargetLocation;
		GrappleData.StartTangent = InStartTarngent;
		GrappleData.EndTangent = InEndTangent;
		GrappleData.StartRotation = InStartRotation;

		StartGrappling();
	}
}

void UGrappleComponent::StartGrappling()
{
	GrappleRopeTrack.BindDynamic(this, &UGrappleComponent::UpdateMovement);
	if (GrappleRopeCurve && GrappleRopeTimeline)
	{
		GrappleRopeTimeline->AddInterpFloat(GrappleRopeCurve, GrappleRopeTrack);
		OnTimelineFinishedCallback.BindUFunction(this, FName(TEXT("TimelineFinishedCallback")));
		GrappleRopeTimeline->SetTimelineFinishedFunc(OnTimelineFinishedCallback);
		GrappleRopeTimeline->PlayFromStart();
		
	}
}

void UGrappleComponent::UpdateMovement(float Alpha)
{
	if (Character && BestTarget)
	{
		CharacterLocation = Character->GetActorLocation();

		FVector NewLocation = FMath::Lerp(CharacterLocation, GrappleData.FinalTargetLocation, Alpha * 0.3);

		Character->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

		if (Character->GetEquippedWeapon() && Character->GetEquippedWeapon()->GetWeaponMesh())
		{
			USkeletalMeshComponent* GrappleWeaponMesh = Character->GetEquippedWeapon()->GetWeaponMesh();
			const USkeletalMeshSocket* GrappleWeaponSocket = GrappleWeaponMesh->GetSocketByName("GrappleSocket");
			if (GrapplingRope && GrappleWeaponSocket)
			{
				FTransform GrappleSocketTransform = GrappleWeaponSocket->GetSocketTransform(GrappleWeaponMesh);
				GrappleSocketLocation = GrappleSocketTransform.GetLocation();

				FVector NewRopeLocation = FMath::Lerp(GrappleSocketLocation, GrappleData.TargetLocation, Alpha * 0.3);

				GrapplingRope->SetPoints(GrappleData.TargetLocation, NewRopeLocation, FVector::ZeroVector, FVector::ZeroVector);
			}
		}
	}
}

void UGrappleComponent::GrapplingTimerFinished()
{
	bCanGrapple = true;
}

void UGrappleComponent::TimelineFinishedCallback()
{
	if (GrapplingRope == nullptr) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("GrapplingRope is null1"));
	if (GrapplingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			GrapplingSound,
			CharacterLocation
		);
	}
	if (Character && GrapplingRope)
	{
		GrapplingRope->Destroy();
		Character->GetCharacterMovement()->StopMovementImmediately();
		bShouldLookForTarget = true;

		
		if (GrapplingRope == nullptr) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Grappling rope is null2"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("bShouldLookForTarget: %d"), bShouldLookForTarget));
	}
}

void UGrappleComponent::TickFiring()
{
}

void UGrappleComponent::TickNearingTarget()
{
}

void UGrappleComponent::TickOnTarget()
{
}





