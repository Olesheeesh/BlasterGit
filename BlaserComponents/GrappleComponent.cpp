
#include "GrappleComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Grappling/GrappleTarget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blaster/Blaster.h"
#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

UGrappleComponent::UGrappleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UGrappleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGrappleComponent, GrappleState);
}


void UGrappleComponent::BeginPlay()
{
	Super::BeginPlay();

	
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
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceHitResult = HitResult.ImpactPoint;

		FVector StartLocation = Character->GetMesh()->GetSocketLocation(TEXT("hand_l"));
		FVector EndLocation = StartLocation + Character->GetActorForwardVector() * MaxGrappleDistance + 500;

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel3));//находит только ECC_GrappleTarget

		TArray<AActor*> OverlappingActors;
		UKismetSystemLibrary::SphereOverlapActors(
			GetWorld(),
			StartLocation,
			MaxGrappleDistance,
			ObjectTypes,
			AGrappleTarget::StaticClass(),
			TArray<AActor*>(),
			OverlappingActors
		);
		if (!OverlappingActors.Contains(BestTarget) && BestTarget)
		{
			BestTarget->SetActive(false);
			if(BestTarget == nullptr)
			{
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, FString("Best target is null"));
				BestTarget = nullptr;
			}
			return;
		}

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
					DrawDebugLine(
						GetWorld(),
						StartLocation,
						CurrentTarget->GetActorLocation(),
						FColor::Red,
						false,
						.25f
					);
					FVector NormolizedDistanceToTarget = (CurrentTarget->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();
					FVector CameraFwdVector = Character->GetFollowCamera()->GetForwardVector();
					float Angle = FMath::Acos(FVector::DotProduct(NormolizedDistanceToTarget, CameraFwdVector));

					CurrentAngle = FMath::RadiansToDegrees(Angle);//current angle to the target

					//UE_LOG(LogTemp, Error, TEXT("Current angle: %f"), CurrentAngle);

					if (CurrentAngle < BestAngle || BestTarget == nullptr)//if angle < then current angle - change target
					{
						BestTarget = CurrentTarget;
						BestAngle = CurrentAngle;
						
					}
					
					//UE_LOG(LogTemp, Warning, TEXT("Best angle: %f"), BestAngle);
					SetCurrentTarget(BestTarget);

				}
			}
		}
	}
}

void UGrappleComponent::SetCurrentTarget(class AGrappleTarget* NewTarget)
{
	if(NewTarget == CurrentTarget)
	{
		NewTarget->SetActive(true);
		BestAngle = CurrentAngle;
		FVector StartLocation = Character->GetMesh()->GetSocketLocation(TEXT("hand_l"));
		float Distance = FVector::Distance(CurrentTarget->GetActorLocation(), StartLocation);
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("Distance to Target: %f"), Distance));
	}
	else
	{
		CurrentTarget->SetActive(false);
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





