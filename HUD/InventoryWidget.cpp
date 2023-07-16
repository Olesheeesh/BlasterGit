#include "InventoryWidget.h"

#include "Blaster/BlaserComponents/InventoryComponent.h"
#include "Blaster/InventorySystem/InventorySlot.h"
#include "Components/WrapBox.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();


}

void UInventoryWidget::AddItemToInventory(UTexture2D* SlotImage, int32 Quantity)
{
	if (InventorySlots.Num() > 0)
	{
		CurrentSlot = Cast<UInventorySlot>(InventoryBox->GetChildAt(SlotNumber));//GetChildAt() возвращает указатель на базовый класс UWidget
		CurrentSlot->SetSlotData(SlotImage, Quantity);

		if(SlotNumber < InventorySlots.Num())
		++SlotNumber;
	}
}

void UInventoryWidget::RemoveItemFromInventory(UInventorySlot* SlotToRemove)
{

}


