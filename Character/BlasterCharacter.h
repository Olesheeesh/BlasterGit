// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "AbilitySystemInterface.h"
#include <GameplayEffectTypes.h>
#include "BlasterCharacter.generated.h"

UENUM(BlueprintType)
enum class ECollisionSettings : uint8
{
	ECS_CharacterDefault UMETA(DisplayName = "Character Default"),
	ECS_WeaponDefault UMETA(DisplayName = "Weapon Default"),
	ECS_CharacterGhost UMETA(DisplayName = "Character Ghost"),
	ECS_WeaponGhost UMETA(DisplayName = "Weapon Ghost")
};

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (allowPrivateAccess = "true"))
	class UGASComponent* AbilitySystemComponent;

	UPROPERTY()
	class UGASAttributeSet* Attributes;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Abilities", meta = (allowPrivateAccess = "true"))
	TArray<TSubclassOf<class UAbilityComponent>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, Category = Abilities)
	class UNiagaraComponent* ShiftAbilitySystemComponent;

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* TPMesh;

	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* FPSMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (allowPrivateAccess = "true"))
	class UGrappleComponent* GrappleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (allowPrivateAccess = "true"))
	class UInventoryComponent* Inventory;
public:
	// Sets default values for this character's properties
	ABlasterCharacter();

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void InitializeAttributes();//set default values to Attributes
	virtual void GiveAbilities();
	virtual void PossessedBy(AController* NewController) override;//calls on a server
	virtual void OnRep_PlayerState() override;//calls on client

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS", meta = (allowPrivateAccess = "true"));
	TSubclassOf<class UGameplayEffect> DefaultAttributeEffect;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GAS", meta = (allowPrivateAccess = "true"));
	TArray<TSubclassOf<class UGASGameplayAbility>> DefaultGASAbilities;

	friend class UCombatComponent;
	friend class UAbilityComponent;
	friend class UGrappleComponent;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;// Called to bind functionality to input
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();
	virtual void OnRep_ReplicatedMovement() override;
	virtual void Destroyed() override;
	void Elim();

	UFUNCTION(BlueprintCallable, Category = "Items")
	void DropItem(class UItem* Item);

	USkeletalMeshComponent* GetNeededMesh();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	void PlayReloadingMontage();

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)//should implement in character blueprint
	void ShowSniperScopeWidget(bool bShowScope);

	UFUNCTION(BlueprintImplementableEvent)//should implement in character blueprint
	void ShowGrenadeLauncherScopeWidget(bool bShowScope);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	class UCameraComponent* PlayerCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	class UCameraComponent* TPCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	class UCameraComponent* FirstPersonCamera;

	UPROPERTY(EditAnywhere)
	class UChildActorComponent* ChildActor;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AGrappleTarget> GrappleTarget;
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
	void ShowScoreBoardPressed();
	void ShowScoreBoardReleased();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void ChangeOpticButtonPressed();
	void ShiftAbilityButtonPressed();
	void ChangeViewButtonPressed();
	void InitializeHookButtonPressed();

	void EquipFirstWeaponButtonPressed();
	UFUNCTION(Server, Reliable)
	void ServerEquipFirstWeaponButtonPressed();

	void EquipSecondWeaponButtonPressed();
	UFUNCTION(Server, Reliable)
	void ServerEquipSecondWeaponButtonPressed();

	void PlayHitReactMontage();
	void SimProxiesTurn();
	UFUNCTION(Server, Reliable)
	void ServerSetSprint(bool bIsSprinting);
	UFUNCTION()
	void RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	void UpdateHUDHealth();
	//Poll for any relevant classes and initialize our HUD
	void PollInit();
	void RotateInPlace(float DeltaTime);
	bool GetIsSprinting();
	void SetCollisionSettings(ECollisionSettings CurrentCollisionSetting);
private:

	float HighestSpeed = 0.f;

	bool currentbUseControllerRotationYaw = false;

	UPROPERTY(VisibleAnywhere, Category = "Transformation")
	TObjectPtr<USceneComponent> FPScene;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	bool bChangeCamera = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingScope)
	class AScope* OverlappingScope;

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION()
	void OnRep_OverlappingScope(AScope* LastScope);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combatt;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAbilityComponent* Abilitiess;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerChangeOpticButtonPressed();

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

	UPROPERTY(Replicated)
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
	void SetOverlappingScope(AScope* Scope);
	bool isWeaponEquipped();
	bool isAiming(); //getter for BlasterCharacter

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; } //getter AO_Yaw
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; } //getter AO_Pitch
	FORCEINLINE float GetBaseSpeed() const { return BaseSpeed; } //use getter because BaseSpeed is a private variable 
	FORCEINLINE float GetSprintSpeed() const { return SprintSpeed; } //use getter because BaseSpeed is a private variable 
	AWeapon* GetEquippedWeapon();
	AScope* GetEquippedScope();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; } //returning what enum:: ETurnInPlace equals
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return PlayerCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool isElimmed() const { return bElimmed; }
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return Combatt; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
};

//tо replicate:
//	1.Set UPROPERTY(Replicated)
//	2.Go to CombatComponent.cpp and add DOREPLIFETIME to GetLifeTimeReplicatedProps
//	3.create protected: UFUNCTION(Server, Reliable) with Server foo
//  4.Declare with *_implementation
//  Relicated - значит что все клиенты будут значть значение "переменной"

//a forward declaration is required to let the compiler know that these types exist and can be used in the method signature