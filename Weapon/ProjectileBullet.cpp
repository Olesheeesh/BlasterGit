#include "ProjectileBullet.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());//cast to ACaracter result of GetOwner()
	if(OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller; //controller of the owning character, who shot this projectile
		if(OwnerController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());//this - this projectile
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
/*
 *		Summary
 * new ProjectileBullet class, based on Projectile
 *
 * HitEvent generated by projectileBullet results in calling ApplyDamage and our character response on the server
   to ApplyDamage by calling RecieveDamage
 */
 
/*
 *		Projectile.cpp
 * OnHit in Projectile is only doing destroy and spawning bullet hole
 *
 */

/*
 *		ProjectileBullet.cpp
 * override on OnHit
 * super:: at the end
 * calling ApplyDamage()
 */

/*
 *		BlasterCharacter.cpp
 *	RecieveDamage(), bound this foo with OnTakeAnyDamage(), only at the server!
 */
