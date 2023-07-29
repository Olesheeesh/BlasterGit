#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Types.generated.h"

UENUM(BlueprintType)//������� �� � weapon class'�, ���� ��� ������������� � ������ �������, �� �������� ���� ����� weapon
enum class EBoosterType : uint8
{
	EBT_None UMETA(DisplayName = "None"),
	EBT_Health UMETA(DisplayName = "Health"),
	EBT_Armor UMETA(DisplayName = "Armor")

};
