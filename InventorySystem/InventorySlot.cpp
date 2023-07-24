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

	ItemTexture = SlotImage;
	SlotAmmo = Quantity;
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
		/*for (auto& SlotTaip : InventoryWidget->ExistingItemTypesInInventory)
		{
			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("CS_ST: %s"), *GetName()));
		}*/
		Thumbnail->SetBrushFromTexture(nullptr);
		SlotQuantity->SetText(FText::FromString(""));
		SetSlotState(ESlotState::ESS_Empty);
		RemoveCarriedAmmoAmount(SlotType);
		//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("CS_SlotAmmoWas = %d"), SlotAmmo));
		SlotAmmo = 0;
		
		//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("CS_SlotAmmoNow = %d"), SlotAmmo));
		bSlotWasCleared = true;
		//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("CS_SlotTypeWas = %d"), SlotType));
		SlotType = EWeaponType::EWT_None;
		SlotState = ESlotState::ESS_Empty;
		//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("CS_SlotTypeNow = %d"), SlotType));
		//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("CS_SlotAmmoNow = %d"), SlotAmmo));
		if (InventoryWidget->JustRemovedSlot != nullptr)
		{
			InventoryWidget->RefreshInventory();
		}

		
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("CS_SlotNumber: %d"), InventoryWidget->SlotNumber));
	}
}

void UInventorySlot::RemoveCarriedAmmoAmount(EWeaponType WeaponType)
{
	class ABlasterCharacter* Character = Cast<ABlasterCharacter>(GetOwningPlayerPawn());
	if (Character == nullptr) return;
	class UCombatComponent* Combat = Character->GetCombatComponent();
	if (Combat)
	{
		Combat->SetCarriedAmmo(WeaponType, SlotAmmo);//74
		
	}
}

void UInventorySlot::TransferDataFrom(class UTexture2D* SlotImage, int32 Quantity, EWeaponType Type, ESlotState State, bool MximumAmountOfAmmoReached, bool SlotIsFull, bool IsSlotToModify)
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

	ItemTexture = SlotImage;
	SlotAmmo = Quantity;
	SlotType = Type;
	SlotState = State;
	bMximumAmountOfAmmoReached = MximumAmountOfAmmoReached;
	bSlotIsFull = SlotIsFull;
	bIsSlotToModify = IsSlotToModify;
}

void UInventorySlot::SetSlotState(ESlotState State)
{
	SlotState = State;
}

bool UInventorySlot::SlotReachedLimit()
{
	return bSlotIsFull = SlotAmmo >= MaxSlotQuantity;
}
