#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterCharacter.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD()); //BlasterHUD = result of the cast
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if(BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetCurrentHealth(), BlasterCharacter->GetMaxHealth());
	}
}

void ABlasterPlayerController::SetHUDHealth(float CurrentHealth, float MaxHealth)
{
	if(BlasterHUD)
	{
		bool bHUDValid = BlasterHUD && 
			BlasterHUD->CharacterOverlay && 
			BlasterHUD->CharacterOverlay->HealthBar && 
			BlasterHUD->CharacterOverlay->HealthText;
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if(bHUDValid)
		{
			const float HealthPercent = CurrentHealth / MaxHealth;
			BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
			FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::RoundToInt(CurrentHealth), FMath::RoundToInt(MaxHealth));
			BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
		}
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	if(BlasterHUD)
	{
		bool bHUDValid = BlasterHUD &&
			BlasterHUD->CharacterOverlay &&
			BlasterHUD->CharacterOverlay->ScoreAmount;
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if(bHUDValid)
		{
			FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));//FloorToInt Converts a float to a nearest less or equal int
			BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
		}
	}
}

void ABlasterPlayerController::SetHUDDefeats(int Defeats)
{
	if(BlasterHUD)
	{
		bool bHUDValid = BlasterHUD &&
			BlasterHUD->CharacterOverlay &&
			BlasterHUD->CharacterOverlay->DefeatsAmount;
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if(bHUDValid)
		{
			FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);//конвертация int в string
			BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
		}
	}
}

