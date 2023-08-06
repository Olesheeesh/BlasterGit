#include "BlasterHUD.h"

#include "Announcement.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "InventoryWidget.h"
#include "PlayerScore.h"
#include "ScoreBoardWidget.h"
#include "StoreWidget.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)//create overlay widget in BlasterHUD to each characters
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddAnnouncementWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && AnnouncementClass)
	{
		AnnouncementWidget = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		AnnouncementWidget->AddToViewport();
	}
}

void ABlasterHUD::AddScoreBoardWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && ScoreBoardClass)
	{
		ScoreBoardWidget = CreateWidget<UScoreBoardWidget>(PlayerController, ScoreBoardClass);
		ScoreBoardWidget->AddToViewport();
	}
}

void ABlasterHUD::AddPlayerScoreWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && PlayerScoreClass)
	{
		PlayerScoreWidget = CreateWidget<UPlayerScore>(PlayerController, PlayerScoreClass);
		PlayerScoreWidget->AddToViewport();
	}
}

void ABlasterHUD::AddInventoryWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && InventoryWidgetClass)
	{
		InventoryWidget = CreateWidget<UInventoryWidget>(PlayerController, InventoryWidgetClass);
		InventoryWidget->AddToViewport();
	}
}

void ABlasterHUD::AddStoreWidget()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && StoreWidgetClass)
	{
		StoreWidget = CreateWidget<UStoreWidget>(PlayerController, StoreWidgetClass);
		StoreWidget->AddToViewport();
	}
}

void ABlasterHUD::SetGameAndUIInputMode()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
	InputMode.SetHideCursorDuringCapture(true);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}

void ABlasterHUD::SetGameOnlyInputMode()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = false;
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if(GEngine)//before access the game viewport
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2, ViewportSize.Y / 2);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;//задаётся отдаление динамического прицела

		if(HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrossHair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrossHair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrossHair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrossHair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrossHair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}
}

void ABlasterHUD::DrawCrossHair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairsColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f + Spread.Y)
	);
	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f, //ViewportCenter.X - (TextureWidth / 2.f),
		0.f, //ViewportCenter.Y - (TextureHeight / 2.f)
		1.f,
		1.f,
		CrosshairsColor
	);
}
