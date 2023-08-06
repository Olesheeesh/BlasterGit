#pragma once
#include "CoreMinimal.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "UObject/NoExportTypes.h"
#include "Types.generated.h"

UENUM(BlueprintType)//создали не в weapon class'е, чтоб при использовании в других классах, не включать весь класс weapon
enum class EBoosterType : uint8
{
	EBT_None UMETA(DisplayName = "None"),
	EBT_Health UMETA(DisplayName = "Health"),
	EBT_Armor UMETA(DisplayName = "Armor")

};

USTRUCT(BlueprintType)
struct FItemType
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
		EWeaponType WeaponType;
	UPROPERTY(EditAnywhere)
		EGrenadeType GrenadeType;
};