// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/Types.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 99999.f
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	friend class ABlasterCharacter;
	friend class AScope;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);

	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

	void AttachWeaponToSocket(AWeapon* WeaponToEquip, USkeletalMeshComponent* Mesh);

	UFUNCTION(Client, Reliable)
	void ClientAttachWeaponToSocket(AWeapon* WeaponToEquip, USkeletalMeshComponent* Mesh);

	UFUNCTION(Server, Reliable)
	void ServerAttachWeaponToSocket(AWeapon* WeaponToEquip);

	void Reload();

	UFUNCTION(BlueprintCallable)//calls on all machines
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void GrenadeThrowFinished();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	void FireButtonPressed(bool bPressed);

	void SetAiming(bool bIsAiming);

	void ChoosePrimaryWeapon();

	void ChooseSecondaryWeapon();

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);
	void PickupBooster(EBoosterType BoosterType, int32 AmountToRestore);

	UFUNCTION(Client, Reliable)
	void ClientAddItemToInventory(EWeaponType WeaponType, int32 Quantity);

	UFUNCTION(Client, Reliable)
	void ClientUpdateSlotAmmo();

	int32 SetCarriedAmmo(EWeaponType WeaponType, int32 RemoveAmmoAmount);
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_EquippedWeapon) //to replicate we have to register variable first
	class AWeapon* EquippedWeapon; //variable to store currently equipped weapon

protected:
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	//UFUNCTION()
	//void OnRep_CurrentScope();

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	void SetHUDCrosshairs(float DeltaTime, FHitResult& TraceHitResult);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

	UPROPERTY(Replicated)
	int32 AmmoToReload = 0;

	UFUNCTION(Client, Reliable)
	void SetAmmoToReloadForClient(int32 ToReload);

	UFUNCTION(Server, Reliable)
	void UpdateCarriedAmmo();

	const USkeletalMeshSocket* GetWeaponSocket(USkeletalMeshComponent* SkeletalMesh);
	/*
	 * Aiming and FOV
	 */

	 //Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFov;//90

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float UnZoomInterpSpeed = 20.f;

	UFUNCTION(BlueprintCallable)
	void InterpFOV(float DeltaTime);

	/*
	 * Grenade
	 */

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	void ShowAttachedGrenade(bool bShowGrenade);

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;

private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* PlayerController;

	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY()
	class UInventoryWidget* InventoryWidget;

	UPROPERTY(Replicated) 
	class AWeapon* PrimaryWeapon;

	UPROPERTY(Replicated) 
	class AWeapon* SecondaryWeapon;

	UPROPERTY()
	class AWeapon* AbilityWeapon;

	UPROPERTY(Replicated)
	bool bAiming = false;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed = 400.f;

	bool bFireButtonPressed;

	FVector HitTarget;


	bool bSprinting;

	/*
	 * HUD and Crosshairs
	 */
	FHUDPackage HUDPackage;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	/*
	 * Automatic Fire
	 */

	FTimerHandle FireTimer;

	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UPROPERTY(ReplicatedUsing = OnRep_CarriedGrenade)
	int32 CarriedGrenade;

	//Carried ammo for a currently-equipped weapon
	UFUNCTION()
	void OnRep_CarriedAmmo();

	UFUNCTION()
	void OnRep_CarriedGrenade();

	UFUNCTION(Server, Reliable)
	void GetCarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;
	TMap<EGrenadeType, int32> CarriedGrenadesMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 StartingSingularityGrenadeAmmo = 0;

	void InitializeCarriedAmmo();
	void InitializeCarriedGrenades();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();

	bool bCarriedAmmoIsSet = false;

	bool isOutOfAmmo();

	void PlayOutOfAmmoSound();

	/*
	 * FPS TUTORIAL
	 */

protected:

	UPROPERTY()
	class UBlasterAnimInstance* AnimInstance;


public:	
	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE float GetDefaultFov() const { return DefaultFov; }
	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
	FORCEINLINE bool GetIsAiming() const { return bAiming; }
	FORCEINLINE int32 GetAmmoToReload() const { return AmmoToReload; }

};



