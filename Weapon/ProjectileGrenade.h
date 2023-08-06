#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "WeaponTypes.h"
#include "ProjectileGrenade.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

protected:
	AProjectileGrenade();

	virtual void BeginPlay() override;
	virtual void Destroyed() override;


	UFUNCTION()//because its bind dynamic to delegate
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;

	UPROPERTY(EditAnywhere)
	bool bUseDestroyTimer = true;

	UPROPERTY(EditAnywhere)
	UTexture2D* GrenadeImage;

	UPROPERTY(EditAnywhere)
	EGrenadeType GrenadeType;

public:
	FORCEINLINE EGrenadeType GetGrenadeType() const { return GrenadeType; }
	FORCEINLINE UTexture2D* GetGrenadeImage() const { return GrenadeImage; }
};
