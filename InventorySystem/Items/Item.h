// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_HeavyAmmo UMETA(DisplayName = "Heavy Ammo"),
	EIT_ShotgunAmmo UMETA(DisplayName = "Shotgun Ammo"),
	EIT_SniperAmmo UMETA(DisplayName = "Sniper Ammo"),
	EIT_EnergyAmmo UMETA(DisplayName = "Energy Ammo"),
	EIT_Hook UMETA(DisplayName = "Grapple Charge"),
	EIT_Grenade UMETA(DisplayName = "Grenade")
};

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class BLASTER_API UItem : public UObject
{
	GENERATED_BODY()

public:
	UItem();
	virtual void Drop(class ABlasterCharacter* Character);

	UFUNCTION(BlueprintImplementableEvent)//bp version
	void OnDrop(class ABlasterCharacter* Character);

	class UWorld* World;

	virtual class UWorld* GetWorld() const { return World; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText UseActionText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	class UStaticMesh* PickupMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	class UTexture2D* Thumbnail;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText ItemDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
	FText ItemDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText ItemQuantity;

	UPROPERTY()
	class UInventoryComponent* OwningInventory;

	EItemType ItemType;
};
