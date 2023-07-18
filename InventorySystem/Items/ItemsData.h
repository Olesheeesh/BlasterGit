#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemsData.generated.h"

USTRUCT(BlueprintType)
struct FHeavyAmmoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "HeavyAmmoData")
	class UTexture2D* ItemImage;

	UPROPERTY(EditAnywhere, Category = "HeavyAmmoData")
	int32 ItemQuantity = 0;
};

USTRUCT(BlueprintType)
struct FLightAmmoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "LightAmmoData")
	class UTexture2D* ItemImage;

	UPROPERTY(EditAnywhere, Category = "LightAmmoData")
	int32 ItemQuantity = 0;
};

USTRUCT(BlueprintType)
struct FShotgunAmmoData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "ShotgunAmmoData")
	class UTexture2D* ItemImage;

	UPROPERTY(EditAnywhere, Category = "ShotgunAmmoData")
	int32 ItemQuantity = 0;
};

USTRUCT(BlueprintType)
struct FGrappleChargeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "GrappleChargeData")
	class UTexture2D* ItemImage;

	UPROPERTY(EditAnywhere, Category = "GrappleChargeData")
	int32 ItemQuantity = 0;
};

USTRUCT(BlueprintType)
struct FGrenadeData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "GrenadeData")
	class UTexture2D* ItemImage;

	UPROPERTY(EditAnywhere, Category = "GrenadeData")
	int32 ItemQuantity = 0;
};