#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetHUDHealth(float CurrentHealth, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void OnPossess(APawn* InPawn) override;
	void SetHUDWeaponAmmo(int32 AmmoAmount);
	void SetHUDCarriedAmmo(int32 AmmoAmount);
	UFUNCTION(Client, Reliable)
	void ShowAmmoHUD(bool ShowHUD);

	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);

	virtual void Tick(float DeltaTime) override;

	virtual float GetServerTime();//Synced with server world clock

	virtual void ReceivedPlayer() override;//Synced with server clock as soon as possible

	void OnMatchStateSet(FName State);//определяет какой будет HUD при разных MatchState

	void FillScoreBoard();

	void RequestPlayerStates();

	void ShowScoreBoard();

	void CloseScoreBoard();
	TArray<class ABlasterPlayerState*> BlasterPlayerStates;

	void HideSniperScope();

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();

	/*
	 * Sync time between client and server
	 */

	//Request the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeofClientRequest);//send client's current time to the server

	//REports the current server time to a client in response to 

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerRecievedClientRequest);

	float ClientServerDelta = 0.f;//difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;//частота синхронизации

	float TimeSyncRunningTime = 0.f;//how much time has passed since the last time we sync it up

	void CheckTimeSyn(float DeltaTime);

	void HandleWaitingToStart();

	void HandleMatchHasStarted();

	void HandleCooldownn();

	void HandleCooldown();

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float EndGame, float Cooldown);//informing client of the match state when it joins

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
	class ABlasterCharacter* BlasterCharacter;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	UPROPERTY()
	class ABlasterGameState* BlasterGameState;

	UPROPERTY()
	class UInventoryComponent* InventoryComponent;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float EndGameTime = 0.f;
	float CooldownTime = 0.f;

	uint32 CountdownInt = 0;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	// Score Board
	UPROPERTY()
	class UScoreBoardWidget* ScoreBoardWidget;

	UPROPERTY()
	class UInventoryWidget* InventoryWidget;

	bool CooldownIsHandled = false;

	FTimerHandle ScoreBoardToUpdateTimer;

	void StartSBTimer();
	void StopSBTimer();

	float UpdateSBDelay = 0.2f;

	bool bInitializeCharacterOverlay = false;
	bool SBIsVisible = false;

	float HUDCurrentHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;

	UPROPERTY(EditAnywhere)
	float SniperZoom = 90.f;
public:
	FORCEINLINE bool GetCooldownIsHandled() const { return CooldownIsHandled; }
};

