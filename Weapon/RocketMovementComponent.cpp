// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"

UProjectileMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(
	const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	//making sure that rocket will continue to move even if RMC registers a hit(beacuse need to register hit with collision box on the rocket class)
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// Rockets should not stop, only explode when their CollisionBox detects a hit
}
