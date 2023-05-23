#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	APawn* OwnerPawn = Cast<APawn>(GetOwner());//casting owner from actor to pawn, so we can get a controller
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());//получаем расположение сокета от куда будут вылетать пули
		FVector Start = SocketTransform.GetLocation();

		TMap<ABlasterCharacter*, uint32> HitMap;
		for (uint32 i = 0; i < NumberOfPellets; ++i)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());//character that was hit
			if (BlasterCharacter && InstigatorController && HasAuthority())//if BlasterCharacter was hit; calculate how many hits were made to BlasterCharacter at server
			{
				if (HitMap.Contains(BlasterCharacter))//if exsists in the TMap
				{
					HitMap[BlasterCharacter]++;//increase BlasterCharacter value
				}
				else//add it to a TMap with 1 hit
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}
			}
			if (ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticle,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()//ImpactNormal is a vector
				);
			}
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					GetWorld(),
					HitSound,
					FireHit.ImpactPoint,
					.5f,
					FMath::FRandRange(-.5f, .5f)//random variation to each impact sound
				);
			}
		}
		for(auto HitPair : HitMap)//apply damage for each ABlasterCharacter in HitsMap based on HitPair.Value
		{
			if (HitPair.Key && InstigatorController && HasAuthority())//Key - return BlasterCharacter pointer
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,//Value - uint32 value
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
		
	}
}
