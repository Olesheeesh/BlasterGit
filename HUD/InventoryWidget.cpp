#include "InventoryWidget.h"

#include "Blaster/BlaserComponents/InventoryComponent.h"
#include "Blaster/InventorySystem/InventorySlot.h"
#include "Components/WrapBox.h"
#include "Net/UnrealNetwork.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();


}

void UInventoryWidget::AddItemToInventory(UTexture2D* SlotImage, int32 Quantity, EWeaponType Type)
{
	if (InventorySlots.Num() > 0 && SlotNumber < InventorySlots.Num())
	{
		CurrentSlot = Cast<UInventorySlot>(InventoryBox->GetChildAt(SlotNumber));//GetChildAt() возвращает указатель на базовый класс UWidget
		CurrentSlot->SetSlotData(SlotImage, Quantity);
		CurrentSlot->SetSlotState(ESlotState::ESS_Filled);

		ExistingItemTypesInInventory.Add(Type);
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("IW_Num: %d"), ExistingItemTypesInInventory.Num()));
		CurrentSlot->SlotType = Type;

		if(SlotNumber < InventorySlots.Num())
		++SlotNumber;
	}
}

void UInventoryWidget::RefreshInventory()
{
	if (JustRemovedSlot == nullptr)
	{
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("Null"));
		return;
	}
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
					if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("NextSlot State: %d"), NextSlot->SlotState));
					class UInventorySlot* LastSlot = Cast<UInventorySlot>(InventoryBox->GetChildAt(q));//здесь
					if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("LastSlot State: %d"), LastSlot->SlotState));
					if (NextSlot && NextSlot->SlotState == ESlotState::ESS_Filled)
					{
						/*if (SlotToReplace->SlotType == NextSlot->SlotType)
						{
							SlotToReplace->SlotAmmo += NextSlot->SlotAmmo;
						}*/

						if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("j = %d"), j));
						if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Next slots, that are filled: %s"), *NextSlot->GetName()));
						SlotToReplace->TransferDataFrom(NextSlot->ItemTexture, NextSlot->SlotAmmo, NextSlot->SlotType, NextSlot->SlotState, NextSlot->bMximumAmountOfAmmoReached, NextSlot->bSlotIsFull);
						SlotToReplace->SetSlotState(ESlotState::ESS_Filled);
						if (LastSlot && LastSlot->SlotState == ESlotState::ESS_Empty)
						{
							NextSlot->ClearSlot();
							SlotNumber = j;
							break;
						}
					}
					else
					{
						if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString("Am I calling bruh?"));
						//ExistingItemTypesInInventory.Empty();
						if(LastSlot && LastSlot->SlotState == ESlotState::ESS_Empty)
						{
							if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("Am I calling bruh!"));
						}
						SlotNumber = i;
						break;
					}
				}
			}
		}
	}
	else { if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("Bad")); }
}

UInventorySlot* UInventoryWidget::GetCurrentSlot()
{
	return CurrentSlot = Cast<UInventorySlot>(InventoryBox->GetChildAt(SlotNumber));
}

UTexture2D* UInventoryWidget::SetContentForSlot(EWeaponType WeaponType)
{
	switch(WeaponType)
	{
	case EWeaponType::EWT_AssaultRifle:
		return HeavyAmmoImage;
	case EWeaponType::EWT_SniperRifle:
		return SniperAmmoImage;
	case EWeaponType::EWT_GrapplingHook:
		return GrappleChargeImage;
	case EWeaponType::EWT_GrenadeLauncher:
		return GrenadeAmmoImage;
	case EWeaponType::EWT_Pistol:
		return LightAmmoImage;
	case EWeaponType::EWT_RocketLauncher:
		return RocketAmmoImage;
	case EWeaponType::EWT_SF_Pistol:
		return LightAmmoImage;
	case EWeaponType::EWT_SF_ShotGun:
		return ShotgunAmmoImage;
	case EWeaponType::EWT_ShotGun:
		return ShotgunAmmoImage;
	case EWeaponType::EWT_SubmachineGun:
		return LightAmmoImage;
	}
	if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("Badd"));
	return nullptr;
}



