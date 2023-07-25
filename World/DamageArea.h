// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageArea.generated.h"

UCLASS()
class BLASTER_API ADamageArea : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADamageArea();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnBoxEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY(EditAnywhere, Category = "KillZone Properties")
	class UBoxComponent* AreaBox;

	UPROPERTY(EditAnywhere, Category = "KillZone Properties")
	USkeletalMeshComponent* KillZoneMesh;

	//Timer
	FTimerHandle KillZoneTimer;
	float DamageDelay = 2.f;

	UPROPERTY(EditAnywhere)
	float Damage = 25.f;

	UFUNCTION()
	void UpdateDamage();

	AActor* Actor;

};
