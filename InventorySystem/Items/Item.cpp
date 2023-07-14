#include "Item.h"

UItem::UItem()
{
	ItemDisplayName = FText::FromString("Item");
	UseActionText = FText::FromString("Use");
}

void UItem::Drop(class ABlasterCharacter* Character)
{
}

 