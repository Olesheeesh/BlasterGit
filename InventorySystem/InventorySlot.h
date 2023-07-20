#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "InventorySlot.generated.h"

UENUM(BlueprintType)
enum class ESlotState : uint8
{
	ESS_Empty UMETA(DisplayName = "Empty"),
	ESS_Filled UMETA(DisplayName = "Empty")

};

UCLASS()
class BLASTER_API UInventorySlot : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SetSlotData(class UTexture2D* SlotImage, int32 Quantity);

	UFUNCTION(BlueprintImplementableEvent)//bp version
	void OnClearSlot(class ABlasterCharacter* Character);

	UFUNCTION(BlueprintCallable)
	void ClearSlot();

	void TransferDataTo(class UInventorySlot* OtherSlot);

	void SetSlotState(ESlotState State);

	UPROPERTY()
	class UInventoryComponent* InventoryComponent;

	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
	class UInventoryWidget* InventoryWidget;

	UPROPERTY(meta = (BindWidget))
	class UButton* DropButton;

	UPROPERTY(meta = (BindWidget))
	class UOverlay* Overlay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	class UImage* Thumbnail;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* SlotQuantity;

	float ProgressMagCapacity = 30.f;
	bool bMagIsFull = false;
	int32 CurrentMag = 0;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar1;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar2;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex;

	ESlotState SlotState;

	EWeaponType SlotType = EWeaponType::EWT_None;

	UPROPERTY()
	UTexture2D* ItemTexture;

	int32 ItemQuantity = 0;
};
