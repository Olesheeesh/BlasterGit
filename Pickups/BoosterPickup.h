// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Blaster/BlasterTypes/Types.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "BoosterPickup.generated.h"

UCLASS()
class BLASTER_API ABoosterPickup : public APickup
{
	GENERATED_BODY()
public:

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
private:

	UPROPERTY(EditAnywhere, Category = "Booster Properties")
	int32 AmountToRestore = 30;

	UPROPERTY(EditAnywhere, Category = "Booster Properties")
	EBoosterType BoosterType;

public:

};
