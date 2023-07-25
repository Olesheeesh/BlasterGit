#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPlatform.generated.h"

UCLASS()
class BLASTER_API APickupSpawnPlatform : public AActor
{
	GENERATED_BODY()

public:	
	APickupSpawnPlatform();

protected:

	virtual void BeginPlay() override;
	UClass* GetRandomPickupType();

	UFUNCTION(NetMulticast, Reliable)
	void SpawnPickup(UClass* Pickup);

	UPROPERTY(EditAnywhere)
	class USceneComponent* Scene;

	UPROPERTY(EditAnywhere, Category = "Platform Properties")
	UStaticMeshComponent* PlatformMesh;

	UPROPERTY()
	TSubclassOf<class APickup> PickupToSpawn;

	UPROPERTY(EditAnywhere, Category = "Platform Properties")
	TArray <TSubclassOf<class APickup>> SpawnablePickups;

	/*
	 * Timer
	 */

	FTimerHandle SpawnPickupTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Platform Properties")
	float SpawnDelay = 5.0f;//1.9f

	bool bCanGrapple = true;

	void SpawnPickupTimerFinished();

	
public:	

};
