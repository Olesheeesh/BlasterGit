#include "InventorySlot.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/InventoryWidget.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UInventorySlot::SetSlotData(class UTexture2D* SlotImage, int32 Quantity)
{
	Thumbnail->SetBrushFromTexture(SlotImage);
	float AmmoPercent = Quantity / ProgressMagCapacity;
	bMagIsFull = AmmoPercent > 1.f;

	/*if (bMagIsFull)
	{
		AmmoPercent = 0.f;
		Quantity = 
	}*/
	FString QuantityText = FString::Printf(TEXT("%d"), Quantity);
	SlotQuantity->SetText(FText::FromString(QuantityText));

	ItemTexture = SlotImage;
	ItemQuantity = Quantity;
}

void UInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();

	class ABlasterCharacter* Character = Cast<ABlasterCharacter>(GetOwningPlayerPawn());
	if (Character)
	{
		class ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(Character->Controller);
		if (PlayerController)
		{
			BlasterHUD = Cast<ABlasterHUD>(PlayerController->GetHUD());
			if (BlasterHUD)
			{
				InventoryWidget = BlasterHUD->InventoryWidget;
			}
		}
	}
}


void UInventorySlot::ClearSlot()
{
	if (InventoryWidget)
	{
		InventoryWidget->JustRemovedSlot = this;

		Thumbnail->SetBrushFromTexture(nullptr);
		SlotQuantity->SetText(FText::FromString(""));
		SetSlotState(ESlotState::ESS_Empty);
		if (InventoryWidget->JustRemovedSlot != nullptr)
		{
			FString A = InventoryWidget->JustRemovedSlot->GetName();
			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString(A));
			InventoryWidget->RefreshInventory();
		}
	}
}

void UInventorySlot::TransferDataTo(UInventorySlot* OtherSlot)
{
	OtherSlot->Thumbnail = Thumbnail;
	OtherSlot->SlotQuantity = SlotQuantity;
}

void UInventorySlot::SetSlotState(ESlotState State)
{
	SlotState = State;
}
