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
	void AddAmmoItemToInventory(UTexture2D* SlotImage, int32 Quantity, EWeaponType InWeaponType);

	UFUNCTION(BlueprintCallable)
	void AddGrenadeItemToInventory(UTexture2D* SlotImage, int32 Quantity, EGrenadeType InGrenadeType);

	void RefreshInventory();
	UTexture2D* SetAmmoContentForSlot(EWeaponType InWeaponType);
	UTexture2D* SetGrenadeContentForSlot(EGrenadeType InGrenadeType);

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
	TArray<EWeaponType> ExistingAmmoTypesInInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<EGrenadeType> ExistingGrenadeTypesInInventory;

	int32 SlotNumber = 0;

	bool bNewItemAdded = false;
	bool bTypeOfAmmoRunOut = false;
	bool bSlotNoLongerModified = false;
	/*
	 * Images
	 */
	UPROPERTY(EditAnywhere)
	class UTexture2D* EmptyImage;

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

	/*
	 * Grenades
	 */

	UPROPERTY(EditAnywhere)
	class UTexture2D* SingularityGrenadeImage;

	
	
};
