#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/HUD/Announcement.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD()); //BlasterHUD = result of the cast
	ServerCheckMatchState();
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PollInit();
	SetHUDTime();
	CheckTimeSyn(DeltaTime);
}

void ABlasterPlayerController::CheckTimeSyn(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);//when changes, it will be reflected on all clients
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

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 AmmoAmount)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;

	if(bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), AmmoAmount);//float -> FString
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::ShowAmmoHUD_Implementation(bool ShowHUD)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount &&
		BlasterHUD->CharacterOverlay->AmmoText &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount &&
		BlasterHUD->CharacterOverlay->Slash;

	if(bHUDValid)
	{
		if (ShowHUD == true)
		{
			BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetVisibility(ESlateVisibility::Visible);
			BlasterHUD->CharacterOverlay->AmmoText->SetVisibility(ESlateVisibility::Visible);
			BlasterHUD->CharacterOverlay->Slash->SetVisibility(ESlateVisibility::Visible);
			BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetVisibility(ESlateVisibility::Visible);
		}
		else {
			BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->AmmoText->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->Slash->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::SetHUDHealth(float CurrentHealth, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay && 
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText;

	if(bHUDValid)
	{
		const float HealthPercent = CurrentHealth / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::RoundToInt(CurrentHealth), FMath::RoundToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDCurrentHealth = CurrentHealth;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmount;

	if(bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));//FloorToInt Converts a float to a nearest less or equal int
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::SetHUDDefeats(int Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsAmount;

	if(bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);//конвертация int в string
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountdownText;

	if (CountdownTime < 0.f)
	{
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
		return;
	}

	if(bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString MatchCountdown = FString::Printf(TEXT("%02d:%02d "), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(MatchCountdown));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->AnnouncementWidget &&
		BlasterHUD->AnnouncementWidget->WapmupTimeText;

	if(bHUDValid)
	{
		if(CountdownTime < 0.f)
		{
			BlasterHUD->AnnouncementWidget->WapmupTimeText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;//if CountdownTime == 125 -> Seconds = 5

		FString MatchCountDown = FString::Printf(TEXT("%d:%d"), Minutes, Seconds);
		BlasterHUD->AnnouncementWidget->WapmupTimeText->SetText(FText::FromString(MatchCountDown));
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = WarmupTime + MatchTime + CooldownTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);//"120"(MatchTime) - one second, every second

	FString ToDisplay = FString::Printf(TEXT("Seconds left: %d"), SecondsLeft);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, ToDisplay);

	if(HasAuthority())
	{
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if(BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);

			FString ToDisplay2 = FString::Printf(TEXT("Server_Seconds left: %d"), SecondsLeft);
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3, FColor::Yellow, ToDisplay2);
		}
	}

	if(CountdownInt != SecondsLeft)//use to update time not every frame, but every time it changes(every second)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		else if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::PollInit()
{
	if(CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{	
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDCurrentHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
				ShowAmmoHUD(false);
			}
		}
	}
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	//сервер получает пакет
	float TimeServerRecievedClientRequest = GetWorld()->GetTimeSeconds();//локальное время, когда сервер получил запрос от клиента(оно совпадает с TimeOfClientRequest)

	//сервер отправляет ответ на запрос
	ClientReportServerTime(TimeOfClientRequest, TimeServerRecievedClientRequest);//TimeofClientRequest - время клиента в момент отправки запроса
}

//обработка ответа сервера на запрос, который был отправлен из функции ServerRequestServerTime
void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecievedClientRequest)
{
	//round-trip time - разницу между временем, когда был получен ответ от сервера, и временем, когда был отправлен запрос на сервер.
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;//текущее локальное время клиента (минус) время клиента в момент отправки запроса

	//calculate server's current time
	float CurrentServerTime = TimeServerRecievedClientRequest + (RoundTripTime * 0.5);//время за сколько сервер получил запрос

	//Время на сервере всегда будет впереди, поэтому, чтобы определить задержку, отнимаем от текущего времени сервера текущее время клиента.
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();//(задержка)разница между текущим временем сервера и временем клиента, используется для коррекции времени на клиенте.

	//now playerController on the client knows the difference between server's starting time and client's starting time
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ReceivedPlayer()//вызывается на сервере, когда новый игрок присоединяется к игре
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		UE_LOG(LogTemp, Error, TEXT("GetTimeSeconds in ReceivedPlayer: %f"), GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->AddCharacterOverlay();
		if (BlasterHUD->AnnouncementWidget)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if(BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();//get rid of CharacterOverlay
		bool bHUDValid = BlasterHUD->AnnouncementWidget && 
			BlasterHUD->AnnouncementWidget->AnnouncementText && 
			BlasterHUD->AnnouncementWidget->InfoText;

		if(bHUDValid)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
			FString Announcement("New Match Starts In: ");
			BlasterHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(Announcement));
			BlasterHUD->AnnouncementWidget->InfoText->SetText(FText());
		}
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);
	}
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;
	OnMatchStateSet(MatchState);
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncementWidget();//here because in WaitingToStart, BlasterHUD is not exist
	}
	if(BlasterHUD && MatchState == MatchState::Cooldown)
	{
		BlasterHUD->AddAnnouncementWidget();
	}
}