#include "InventorySlot.h"

#include "Blaster/BlaserComponents/InventoryComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UInventorySlot::SetSlotData(class UTexture2D* SlotImage, int32 Quantity)
{
	Thumbnail->SetBrushFromTexture(SlotImage);
	SlotQuantity->SetText(FText::FromString("35"));
}

void UInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();

	Character = Cast<ABlasterCharacter>(GetOwningPlayerPawn());
	InventoryComponent = Character->Inventory;
}

void UInventorySlot::ClearSlot()
{
	InventoryComponent->JustRemovedSlot = this;
	RemoveFromParent();
	//Thumbnail->SetBrushFromTexture(nullptr);
	//SlotQuantity->SetText(FText::FromString(""));
}
