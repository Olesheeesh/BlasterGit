// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/InventorySystem/Items/Item.h"
#include "HeavyAmmoItem.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UHeavyAmmoItem : public UItem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item | Ammo")
	FText ItemDisplayAmmo;

	void SetAmmoAmount(int32 AmmoAmount);

	EItemType ItemType = EItemType::EIT_HeavyAmmo;

	UTexture2D* AmmoImage;

	int32 AmmoQuantity;
};
