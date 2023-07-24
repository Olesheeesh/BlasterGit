#include "AmmoPickup.h"

#include "Blaster/BlaserComponents/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor);
	if(Character)
	{
		UCombatComponent* Combat = Character->GetCombatComponent();
		if(Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount);
			FString RoleStr;
			switch (Character->GetLocalRole())
			{
			case ROLE_None:
				RoleStr = TEXT("None");
				break;
			case ROLE_SimulatedProxy:
				RoleStr = TEXT("SimulatedProxy");
				break;
			case ROLE_AutonomousProxy:
				RoleStr = TEXT("AutonomousProxy");
				break;
			case ROLE_Authority:
				RoleStr = TEXT("Authority");
				break;
			case ROLE_MAX:
				RoleStr = TEXT("Unknown");
				break;
			}
		}
	}
	Destroy();
}
