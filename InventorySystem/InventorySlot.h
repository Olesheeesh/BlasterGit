#pragma once

#include "CoreMinimal.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex;

	ESlotState SlotState;

	UPROPERTY()
	UTexture2D* ItemTexture;

	int32 ItemQuantity = 0;
};
