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
	void ServerActivateShift();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastActivateShift();

	UFUNCTION()
	void BlinkAbility();

	UFUNCTION()
	void OnAfterImageSystemFinished(class UNiagaraComponent* NiagaraComponent);

protected://функции
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnAfterImageFinished OnAfterImageFinished;
private:

	UPROPERTY()
	class AWeapon* EquippedWeapon;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

protected://переменные

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Shift")
	FVector ShiftActivationLocation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Shift")
	float ShiftDuration = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Shift")
	float ImpulseStrength = 250.f;

	bool CanShift = true;
	FTimerHandle ShiftTimer;

	void StartShiftTimer();
	void ShiftTimerFinished();
};
