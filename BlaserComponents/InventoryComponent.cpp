#include "InventoryComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/InventoryWidget.h"
#include "Blaster/InventorySystem/InventorySlot.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (OwningCharacter && OwningCharacter->BlasterPlayerController)
	{
		BlasterHUD = Cast<ABlasterHUD>(OwningCharacter->BlasterPlayerController->GetHUD());
		if (BlasterHUD)
		{
			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("Im good1"));
			InventoryWidget = BlasterHUD->InventoryWidget;
			if (InventoryWidget)
			{
				if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("Im good2"));
			}
		}
	}
	//for(auto& Item : DefaultItems)
	//{
		//AddItem(Item);
	//}
}

void UInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();

	
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

