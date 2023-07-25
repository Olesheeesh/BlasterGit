#include "PickupSpawnPlatform.h"

#include "Blaster/Pickups/AmmoPickup.h"
#include "Engine/StaticMeshSocket.h"
#include "Kismet/GameplayStatics.h"

APickupSpawnPlatform::APickupSpawnPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(PlatformMesh);

}

void APickupSpawnPlatform::BeginPlay()
{
	Super::BeginPlay();
	if(HasAuthority())
	{
		SpawnPickup(GetRandomPickupType());
	}
}

UClass* APickupSpawnPlatform::GetRandomPickupType()
{
	if (SpawnablePickups.Num() > 0)
	{
		float RandomIndex = FMath::FRandRange(0.f, SpawnablePickups.Num() - 1);
		PickupToSpawn = SpawnablePickups[RandomIndex];
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("RandomIndex = %d"), RandomIndex));
		return PickupToSpawn;
	}
	return {};
}

void APickupSpawnPlatform::SpawnPickup_Implementation(UClass* Pickup)
{
	if(Pickup)
	{
		FTransform PlatformTransform = PlatformMesh->GetComponentTransform();
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<APickup>(
				Pickup,
				PlatformTransform.GetLocation(),
				PlatformTransform.GetRotation().Rotator()
			);
		}
	}
}


