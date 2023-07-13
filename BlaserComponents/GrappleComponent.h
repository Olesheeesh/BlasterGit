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
	FVector FinalTargetLocation;

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

	UPROPERTY()
	class AWeapon* EquippedGrapple;

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
private://функции
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

private: //переменные

	UPROPERTY(EditAnywhere)
	class USoundCue* RopeSound;

	UPROPERTY(EditAnywhere)
	class USoundCue* GrapplingSound;

	UPROPERTY(EditAnywhere, Category = "Hook")
	float MaxGrappleDistance = 2200.f;

	FVector TraceHitResult;

	UPROPERTY()
	TArray<AActor*> GrappleTargets;

	float BestAngle = 0.f;
	float CurrentAngle = 0.f;

	UPROPERTY(EditAnywhere, Category = "Hook")
	UCurveFloat* GrappleRopeCurve;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* GrappleRopeTimeline;

	FUpdateMovementDelegate UpdateMovementDelegate;
	FOnTimelineFloat GrappleRopeTrack;

	bool bShouldLookForTarget = true;

	FOnTimelineEventStatic OnTimelineFinishedCallback;

	UPROPERTY()
	TArray<AActor*> IgnoreActors;

	UPROPERTY(EditAnywhere, Category = "Hook")
	float MaxTargetScanAngle = 30.f;

	UPROPERTY(Replicated)
	FVector CharacterLocation;

	UPROPERTY(Replicated)
	FVector GrappleSocketLocation;

	FGrappleData GrappleData;

	UPROPERTY(EditAnywhere, Category = "Hook")
	float CapsuleRadius = -34.f;

	/*
	 * Timer
	 */

	FTimerHandle GrapplingTimer;

	UPROPERTY(EditDefaultsOnly)
	float GrapplingDelay = 3.0f;//1.9f

	bool bCanGrapple = true;

	void GrapplingTimerFinished();
public:

};


