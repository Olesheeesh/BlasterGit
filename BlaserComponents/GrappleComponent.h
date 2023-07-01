#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/GrappleState.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "GrappleComponent.generated.h"


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
	void StartGrappling();
	UFUNCTION()
	void TimelineFinishedCallback();

private://функции
	void TickFiring();
	void TickNearingTarget();
	void TickOnTarget();
	void SetCurrentTarget(class AGrappleTarget* NewTarget);

	UFUNCTION()
	void UpdateMovement(float Alpha);
protected:
	UPROPERTY(Replicated)
	EGrappleState GrappleState = EGrappleState::EGS_Retracted;

	UPROPERTY()
	class AGrappleTarget* CurrentTarget;

	UPROPERTY()
	AGrappleTarget* BestTarget;

	UPROPERTY(EditAnywhere, Category = "Hook")
	TSubclassOf<class AGrapplingRope> GrapplingRopeClass;

	UPROPERTY(EditAnywhere, Category = "Hook")
	class AGrapplingRope* GrapplingRope;

private: //переменные
	UPROPERTY(EditAnywhere, Category = "Hook")
	float MaxGrappleDistance = 2200.f;

	FVector TraceHitResult;

	UPROPERTY()
	TArray<AActor*> GrappleTargets;

	float BestAngle = 0.f;
	float CurrentAngle = 0.f;

	UPROPERTY(EditAnywhere)
	UCurveFloat* GrappleRopeCurve;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* GrappleRopeTimeline;

	FOnTimelineFloat GrappleRopeTrack;

	bool bShouldLookForTarget = true;
	bool bRopeIsSpawned = false;

	FOnTimelineEventStatic OnTimelineFinishedCallback;

	TArray<AActor*> IgnoreActors;

	UPROPERTY(EditAnywhere)
	float MaxTargetScanAngle = 30.f;
	// UPROPERTY(EditAnywhere)
	// FVector StartTangent = FVector::ZeroVector;
	// UPROPERTY(EditAnywhere)
	// FVector EndTangent = FVector::ZeroVector;
public:
};


