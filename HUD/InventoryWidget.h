#pragma once

#include "CoreMinimal.h"
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
	void AddItemToInventory(class UTexture2D* SlotImage, int32 Quantity);
	void RemoveItemFromInventory(class UInventorySlot* SlotToRemove);
	void RefreshInventory();

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

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	int32 SlotNumber = 0;
};
