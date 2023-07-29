#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/ProjectileGrenade.h"
#include "Blaster/Weapon/Projectile.h"
#include "SingularityGrenade.generated.h"

/**
 */
UCLASS()
class BLASTER_API ASingularityGrenade : public AProjectileGrenade
{
	GENERATED_BODY()

public:
	ASingularityGrenade();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	UPROPERTY(VisibleAnyWhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	UPROPERTY(EditAnywhere)
	float CharCapsuleRadius = -34;
	/*
	 * Ablility
	 */
	void SpawnSingularity();
	bool bUseGrenateAbility = false;
	TArray<class ABlasterCharacter*> OverlappingCharacters;
	void MoveCharacterToEpicenter(class ABlasterCharacter* Character, float DeltaTime);
	UPROPERTY(EditAnywhere)
	float InterpolationSpeed;
	/*
	 * Collapse Timer
	 */
	void StartCollapseSingularityTimer();
	void CollapseSingularityTimerFinished();
	
	FTimerHandle SingularityTimer;

	UPROPERTY(EditAnywhere)
	float TimeToSpawnSingularity = 3.f;

	/*
	 * ExplosionTimer
	 */

	void StartExplosionTimer();
	void ExplosionTimerFinished();

	FTimerHandle ExplosionTimer;

	UPROPERTY(EditAnywhere)
	float TimeToExplosion = 1.2f;

};
