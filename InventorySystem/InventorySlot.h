#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "InventorySlot.generated.h"

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

	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY()
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(meta = (BindWidget))
	class UButton* DropButton;

	UPROPERTY(meta = (BindWidget))
	class UOverlay* Overlay;

	UPROPERTY(meta = (BindWidget))
	class UImage* Thumbnail;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SlotQuantity;


};
