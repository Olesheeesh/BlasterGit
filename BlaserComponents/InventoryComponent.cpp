#include "InventoryComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/InventoryWidget.h"
#include "Blaster/InventorySystem/InventorySlot.h"
#include "Blaster/InventorySystem/Items/Item.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/WrapBox.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	//for(auto& Item : DefaultItems)
	//{
		//AddItem(Item);
	//}
}

/*
bool UInventoryComponent::AddItem(UItem* Item)
{
	if (Items.Num() >= Capacity || !Item) return false;
	Item->OwningInventory = this;
	Item->World = GetWorld();

	Items.Add(Item);

	//Update UI
	OnInventoryUpdated.Broadcast();

	return true;//succesfuly added
}

bool UInventoryComponent::RemoveItem(UItem* Item)
{
	if(Item)
	{
		Item->OwningInventory = nullptr;
		Item->World = nullptr;

		Items.RemoveSingle(Item);

		//Update UI
		OnInventoryUpdated.Broadcast();

		return true;//succesfuly added
	}
	return false;//couldn't remove
}*/

