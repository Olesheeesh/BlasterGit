// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilityComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAfterImageFinished);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAbilityComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ActicateShiftAbility();

	UFUNCTION(Server, Reliable)
	void ServerActivateShift(FVector Loc, FRotator Rot);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastActivateShift(FVector Loc, FRotator Rot);

	UFUNCTION()
	void BlinkAbility();

protected://функции
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY()
	class ABlasterCharacter* Character;

private:

	UPROPERTY()
	class AWeapon* EquippedWeapon;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* AfterImageSystem;

	UPROPERTY()
	class UNiagaraComponent* NiagaraComponent;

protected://переменные
	UPROPERTY(Replicated)
	FVector ShiftActivationLocation;
	UPROPERTY(Replicated)
	FRotator ShiftActivationRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Shift")
	float ShiftDuration = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Shift")
	float ShiftAbilityDelay = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Shift")
	float ImpulseStrength = 3300.f;

	FVector LaunchDirection;

	UPROPERTY(Replicated)
	bool CanShift = true;

	FTimerHandle ShiftDurationTimer;
	FTimerHandle ShiftDelayTimer;

	void StartShiftTimer();
	void StartShiftDelayTimer();
	void ShiftTimerFinished();
	void ShiftTimerDelayFinished();
public:
	FORCEINLINE UNiagaraComponent* GetNiagaraComp() const { return NiagaraComponent; }
};
