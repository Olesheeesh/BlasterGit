#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

protected:
	virtual void BeginPlay() override;
	//virtual void InitializeComponent() override;
public:	

	//bool AddItem(class UItem* Item);
	//bool RemoveItem(class UItem* Item);
	//void AddItemToInventory(class UTexture2D* SlotImage, int32 Quantity);
	//void IncreaseSlotNumberByOne();

	//UFUNCTION(BlueprintCallable)
	//void DropItem();

	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<class UInventorySlot*> DefaultItems;

	UPROPERTY()
	class ABlasterCharacter* OwningCharacter;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	class UInventorySlot* JustRemovedSlot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* HeavyAmmoImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* LightAmmoImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* ShtgunAmmoImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* SniperAmmoImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* EnergyAmmoImage;

	UPROPERTY(EditAnywhere)
	class UTexture2D* GrappleChargeImage;
};
