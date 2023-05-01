// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()//so the reflection system can work with FHUDPackage struct
public:
	UPROPERTY() // ADDED THIS!!!
	UTexture2D* CrosshairsCenter;
	UPROPERTY() // ADDED THIS!!!
	UTexture2D* CrosshairsLeft;
	UPROPERTY() // ADDED THIS!!!
	UTexture2D* CrosshairsRight;
	UPROPERTY() // ADDED THIS!!!
	UTexture2D* CrosshairsTop;
	UPROPERTY() // ADDED THIS!!!
	UTexture2D* CrosshairsBottom;
	float CrosshairSpread;
	FLinearColor CrosshairsColor;
};

UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;//CharacterOverlayClass is a TSubclassOf variable that holds a reference to a subclass of UUserWidget class (allows to select a UUserWidget subclass in the editor for this particular actor)

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<class UUserWidget> AnnouncementClass;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;//reference to an instance of the UCharacterOverlay class

	UPROPERTY()
	class UAnnouncement* AnnouncementWidget;

	void AddCharacterOverlay();

	void AddAnnouncementWidget();
protected:
	virtual void BeginPlay() override;

private:
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	void DrawCrossHair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor);
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }//setter
};
