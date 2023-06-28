// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/GrappleState.h"
#include "Components/ActorComponent.h"
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
private://функции
	UPROPERTY(Replicated)
	EGrappleState GrappleState = EGrappleState::EGS_Retracted;

	
	void TickFiring();
	void TickNearingTarget();
	void TickOnTarget();
	void SetCurrentTarget(class AGrappleTarget* NewTarget);
protected:

	UPROPERTY()
	class AGrappleTarget* CurrentTarget;

	UPROPERTY()
	AGrappleTarget* BestTarget;
private: //переменные
	UPROPERTY(EditAnywhere, Category = "Hook")
	float MaxGrappleDistance = 2200.f;

	FVector TraceHitResult;

	UPROPERTY()
	TArray<AActor*> GrappleTargets;

	float BestAngle = 0.f;
	float CurrentAngle = 0.f;
};
