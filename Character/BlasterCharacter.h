// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;// Called to bind functionality to input
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();
	virtual void OnRep_ReplicatedMovement() override;
	virtual void Destroyed() override;
	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	void PlayReloadingMontage();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void SprintButtonPressed();
	void SprintButtonReleased();
	void ReloadButtonPressed();
	void SetSprint(bool bIsSprinting);
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	void SimProxiesTurn();
	UFUNCTION(Server, Reliable)
	void ServerSetSprint(bool bIsSprinting);
	UFUNCTION()
	void RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	void UpdateHUDHealth();
	//Poll for any relevant classes and initialize our HUD
	void PollInit();

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCameraa;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combatt;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UPROPERTY(Replicated)
	bool isSprinting;

	UPROPERTY(Replicated)
	float BaseSpeed = 550.f;

	UPROPERTY(Replicated)
	float SprintSpeed = 750.f;

	bool isMovingRight;
	float AO_Yaw; //√оризонтальна€ ось
	float InterpAO_Yaw;
	float AO_Pitch; //¬ертикальна€ ось
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ReloadingMontage;

	void HideCharacterWhenCameraClose();

	UPROPERTY(EditAnywhere)
	float CameraTrashold = 200.f;

	bool bPlayMontage = true;

	bool bRotateRootBone;
	UPROPERTY(EditAnywhere)
	float TurnThreshold = 1.4;//5

	//Proxy
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/*
	 * Player Health
	 */

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float CurrentHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 1.9f;//1.9f

	void ElimTimerFinished();

	/*
	 * Dissolve effect
	 */

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;// - dynamic delegate, designed to handle timeline float track

	UFUNCTION()
	void UpdateDissoveMaterial(float DissolveValue);

	void StartDissolve();

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;//curve

	//Dynamic Instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance2;

	//Dynamic instance set on the Blueprint, used with a dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance2;

	/*
	 * Elim bot
	 */

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;//нова€ переменна€ типа USoundCue

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;
public:
	float MaxSpeed;
	void UELogInfo(float Value);
	void PrintNetModeAndRole();

	void SetOverlappingWeapon(AWeapon* Weapon);
	bool isWeaponEquipped();
	bool isAiming(); //getter for BlasterCharacter
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; } //getter AO_Yaw
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; } //getter AO_Pitch
	FORCEINLINE float GetBaseSpeed() const { return BaseSpeed; } //use getter because BaseSpeed is a private variable 
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; } //returning what enum:: ETurnInPlace equals
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCameraa; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool isElimmed() const { return bElimmed; }
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;
};



//tо replicate:
//	1.Set UPROPERTY(Replicated)
//	2.Go to CombatComponent.cpp and add DOREPLIFETIME to GetLifeTimeReplicatedProps
//	3.create protected: UFUNCTION(Server, Reliable) with Server foo
//  4.Declare with *_implementation
//  Relicated - значит что все клиенты будут значть значение "переменной"

//a forward declaration is required to let the compiler know that these types exist and can be used in the method signature