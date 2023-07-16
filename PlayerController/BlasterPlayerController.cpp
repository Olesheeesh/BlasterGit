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
#include "Blaster/BlaserComponents/CombatComponent.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/HUD/ScoreBoardWidget.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "TimerManager.h"
#include "Blaster/BlaserComponents/InventoryComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/WrapBox.h"
#include "Kismet/GameplayStatics.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD()); //BlasterHUD = result of the cast
	ServerCheckMatchState();
	RequestPlayerStates();
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
	BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (MatchState == MatchState::Cooldown && BlasterCharacter)
	{
		BlasterCharacter->bDisableGameplay = true;
	}

	if(BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetCurrentHealth(), BlasterCharacter->GetMaxHealth());
	}

	InventoryComponent = BlasterCharacter->FindComponentByClass<UInventoryComponent>();
	InventoryWidget = BlasterHUD->InventoryWidget;
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
		BlasterHUD->AnnouncementWidget->WapmupTimeText &&
		BlasterHUD->AnnouncementWidget->CountdownAnimation;

	if(bHUDValid)
	{
		if(CountdownTime < 0.f)
		{
			BlasterHUD->AnnouncementWidget->WapmupTimeText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;//if CountdownTime == 125 -> Seconds = 5

		FString MatchCountDown = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->AnnouncementWidget->WapmupTimeText->SetText(FText::FromString(MatchCountDown));

		BlasterHUD->AnnouncementWidget->PlayAnimation(BlasterHUD->AnnouncementWidget->CountdownAnimation);
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = WarmupTime + MatchTime + CooldownTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);//"120"(MatchTime) - one second, every second

	if(HasAuthority())//Блок if(HasAuthority()) просто снова добавляет LevelStartingTime для учета дополнительного времени сервера в лобби
	{
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if(BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
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

void ABlasterPlayerController::HideSniperScope()
{
	BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());

	bool bSniperScopeIsValid = BlasterCharacter &&
		BlasterCharacter->IsLocallyControlled() &&
		BlasterCharacter->isAiming() == true &&
		BlasterCharacter->GetCombatComponent()->GetEquippedWeapon() &&
		BlasterCharacter->GetCombatComponent()->GetEquippedWeapon()->GetWeaponType() == EWeaponType::EWT_SniperRifle &&
		BlasterCharacter->GetFollowCamera();

	if (bSniperScopeIsValid)
	{
		BlasterCharacter->ShowSniperScopeWidget(false);
		BlasterCharacter->GetFollowCamera()->SetFieldOfView(SniperZoom);
		UE_LOG(LogTemp, Error, TEXT("Should call"));
		UE_LOG(LogTemp, Warning, TEXT("Current FOV1: %f"), BlasterCharacter->GetFollowCamera()->FieldOfView);
	}
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
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
	/*else if (ScoreBoardWidget == nullptr)
	{
		
	}*/
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

	if(MatchState == MatchState::WaitingToStart)
	{
		HandleWaitingToStart();
	}
	else if(MatchState == MatchState::InProgress)
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
	if (MatchState == MatchState::WaitingToStart)
	{
		HandleWaitingToStart();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}


void ABlasterPlayerController::HandleWaitingToStart()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		bool bHUDValid = BlasterHUD->AnnouncementWidget &&
			BlasterHUD->AnnouncementWidget->InfoText;

		if(bHUDValid)
		{
			BlasterHUD->AnnouncementWidget->InfoText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		if(BlasterHUD->CharacterOverlay == nullptr) BlasterHUD->AddCharacterOverlay();
		if (BlasterHUD->AnnouncementWidget)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		BlasterHUD->AddScoreBoardWidget();

		bool bHUDValid = BlasterHUD->ScoreBoardWidget &&
			BlasterHUD->ScoreBoardWidget->PlayersS &&
			BlasterHUD->AnnouncementWidget &&
			BlasterHUD->AnnouncementWidget->AnnouncementText;

		//Сюда
		if (bHUDValid)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
			FString Announcement("Game Restarts In: ");
			BlasterHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(Announcement));
			CooldownIsHandled = true;
			HideSniperScope();
			ShowScoreBoard();


		}
	}
	BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());

	if (BlasterCharacter && BlasterCharacter->GetCombatComponent())
	{
		BlasterCharacter->bDisableGameplay = true;
		BlasterCharacter->GetCombatComponent()->FireButtonPressed(false);
	}
}

void ABlasterPlayerController::HandleCooldownn()
{
	BlasterHUD = BlasterHUD ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if(BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();//get rid of CharacterOverlay
		bool bHUDValid = BlasterHUD->AnnouncementWidget && 
			BlasterHUD->AnnouncementWidget->AnnouncementText && 
			BlasterHUD->AnnouncementWidget->InfoText &&
			BlasterHUD->AnnouncementWidget->StartingInfoText;

		if(bHUDValid)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
			BlasterHUD->AnnouncementWidget->InfoText->SetVisibility(ESlateVisibility::Visible);//добавить после завершения EndGameTime
			BlasterHUD->AnnouncementWidget->StartingInfoText->SetVisibility(ESlateVisibility::Hidden);

			FString Announcement("Game Restarts In: ");
			BlasterHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(Announcement));

			BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();

			if(BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				FString MostWantedPlayersInfo;
				if(TopPlayers.Num() == 0)
				{
					MostWantedPlayersInfo = FString("No MVPs, u are so damn loosers.. ");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
				{
					MostWantedPlayersInfo = FString("You are an MVP");
				}
				else if(TopPlayers.Num() == 1)
				{
					MostWantedPlayersInfo = FString::Printf(TEXT("MVP IS: \n %s"), *TopPlayers[0]->GetPlayerName());
				}
				else if(TopPlayers.Num() > 1)
				{
					MostWantedPlayersInfo = FString("Most Valuable Players ARE: \n");
					for(auto& MVPlayers : TopPlayers)
					{
						MostWantedPlayersInfo.Append(FString::Printf(TEXT("%s\n"), *MVPlayers->GetPlayerName()));
					}
				}
				BlasterHUD->AnnouncementWidget->InfoText->SetText(FText::FromString(MostWantedPlayersInfo));
			}

		}
	}
	BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());

	if(BlasterCharacter && BlasterCharacter->GetCombatComponent())
	{
		BlasterCharacter->bDisableGameplay = true;
		BlasterCharacter->GetCombatComponent()->FireButtonPressed(false);
	}
}

void ABlasterPlayerController::RequestPlayerStates()
{
	BlasterGameState = BlasterGameState == nullptr ? Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)) : BlasterGameState;

	if (BlasterGameState && HasAuthority())
	{
		BlasterGameState->GetAllPlayerStates();
	}
}

void ABlasterPlayerController::FillScoreBoard()
{
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->ScoreBoardWidget->PlayersS &&
		BlasterHUD->ScoreBoardWidget->PlayersNumText &&
		BlasterHUD->ScoreBoardWidget->MapNameText;

	if (bHUDValid)
	{
		if (!BlasterGameState)
		{
			BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
		}
		if (BlasterGameState && HasAuthority())
		{
			BlasterGameState->GetAllPlayerStates();
		}
		if (BlasterGameState)
		{
			BlasterHUD->ScoreBoardWidget->PlayersS->ClearChildren();

			BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
			BlasterPlayerStates = BlasterGameState->BlasterPlayerStates;
			BlasterHUD->ScoreBoardWidget->PlayersNumText->SetText(FText::AsNumber(BlasterPlayerStates.Num()));
			BlasterHUD->ScoreBoardWidget->MapNameText->SetText(FText::FromString(UGameplayStatics::GetCurrentLevelName(this)));
			if (!BlasterPlayerStates.IsEmpty())
			{
				// создаем виджеты для всех игроков и добавляем их в ScoreBoardWidget
				for (auto& PlayerStat : BlasterPlayerStates)
				{
					UPlayerScore* NewPlayerScoreWidget = CreateWidget<UPlayerScore>(this, BlasterHUD->PlayerScoreClass);
					if (NewPlayerScoreWidget && BlasterCharacter)
					{
						NewPlayerScoreWidget->PlayerName->SetText(FText::FromString(PlayerStat->GetPlayerName()));
						NewPlayerScoreWidget->isDead->SetVisibility(BlasterCharacter->isElimmed() == true ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
						NewPlayerScoreWidget->Kills->SetText(FText::AsNumber(PlayerStat->GetScore()));
						NewPlayerScoreWidget->Deaths->SetText(FText::AsNumber(PlayerStat->GetDefeats()));
						NewPlayerScoreWidget->Ping->SetText(FText::AsNumber(PlayerStat->GetPingInMilliseconds()));//deprecated
						BlasterHUD->ScoreBoardWidget->PlayersS->AddChild(NewPlayerScoreWidget);
					}
				}
			}
			SBIsVisible == true ? StartSBTimer() : StopSBTimer();

			//UE_LOG(LogTemp, Warning, TEXT("ScoreBoardToUpdateTimer started"));
		}
	}
}

void ABlasterPlayerController::StartSBTimer()
{
	GetWorldTimerManager().SetTimer(
		ScoreBoardToUpdateTimer,
		this,
		&ABlasterPlayerController::FillScoreBoard,
		UpdateSBDelay,
		true
	);
}

void ABlasterPlayerController::StopSBTimer()
{
	GetWorldTimerManager().ClearTimer(ScoreBoardToUpdateTimer);
}

void ABlasterPlayerController::ShowScoreBoard()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		SBIsVisible = true;
		BlasterHUD->AddScoreBoardWidget();
		FillScoreBoard();
	}
}

void ABlasterPlayerController::CloseScoreBoard()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->ScoreBoardWidget)
	{
		SBIsVisible = false;
		BlasterHUD->ScoreBoardWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

//BlasterCharacter->isElimmed() == true ? BlasterHUD->PlayerScoreWidget->isDead->SetVisibility(ESlateVisibility::Hidden) : BlasterHUD->PlayerScoreWidget->isDead->SetVisibility(ESlateVisibility::Hidden);
void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		EndGameTime = GameMode->EndGameTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime, EndGameTime, CooldownTime);
	}
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime, float EndGame, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	EndGameTime = EndGame;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncementWidget();//here because in WaitingToStart - BlasterHUD is not exist
	}
	if(BlasterHUD && MatchState == MatchState::Cooldown)
	{
		BlasterHUD->AddAnnouncementWidget();
	}
}