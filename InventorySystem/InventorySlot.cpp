#include "InventorySlot.h"

#include "Blaster/BlaserComponents/CombatComponent.h"
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

	/*if (bMagIsFull)
	{
		AmmoPercent = 0.f;
		Quantity = 
	}*/
	FString QuantityText = FString::Printf(TEXT("%d"), Quantity);
	SlotQuantity->SetText(FText::FromString(QuantityText));

	SlotData.ItemTexture = SlotImage;
	SlotData.SlotAmmo = Quantity;
}

void UInventorySlot::SetSlotQuantity(int32 Quantity)
{
	FString QuantityText = FString::Printf(TEXT("%d"), Quantity);
	SlotQuantity->SetText(FText::FromString(QuantityText));
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
		RemoveCarriedAmmoAmount(SlotData.SlotType);

		SlotData.SlotAmmo = 0;
		bSlotWasCleared = true;
		SlotData.SlotType = EWeaponType::EWT_None;
		SlotData.SlotState = ESlotState::ESS_Empty;

		if(SlotData.bIsSlotToModify)
		{
			InventoryWidget->bSlotNoLongerModified = true;
		}
		if (InventoryWidget->JustRemovedSlot != nullptr)
		{
			InventoryWidget->RefreshInventory();
		}
	}
}

void UInventorySlot::RemoveCarriedAmmoAmount(EWeaponType WeaponType)
{
	class ABlasterCharacter* Character = Cast<ABlasterCharacter>(GetOwningPlayerPawn());
	if (Character == nullptr) return;
	class UCombatComponent* Combat = Character->GetCombatComponent();
	if (Combat)
	{
		Combat->SetCarriedAmmo(WeaponType, SlotData.SlotAmmo);//74
	}
}

void UInventorySlot::TransferDataFrom(const FSlotData& Data)
{
	Thumbnail->SetBrushFromTexture(Data.ItemTexture);
	float AmmoPercent = Data.SlotAmmo / ProgressMagCapacity;

	/*if (bMagIsFull)
	{
		AmmoPercent = 0.f;
		Quantity =
	}*/
	FString QuantityText = FString::Printf(TEXT("%d"), Data.SlotAmmo);
	SlotQuantity->SetText(FText::FromString(QuantityText));

	SlotData.ItemTexture = Data.ItemTexture;
	SlotData.SlotAmmo = Data.SlotAmmo;
	SlotData.SlotType = Data.SlotType;
	SlotData.SlotState = Data.SlotState;
	SlotData.bMximumAmountOfAmmoReached = Data.bMximumAmountOfAmmoReached;
	SlotData.bSlotIsFull = Data.bSlotIsFull;
	SlotData.bIsSlotToModify = Data.bIsSlotToModify;
}

void UInventorySlot::SetSlotState(ESlotState State)
{
	SlotData.SlotState = State;
}

bool UInventorySlot::SlotReachedLimit()
{
	return SlotData.bSlotIsFull = SlotData.SlotAmmo >= MaxSlotQuantity;
}
