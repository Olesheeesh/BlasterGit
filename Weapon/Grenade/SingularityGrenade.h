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
	/*
	 * Properties
	 */

	UPROPERTY(EditAnywhere, Category = "Properties")
	class UNiagaraSystem* SingularitySystem;

	UPROPERTY()
	class UNiagaraComponent* SingularitySystemComponent;

	UPROPERTY(EditAnywhere, Category = "Properties")
	float CharCapsuleRadius = -34;

	/*
	 * Ablility
	 */

	UFUNCTION(NetMulticast, Reliable)
	void SpawnSingularity();

	UPROPERTY(EditAnywhere, Category = "Properties")
	USoundCue* SingularitySound;

	bool bUseGrenateAbility = false;

	TArray<class ABlasterCharacter*> OverlappingCharacters;

	void MoveCharacterToEpicenter(class ABlasterCharacter* Character, float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Properties")
	float InterpolationSpeed;

	/*
	 * SpawnSingularity
	 */

	void StartSpawnSingularityTimer();
	void SpawnSingularityTimerFinished();

	FTimerHandle SpawnSingularityTimer;

	UPROPERTY(EditAnywhere, Category = "Properties | Timer")
	float TimeToSpawnSingularity = 1.3f;

	/*
	 * Collapse Singularity Timer
	 */

	void StartCollapseSingularityTimer();
	void CollapseSingularityTimerFinished();
	
	FTimerHandle SingularityTimer;

	UPROPERTY(EditAnywhere, Category = "Properties | Timer")
	float TimeToCollapseSingularity = .5f;

	/*
	 * ExplosionTimer
	 */

	void StartExplosionTimer();
	void ExplosionTimerFinished();

	FTimerHandle ExplosionTimer;

	UPROPERTY(EditAnywhere, Category = "Properties | Timer")
	float TimeToExplosion = 2.f;

};
