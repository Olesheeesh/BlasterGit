// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX  UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	virtual void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void Deactivate();
	void Activate(class ABlasterCharacter* Character);
	//Textures for the weapon crosshairs

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsBottom;

	 /**
	 * Zoomed FOV while aiming
	 */

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = Combat)//when use UPROPERTY, should add default value to variable
	float FireDelay = .13;//shooting speed

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	bool IsEmpty();

	bool isOutOfAmmo();

	bool MagIsFull();

	void AddAmmo(int32 AmmoToAdd);

	void CycleThroughOptics();


	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	UPROPERTY(EditAnywhere)
	USoundCue* OutOfAmmoSound;

	UPROPERTY(EditAnywhere)//should be replicated
	float AimInterpSpeed = 10.f;

	/*
	 * Scope
	 */

	void EquipScope(class AScope* ScopeToEquip);

	/*
	 * Enable/Disable custom Depth
	 */

	void EnableCustomDepth(bool bEnable);

	UPROPERTY(ReplicatedUsing = OnRep_HideWeapon)
	bool bHideWeapon = false;

	UFUNCTION()
	void OnRep_HideWeapon();
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	UPROPERTY(EditAnywhere)
	float DistanceToSight = 30.f;

	/*
	 * Scope
	 */
	UFUNCTION()
	void OnRep_EquippedScope();

	UFUNCTION(Server, Reliable)
	void ServerSetOpticIndex(uint8 CurrentIndex);

	UFUNCTION()
	void OnRep_OpticIndex();

	UPROPERTY()
	class UBlasterAnimInstance* AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scope Properties")
	TArray<AScope*> Optics;

	UPROPERTY(Replicated)
	AScope* CurrentScope;

	UPROPERTY(ReplicatedUsing = OnRep_OpticIndex)
	uint8 OpticIndex = 0;

private:
	UPROPERTY(VisibleAnyWhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnyWhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnyWhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnyWhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;//pointer of type "UAnimationAsset"

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABulletShell> ShellClass;//чтоб появился в редакторе

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;//ёмкость магазина

	UFUNCTION()
	void OnRep_Ammo();
	void SpendRound();

	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;

	UPROPERTY()//на случай если не инициализирован, чтоб не крашнуло
	class ABlasterPlayerController* BlasterOwnerController;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	EWeaponSocketType WeaponSocketType = EWeaponSocketType::EWST_Custom;

public:
	UPROPERTY(ReplicatedUsing = OnRep_EquippedScope)
	class AScope* EquippedScope;

	void SetWeaponState(EWeaponState State);
	FORCEINLINE EWeaponState GetWeaponState() const {  return WeaponState; }
	FORCEINLINE USphereComponent* GetAreaSphere() const {  return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }//weapon mesh is private->need this getter
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE AScope* GetCurrentScope() const { return CurrentScope; }
	FORCEINLINE float GetDistanceToSight() const { return DistanceToSight; }
	FORCEINLINE EWeaponSocketType GetWeaponSocketType() const { return WeaponSocketType; }
};



