// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Components/SphereComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "DrawDebugHelpers.h"
#include "ParameterCollection.h"
#include "Camera/CameraComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if(Character->GetFollowCamera())
	{
		DefaultFov = Character->GetFollowCamera()->FieldOfView;
		CurrentFOV = DefaultFov;
	}
	if(Character->HasAuthority())
	{
		InitializeCarriedAmmo();
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Character && Character->IsLocallyControlled())//only occur for the local player(such as playing certain sounds or spawning visual effects)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint; //точка попадани€
		SetHUDCrosshairs(DeltaTime, HitResult);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);//when changes, it will be reflected on all clients
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);//replicates only to a client that actively controlled
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::EquipWeapon(class AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	if(EquippedWeapon)//if is not null
	{
		EquippedWeapon->Dropped();
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());//attaching weapon to a socket
	}

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->ShowAmmoHUD(true);
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}

	if(EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation( 
			this,
			EquippedWeapon->EquipSound,
			Character->GetActorLocation()
		);
	}

	if(EquippedWeapon->IsEmpty())
	{
		Reload();
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		HandleReload();
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;//because CombatState is replicated variable, it will be replicated to clients and server.
	HandleReload();
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;

	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if(bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->ShowAmmoHUD(true);
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}

bool UCombatComponent::isOutOfAmmo()
{
	return CarriedAmmo == 0 && EquippedWeapon->GetAmmo() == 0;
}

void UCombatComponent::PlayOutOfAmmoSound()
{
	if(EquippedWeapon->OutOfAmmoSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->OutOfAmmoSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadingMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 least = FMath::Min(RoomInMag, AmountCarried);
		return FMath::Clamp(RoomInMag, 0, least);
	}

	return 0;
}

void UCombatComponent::OnRep_CombatState()//logic for client
{
	switch(CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
	}
}


bool UCombatComponent::GetIsSprinting(bool isSprinting)
{
	if(Character && Character->GetVelocity().Length() > Character->GetBaseSpeed() + 1.f)
	{
		bSprinting = isSprinting;
	}
	else
	{
		bSprinting = !isSprinting;
	}
	return bSprinting;
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return; // if dont have EquippedWeapon - return out of this function
	if(bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());//how fast zoom when aiming
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFov, DeltaTime, UnZoomInterpSpeed);//EquippedWeapon->GetZoomedFOV()-заменить на CurrentFOV
	}

	if(Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;//дл€ теб€ в аиме
	ServerSetAiming(bIsAiming);//передаЄт инфу серверу и остальным клиентам
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)//реплицирует информацию всем клиентам
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()//client
{
	if(EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());//attaching weapon to a socket
		}

		if (EquippedWeapon->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				EquippedWeapon->EquipSound,
				Character->GetActorLocation()
			);
		}

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime, FHitResult& TraceHitResult)
{
	if (Character == nullptr || Character->Controller == nullptr) return;//so we can access controller via character

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller; //if Controller !null -> equel to itself
	if (Controller)//check if is valid
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;//if HUD !null -> equel to itself(we are sure that our HUD is set)(дл€ подстраховки/дл€ избежани€ багов)
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			//Calculate crosshair spread

			//[0, 600] -> [0, 1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 5.f);//прицел расшир€етс€ в воздухе
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);//возвращаетс€ в исходное сост.
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0, DeltaTime, 20.f);

			HUDPackage.CrosshairSpread =
				0.65f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;

			if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
			{
				HUDPackage.CrosshairsColor = FLinearColor::Red;
				HUDPackage.CrosshairSpread -= 0.15f;
			}
			else
			{
				HUDPackage.CrosshairsColor = FLinearColor::White;
			}

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}


void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult) //tracing projectile
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); //Set crosshair position to center
	FVector CrosshairWorldPosition;//пустой вектор
	FVector CrosshairWorldDirection;//пустой вектор
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,//заполн€ем вектор 3D координатами
		CrosshairWorldDirection//заполн€ем вектор 3D координатами
	);

	if(bScreenToWorld)//if DeprojectScreenToWorld was success
	{
		FVector Start = CrosshairWorldPosition;//–асположение прицела

		if(Character)//фикс бага, когда прицел сталкиваетс€ с объектами за спиной у character
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Length();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f); //Ќачало трейсирвки спереди от character
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;//длина луча

		GetWorld()->LineTraceSingleByChannel(//perform lineTrace
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed && EquippedWeapon && GetIsSprinting(false))
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (EquippedWeapon == nullptr || GetIsSprinting(true)) return;

	if (isOutOfAmmo())
	{
		PlayOutOfAmmoSound();
	}

	if (CanFire())
	{
		ServerFire(HitTarget);
		if (EquippedWeapon)
		{
			bCanFire = false;
			CrosshairShootingFactor = 0.75f;
		}
		StartFireTimer();
	}
}
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished() 
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic) //if weapon is automatic
	{
		Fire();
	}
	if(EquippedWeapon->IsEmpty())
	{
		Reload();
	}
	
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;//если есть патроны - true
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);//Emplace() ~= add, but avoid any temporaries.  EWT_AssaultRifle value = 30
}




