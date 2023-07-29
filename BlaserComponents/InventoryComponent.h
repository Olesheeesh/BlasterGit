#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryComponent();

	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<class UInventorySlot*> DefaultItems;

	UPROPERTY()
	class ABlasterCharacter* OwningCharacter;

	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
	class UInventoryWidget* InventoryWidget;

	/*
	 * Gold
	 */

	int32 PlayerGold = 0;

protected:
	virtual void BeginPlay() override;

private:
	
public:

	
};
