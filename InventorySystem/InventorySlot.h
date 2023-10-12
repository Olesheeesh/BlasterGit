#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Types.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "InventorySlot.generated.h"

UENUM(BlueprintType)
enum class ESlotState : uint8
{
	ESS_Empty UMETA(DisplayName = "Empty"),
	ESS_Filled UMETA(DisplayName = "Filled")

};

USTRUCT(BlueprintType)
struct FSlotData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UTexture2D* ItemTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGrenadeType GrenadeType;

	UPROPERTY(BlueprintReadOnly)
	ESlotState SlotState = ESlotState::ESS_Empty;

	bool bMximumAmountOfAmmoReached = false;

	bool bSlotIsFull = false;

	bool bIsSlotToModify = false;

};

UCLASS()
class BLASTER_API UInventorySlot : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void SetSlotData(class UTexture2D* SlotImage, int32 Quantity);

	void SetSlotQuantity(int32 Quantity);

	UFUNCTION(BlueprintImplementableEvent)//bp version
	void OnClearSlot(class ABlasterCharacter* Character);

	UFUNCTION(BlueprintCallable)
	void ClearSlot();
	void ClearSlotData();

	void RemoveCarriedAmmoAmount(EWeaponType WeaponType);
	void RemoveCarriedGrenadesAmount(EGrenadeType GrenadeType);

	void TransferDataFrom(const FSlotData& MyData);

	void SetSlotState(ESlotState State);

	bool SlotReachedLimit();

	/*Struct*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlotData SlotData;

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

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar1;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar2;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ProgressBar3;

	/*
	 * Values
	 */

	float ProgressMagCapacity = 30.f;
	bool bSlotWasCleared = false;
	int32 CurrentMagAmmo = 0;
	UPROPERTY(EditAnywhere)
	int32 MaxSlotQuantity = 80;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex;

public:
	EWeaponType GetWeaponSlotType() const { return SlotData.WeaponType; }
	EGrenadeType GetGrenadeSlotType() const { return SlotData.GrenadeType; }


	

};
