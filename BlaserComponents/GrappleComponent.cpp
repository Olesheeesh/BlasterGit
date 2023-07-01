
#include "GrappleComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Grappling/GrappleTarget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blaster/Blaster.h"
#include "Blaster/Grappling/GrapplingRope.h"
#include "Camera/CameraComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Engine/SplineMeshActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

UGrappleComponent::UGrappleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	GrappleRopeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("RopeTimeLineComponent"));
	
}

void UGrappleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGrappleComponent, GrappleState);
}

void UGrappleComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character && Character->ChildActor)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Character Target: %s"), *Character->ChildActor->GetChildActor()->GetName()));
		IgnoreActors.Add(Character->ChildActor->GetChildActor());
		for (AActor* IgnoreTarget : IgnoreActors)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Ignore Targets: %s"), *IgnoreTarget->GetName()));
			BestTarget = nullptr;
		}
	}
}

void UGrappleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (GrappleState)
	{
	case EGrappleState::EGS_Retracted:
		TickRetracted();
	case EGrappleState::EGS_Firing:
		TickFiring();
	case EGrappleState::EGS_NearingTarget:
		TickNearingTarget();
	case EGrappleState::EGS_OnTarget:
		TickOnTarget();
	}
}

void UGrappleComponent::TickRetracted()
{
	if (!bShouldLookForTarget) return;
	if (Character && Character->IsLocallyControlled() && IgnoreActors.Num() > 0)
	{
		FHitResult HitResult;
		TraceHitResult = HitResult.ImpactPoint;

		FVector StartLocation = Character->GetMesh()->GetSocketLocation(TEXT("hand_l"));

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
		/*if (!OverlappingActors.Contains(BestTarget) && BestTarget)
		{
			BestTarget->SetActive(false);
			BestTarget = nullptr;
			if(BestTarget == nullptr)
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Best target is null"));
			}
			return;
		}*/

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
						BestAngle = CurrentAngle;//возможно не нужно
						
					}
					
					UE_LOG(LogTemp, Warning, TEXT("Best angle: %f"), BestAngle);
					SetCurrentTarget(BestTarget);
					if (BestTarget == nullptr)
					{
						if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Best target is null"));
					}
				}
			}
		}
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Pizda naxyi2"));
		return;
	}
}

void UGrappleComponent::SetCurrentTarget(class AGrappleTarget* NewTarget)
{
	if (NewTarget == CurrentTarget)
	{
		NewTarget->SetActive(true);
		BestAngle = CurrentAngle;

	}
	else
	{
		CurrentTarget->SetActive(false);
		CurrentTarget = nullptr;
	}
	if(BestAngle > MaxTargetScanAngle)
	{
		NewTarget->SetActive(false);
		BestTarget = nullptr;
	}
}

void UGrappleComponent::StartHook()
{
	if (BestTarget == nullptr) return;
	if (Character && BestTarget)
	{
		if (BestTarget == nullptr) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Target is null"));
		if(GrapplingRope)
		{
			GrapplingRope->Destroy();
			GrapplingRope = nullptr;
		}
		bShouldLookForTarget = false;

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString::Printf(TEXT("bShouldLookForTarget: %d"), bShouldLookForTarget));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString("Im calling"));
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName("GrappleSocket");
		if (HandSocket == nullptr)
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Pizda naxyi"));
			return;
		}
		if (HandSocket)
		{
			FTransform SocketTransform = HandSocket->GetSocketTransform(Character->GetMesh());

			FVector StartLocation = SocketTransform.GetLocation();
			FVector TargetLocation = BestTarget->GetActorLocation();
			FVector StartTangent(0,0,0);
			FVector EndTangent(0,0,0);

			DrawDebugSphere(GetWorld(), StartLocation, 30.f, 15, FColor::Purple, false, 1.f);
			GrapplingRope = GetWorld()->SpawnActor<AGrapplingRope>(GrapplingRopeClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			//GrapplingRope->AttachToActor(Character, FAttachmentTransformRules::KeepWorldTransform);

			GrapplingRope->SetPoints(TargetLocation, StartLocation, StartTangent, EndTangent);
			bRopeIsSpawned = true;
			StartGrappling();
		}
	}
}

void UGrappleComponent::StartGrappling()
{
	GrappleRopeTrack.BindDynamic(this, &UGrappleComponent::UpdateMovement);
	if(GrappleRopeCurve && GrappleRopeTimeline)
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
		FVector CharacterLocation = Character->GetActorLocation();
		FVector TargetLocation = BestTarget->GetActorLocation();
		//TargetLocation.Z -= 134.f;
		if (CharacterLocation == TargetLocation) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Emerald, FString("Im here"));
		FVector NewLocation = FMath::Lerp(CharacterLocation, TargetLocation, Alpha);

		Character->SetActorLocation(NewLocation);
	}
}

void UGrappleComponent::TimelineFinishedCallback()
{
	Character->GetCharacterMovement()->StopMovementImmediately();
	bShouldLookForTarget = true;
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("bShouldLookForTarget: %d"), bShouldLookForTarget));
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





