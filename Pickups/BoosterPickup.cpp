#include "BoosterPickup.h"

#include "Blaster/BlaserComponents/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

void ABoosterPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor);
	if (Character)
	{
		UCombatComponent* Combat = Character->GetCombatComponent();
		if (Combat)
		{
			Combat->PickupBooster(BoosterType, AmountToRestore);
		}
	}


	Destroy();
}
