// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScopeType.h"
#include "GameFramework/Actor.h"
#include "Scope.generated.h"

UENUM(BlueprintType)
enum class EScopeState : uint8
{
	ESS_Initial UMETA(DisplayName = "Initial State"),
	ESS_Equipped UMETA(DisplayName = "Equipped"),
	ESS_Dropped UMETA(DisplayName = "Dropped"),

	ESS_MAX UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class BLASTER_API AScope : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AScope();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ShowPickupWidget(bool bShowWidget);

	UPROPERTY(VisibleAnyWhere, Category = "Scope Properties")
	class UStaticMeshComponent* Scope;

	UPROPERTY(VisibleAnyWhere, Category = "Scope Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnyWhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UFUNCTION()
	void OnRep_ScopeState();

	void SetScopeState(EScopeState State);

	UPROPERTY(ReplicatedUsing = OnRep_ScopeState, VisibleAnyWhere, Category = "Scope Properties")
	EScopeState ScopeState;

	UPROPERTY(EditAnywhere)
	EScopeType ScopeType;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void FillOpticsArray();

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

private:
	class AWeapon* EquippedWeapon;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	FORCEINLINE UStaticMeshComponent* GetScope() { return Scope; }
};
