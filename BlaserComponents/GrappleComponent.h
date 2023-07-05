#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/GrappleState.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "GrappleComponent.generated.h"

DECLARE_DELEGATE_ThreeParams(FUpdateMovementDelegate, UCurveFloat*, FVector, FVector);

USTRUCT(BlueprintType)
struct FGrappleData
{
	GENERATED_BODY()

	FVector TargetLocation;

	FVector StartLocation;

	FVector StartTangent;

	FVector EndTangent;

	FRotator StartRotation;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UGrappleComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPROPERTY()
	class ABlasterCharacter* Character;

protected:
	virtual void BeginPlay() override;

public:	
	UGrappleComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void TickRetracted();
	void StartHook();

	UFUNCTION(Server, Reliable)
	void ServerStartHook(FVector InStartLoc, FVector InTargetLoc, FVector InStartTarngent, FVector InEndTangent, FRotator InStartRotationy);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartHook(FVector InStartLoc, FVector InTargetLoc, FVector InStartTarngent, FVector InEndTangent, FRotator InStartRotation);

	void StartGrappling();
	UFUNCTION()

	void TimelineFinishedCallback();

	UFUNCTION()
	void UpdateMovement(float Alpha);
private://�������
	void TickFiring();
	void TickNearingTarget();
	void TickOnTarget();
	void SetCurrentTarget(class AGrappleTarget* NewTarget);
	UFUNCTION(Server, Reliable)
	void ServerSetBestTarget(class AGrappleTarget* NewTarget);
	
protected:
	UPROPERTY()
	EGrappleState GrappleState = EGrappleState::EGS_Retracted;

	class AGrappleTarget* CurrentTarget;

	UPROPERTY(Replicated)
	AGrappleTarget* BestTarget;

	UPROPERTY(EditAnywhere, Category = "Hook")
	TSubclassOf<class AGrapplingRope> GrapplingRopeClass;

	UPROPERTY(EditAnywhere, Category = "Hook")
	class AGrapplingRope* GrapplingRope;

private: //����������
	UPROPERTY(EditAnywhere, Category = "Hook")
	float MaxGrappleDistance = 2200.f;

	FVector TraceHitResult;

	TArray<AActor*> GrappleTargets;

	float BestAngle = 0.f;
	float CurrentAngle = 0.f;

	UPROPERTY(EditAnywhere)
	UCurveFloat* GrappleRopeCurve;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* GrappleRopeTimeline;

	FUpdateMovementDelegate UpdateMovementDelegate;
	FOnTimelineFloat GrappleRopeTrack;

	bool bShouldLookForTarget = true;

	FOnTimelineEventStatic OnTimelineFinishedCallback;

	TArray<AActor*> IgnoreActors;

	UPROPERTY(EditAnywhere)
	float MaxTargetScanAngle = 30.f;


	UPROPERTY(Replicated)
	FVector CharacterLocation;

	FGrappleData GrappleData;
	/*
	 * FVectors
	 */

	// UPROPERTY(EditAnywhere)
	// FVector StartTangent = FVector::ZeroVector;
	// UPROPERTY(EditAnywhere)
	// FVector EndTangent = FVector::ZeroVector;
public:

};


