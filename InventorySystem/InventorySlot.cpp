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
	SlotData.SlotAmount = Quantity;
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

		if(SlotData.WeaponType != EWeaponType::EWT_None) RemoveCarriedAmmoAmount(SlotData.WeaponType);
		if(SlotData.GrenadeType != EGrenadeType::EGT_None) RemoveCarriedGrenadesAmount(SlotData.GrenadeType);

		ClearSlotData();
		
		if (InventoryWidget->JustRemovedSlot != nullptr)
		{
			InventoryWidget->RefreshInventory();
		}
	}
}

void UInventorySlot::ClearSlotData()
{
	Thumbnail->SetBrushFromTexture(nullptr);
	SlotQuantity->SetText(FText::FromString(""));

	SetSlotState(ESlotState::ESS_Empty);

	SlotData.SlotAmount = 0;
	bSlotWasCleared = true;
	SlotData.WeaponType = EWeaponType::EWT_None;
	SlotData.GrenadeType = EGrenadeType::EGT_None;
	SlotData.SlotState = ESlotState::ESS_Empty;

	if (SlotData.bIsSlotToModify)
	{
		InventoryWidget->bSlotNoLongerModified = true;
	}
}

void UInventorySlot::RemoveCarriedAmmoAmount(EWeaponType WeaponType)
{
	class ABlasterCharacter* Character = Cast<ABlasterCharacter>(GetOwningPlayerPawn());
	if (Character == nullptr) return;
	if (Character && Character->HasAuthority())
	{
		class UCombatComponent* Combat = Character->GetCombatComponent();
		if (Combat)
		{
			Combat->RemoveCarriedAmmo(WeaponType, SlotData.SlotAmount);//74
		}
	}
}

void UInventorySlot::RemoveCarriedGrenadesAmount(EGrenadeType GrenadeType)
{
	class ABlasterCharacter* Character = Cast<ABlasterCharacter>(GetOwningPlayerPawn());
	if (Character == nullptr) return;
	if (Character && Character->HasAuthority())
	{
		class UCombatComponent* Combat = Character->GetCombatComponent();
		if (Combat)
		{
			Combat->RemoveCarriedGrenades(GrenadeType, SlotData.SlotAmount);//74
		}
	}
}

void UInventorySlot::TransferDataFrom(const FSlotData& Data)
{
	Thumbnail->SetBrushFromTexture(Data.ItemTexture);
	float AmmoPercent = Data.SlotAmount / ProgressMagCapacity;

	/*if (bMagIsFull)
	{
		AmmoPercent = 0.f;
		Quantity =
	}*/
	FString QuantityText = FString::Printf(TEXT("%d"), Data.SlotAmount);
	SlotQuantity->SetText(FText::FromString(QuantityText));

	SlotData.ItemTexture = Data.ItemTexture;
	SlotData.SlotAmount = Data.SlotAmount;
	SlotData.WeaponType = Data.WeaponType;
	SlotData.GrenadeType = Data.GrenadeType;
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
	return SlotData.bSlotIsFull = SlotData.SlotAmount >= MaxSlotQuantity;
}

