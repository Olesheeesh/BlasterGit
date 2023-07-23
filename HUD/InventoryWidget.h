#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;//толи это же что и UInventoryWidget()?
public:
	UFUNCTION(BlueprintCallable)
	void AddItemToInventory(class UTexture2D* SlotImage, int32 Quantity, EWeaponType Type);

	void RefreshInventory();
	UTexture2D* SetContentForSlot(EWeaponType WeaponType);

	class UInventorySlot* GetCurrentSlot();

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 Capacity = 10;

	UPROPERTY(meta = (BindWidget))	
	class UWrapBox* InventoryBox;

	UPROPERTY()
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	class UInventorySlot* CurrentSlot;

	UPROPERTY()
	class UInventorySlot* JustRemovedSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<class UInventorySlot*> InventorySlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<EWeaponType> ExistingItemTypesInInventory;

	int32 SlotNumber = 0;

	bool bNewItemAdded = false;
	bool bTypeOfAmmoRunOut = false;
	bool bSlotNoLongerModified = false;
	/*
	 * Images
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* HeavyAmmoImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* LightAmmoImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* ShotgunAmmoImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* SniperAmmoImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* EnergyAmmoImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* GrappleChargeImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* GrenadeAmmoImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* RocketAmmoImage;
};
