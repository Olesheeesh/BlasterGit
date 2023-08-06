#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Types.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blueprint/UserWidget.h"
#include "StoreSlot.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UStoreSlot : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EGrenadeType GrenadeType;

protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* SlotImage;

	UPROPERTY(meta = (BindWidget))
	class UWrapBox* CheckBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BoxSlotIndex;

public:
	FORCEINLINE EGrenadeType GetGrenadeSlotType() const { return GrenadeType; }
};
