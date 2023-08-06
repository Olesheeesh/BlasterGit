#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StoreWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UStoreWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemStoreIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemBoxIndex = 0;

	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* AbilitiesBox;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* GrenadesBox;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* UpgradesBox;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* TrapsBox;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* ArmorBox;
};
