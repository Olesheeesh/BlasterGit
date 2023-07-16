#include "InventoryWidget.h"

#include "Blaster/BlaserComponents/InventoryComponent.h"
#include "Blaster/InventorySystem/InventorySlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
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
		CurrentSlot->SetSlotState(ESlotState::ESS_Filled);

		if(SlotNumber < InventorySlots.Num())
		++SlotNumber;
	}
}

void UInventoryWidget::RemoveItemFromInventory(UInventorySlot* SlotToRemove)
{

}

void UInventoryWidget::RefreshInventory()
{
	if (JustRemovedSlot)
	{
		for (int i = JustRemovedSlot->SlotIndex; i < InventorySlots.Num(); ++i)
		{
			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("i = %d"), i));
			class UInventorySlot* SlotToReplace = Cast<UInventorySlot>(InventoryBox->GetChildAt(i));
			if (SlotToReplace)
			{
				int j = i + 1;
				if (j < InventorySlots.Num())
				{
					int q = j + 1;
					class UInventorySlot* NextSlot = Cast<UInventorySlot>(InventoryBox->GetChildAt(j));
					class UInventorySlot* LastSlot = Cast<UInventorySlot>(InventoryBox->GetChildAt(q));//здесь
					if (NextSlot && NextSlot->SlotState == ESlotState::ESS_Filled)
					{
						if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("j = %d"), j));
						if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Next slots, that are filled: %s"), *NextSlot->GetName()));
						SlotToReplace->SetSlotData(NextSlot->ItemTexture, NextSlot->ItemQuantity);
						SlotToReplace->SetSlotState(ESlotState::ESS_Filled);
						if (LastSlot && LastSlot->SlotState == ESlotState::ESS_Empty)
						{
							NextSlot->ClearSlot();
							SlotNumber = j;
						}
					}
					else
					{
						break;
					}
				}
			}
		}
	}
	else { if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("Bad")); }
}


