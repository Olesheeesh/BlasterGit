#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());//casting owner from actor to pawn, so we can get a controller
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());//position, rotation, scale
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25;// HitTarget - Start: from start to hit target; 1.25: to be sure target is hit

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);
			FVector BeamEnd = End;
			if(FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());//character that was hit
				if(BlasterCharacter && InstigatorController && HasAuthority())//HasAuthority() - call apply damage, only if we are the server
				{
					UGameplayStatics::ApplyDamage(
					BlasterCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
					);
				}
				if(ImpactParticle)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticle,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()//ImpactNormal is a vector
					);
				}
			}
			if(BeamParticle)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticle,
					SocketTransform
				);
				if(Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);//create vector param by name
				}
			}
		}
	}
}
