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
#include "Blaster/HUD/InventoryWidget.h"
#include "Blaster/HUD/StoreWidget.h"
#include "Blaster/InventorySystem/InventorySlot.h"
#include "Blaster/StoreSystem/StoreSlot.h"
#include "Blaster/Weapon/ProjectileGrenade.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/HorizontalBox.h"
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
		else 
		{
			BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->AmmoText->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->Slash->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::ShowGrenadeHUD(bool ShowHUD)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadeSlot &&
		BlasterHUD->CharacterOverlay->GrenadeAmountText;

	if (bHUDValid)
	{
		if (ShowHUD == true)
		{
			BlasterHUD->CharacterOverlay->GrenadeSlot->SetVisibility(ESlateVisibility::Visible);
			BlasterHUD->CharacterOverlay->GrenadeAmountText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			BlasterHUD->CharacterOverlay->GrenadeSlot->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->CharacterOverlay->GrenadeAmountText->SetVisibility(ESlateVisibility::Hidden);
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
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);//����������� int � string
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetGrenadeHUD(class AProjectileGrenade* Grenade, int32 Amount)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());

	bool bHUDValid = BlasterCharacter &&
		BlasterCharacter->GetCombatComponent() &&
		BlasterCharacter->GetCombatComponent()->EquippedGrenade &&
		BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadeSlot &&
		BlasterHUD->CharacterOverlay->GrenadeAmountText;
	
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->GrenadeSlot->SetBrushFromTexture(Grenade->GetGrenadeImage());
		FString GrenadeQuantity = FString::Printf(TEXT("%d"), Amount);
		BlasterHUD->CharacterOverlay->GrenadeAmountText->SetText(FText::FromString(GrenadeQuantity));
		if (BlasterHUD->CharacterOverlay->bGrenadeSlotIsEmpty)
		{
			BlasterHUD->CharacterOverlay->bGrenadeSlotIsEmpty = false;
		}
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

	if(HasAuthority())//���� if(HasAuthority()) ������ ����� ��������� LevelStartingTime ��� ����� ��������������� ������� ������� � �����
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

void ABlasterPlayerController::AddAmmoToInventory(EWeaponType WeaponType, int32 Quantity)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		InventoryWidget = InventoryWidget == nullptr ? BlasterHUD->InventoryWidget : InventoryWidget;
		if (InventoryWidget && InventoryWidget->InventorySlots.Num() > 0)
		{
			if (InventoryWidget->bTypeOfAmmoRunOut)
			{
				InventoryWidget->AddAmmoItemToInventory(InventoryWidget->SetAmmoContentForSlot(WeaponType), Quantity, WeaponType);
				InventoryWidget->CurrentSlot->SlotData.bIsSlotToModify = true;
				InventoryWidget->bTypeOfAmmoRunOut = false;
				return;
			}
			if (!InventoryWidget->ExistingAmmoTypesInInventory.Contains(WeaponType))
			{
				InventoryWidget->AddAmmoItemToInventory(InventoryWidget->SetAmmoContentForSlot(WeaponType), Quantity, WeaponType);//����� ����������� � CarrieAmmoMap ������ quantity
				InventoryWidget->CurrentSlot->SlotData.bIsSlotToModify = true;
			}
			else
			{
				for (auto& Item : InventoryWidget->InventorySlots)
				{
					if (Item->SlotData.WeaponType == WeaponType)//���� � ��� �� ����� ��������
					{

						Item->SlotData.bIsSlotToModify = false;
						if (Item->SlotData.SlotAmount + Quantity > Item->MaxSlotQuantity)
						{
							if (!Item->SlotReachedLimit())
							{
								Item->bSlotWasCleared = false;

								int32 AmmoLeft = Item->SlotData.SlotAmount + Quantity - Item->MaxSlotQuantity;//10

								Item->SetSlotData(InventoryWidget->SetAmmoContentForSlot(WeaponType), Item->SlotData.SlotAmount + Quantity - AmmoLeft);//������������� ������� �������� ����� � �������� (80)
								Item->SlotData.bIsSlotToModify = false;

								Item->SlotData.bMximumAmountOfAmmoReached = true;

								InventoryWidget->AddAmmoItemToInventory(InventoryWidget->SetAmmoContentForSlot(WeaponType), AmmoLeft, WeaponType);//����� ���� � 10
								InventoryWidget->CurrentSlot->SlotData.bIsSlotToModify = true;

								InventoryWidget->CurrentSlot->SlotData.SlotAmount = AmmoLeft;
								break;
							}
							if (Item->SlotData.SlotAmount == Item->MaxSlotQuantity && !Item->SlotData.bMximumAmountOfAmmoReached)
							{
								Item->SlotData.bMximumAmountOfAmmoReached = true;
								InventoryWidget->AddAmmoItemToInventory(InventoryWidget->SetAmmoContentForSlot(WeaponType), Quantity, WeaponType);
								InventoryWidget->CurrentSlot->SlotData.bIsSlotToModify = true;
								InventoryWidget->CurrentSlot->SlotData.SlotAmount = Quantity;
								break;
							}
						}
						else
						{
							Item->SetSlotData(InventoryWidget->SetAmmoContentForSlot(WeaponType), Item->SlotData.SlotAmount + Quantity);
							Item->SlotData.bIsSlotToModify = true;
							break;
						}
					}
				}
			}
		}
	}
}

void ABlasterPlayerController::AddGrenadeToInventory(EGrenadeType InGrenadeType, int32 Quantity)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		InventoryWidget = InventoryWidget == nullptr ? BlasterHUD->InventoryWidget : InventoryWidget;
		if (InventoryWidget && InventoryWidget->InventorySlots.Num() > 0)
		{
			if (InventoryWidget->bTypeOfAmmoRunOut)
			{
				if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("MMMMM???"));
				InventoryWidget->AddGrenadeItemToInventory(InventoryWidget->SetGrenadeContentForSlot(InGrenadeType), Quantity, InGrenadeType);
				InventoryWidget->CurrentSlot->SlotData.bIsSlotToModify = true;
				InventoryWidget->bTypeOfAmmoRunOut = false;
				return;
			}
			if (!InventoryWidget->ExistingGrenadeTypesInInventory.Contains(InGrenadeType))
			{
				if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("Here1"));
				InventoryWidget->AddGrenadeItemToInventory(InventoryWidget->SetGrenadeContentForSlot(InGrenadeType), Quantity, InGrenadeType);//����� ����������� � CarrieAmmoMap ������ quantity
				InventoryWidget->CurrentSlot->SlotData.bIsSlotToModify = true;
			}
			else
			{
				for (auto& Item : InventoryWidget->InventorySlots)
				{
					if (Item->SlotData.GrenadeType == InGrenadeType)//���� � ��� �� ����� ��������
					{
						Item->SlotData.bIsSlotToModify = false;
						if (Item->SlotData.SlotAmount + Quantity > Item->MaxSlotQuantity)
						{
							if (!Item->SlotReachedLimit())
							{
								Item->bSlotWasCleared = false;

								int32 AmmoLeft = Item->SlotData.SlotAmount + Quantity - Item->MaxSlotQuantity;//10

								Item->SetSlotData(InventoryWidget->SetGrenadeContentForSlot(InGrenadeType), Item->SlotData.SlotAmount + Quantity - AmmoLeft);//������������� ������� �������� ����� � �������� (80)
								Item->SlotData.bIsSlotToModify = false;

								Item->SlotData.bMximumAmountOfAmmoReached = true;

								InventoryWidget->AddGrenadeItemToInventory(InventoryWidget->SetGrenadeContentForSlot(InGrenadeType), AmmoLeft, InGrenadeType);//����� ���� � 10
								InventoryWidget->CurrentSlot->SlotData.bIsSlotToModify = true;

								InventoryWidget->CurrentSlot->SlotData.SlotAmount = AmmoLeft;
								break;
							}
							if (Item->SlotData.SlotAmount == Item->MaxSlotQuantity && !Item->SlotData.bMximumAmountOfAmmoReached)
							{
								Item->SlotData.bMximumAmountOfAmmoReached = true;
								InventoryWidget->AddGrenadeItemToInventory(InventoryWidget->SetGrenadeContentForSlot(InGrenadeType), Quantity, InGrenadeType);
								InventoryWidget->CurrentSlot->SlotData.bIsSlotToModify = true;
								InventoryWidget->CurrentSlot->SlotData.SlotAmount = Quantity;
								break;
							}
						}
						else
						{
							Item->SetSlotData(InventoryWidget->SetGrenadeContentForSlot(InGrenadeType), Item->SlotData.SlotAmount + Quantity);
							Item->SlotData.bIsSlotToModify = true;
							break;
						}
					}
				}
			}
		}
	}
}

void ABlasterPlayerController::UpdateSlotAmmo()
{
	BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if(BlasterCharacter && BlasterCharacter->GetCombatComponent())
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if (BlasterHUD)
		{
			//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("Check0"));
			InventoryWidget = InventoryWidget == nullptr ? BlasterHUD->InventoryWidget : InventoryWidget;
			if (InventoryWidget)
			{
				//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("Check1"));

				//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("Check2"));
				class UCombatComponent* Combat = BlasterCharacter->GetCombatComponent();
				//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("AmmoToReload = %d"), Combat->GetAmmoToReload()));
				if (Combat->GetAmmoToReload() > 0)
				{
					for (auto& Slot : InventoryWidget->InventorySlots)
					{
						if (Slot->SlotData.WeaponType == Combat->GetEquippedWeapon()->GetWeaponType())
						{
							/*SlotAmmo == MaxSlotQuantity*/
							if (Slot->SlotReachedLimit() && InventoryWidget->bSlotNoLongerModified)
							{
								Slot->SlotData.bIsSlotToModify = true;
							}
							if (Slot->SlotData.bIsSlotToModify && Slot->SlotData.SlotAmount >= 0)
							{
								/*AmmoToReload > SlotAmmo*/
								if (Combat->GetAmmoToReload() > Slot->SlotData.SlotAmount)
								{
									//if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("Here?!"));
									Combat->RemoveCarriedAmmo(Combat->GetEquippedWeapon()->GetWeaponType(), -Slot->SlotData.SlotAmount);
									InventoryWidget->bSlotNoLongerModified = true;
									Slot->SlotData.bIsSlotToModify = false;
									for (auto& Slot2 : InventoryWidget->InventorySlots)
									{
										/*Dicrease Ammo from another slot*/
										if (Slot2->SlotData.WeaponType == Combat->GetEquippedWeapon()->GetWeaponType() && !Slot2->SlotData.bIsSlotToModify && Slot2->SlotReachedLimit() && InventoryWidget->bSlotNoLongerModified)
										{
											Slot2->SlotData.bIsSlotToModify = true;
											InventoryWidget->bSlotNoLongerModified = false;
											int32 UpdatedSlotValue2 = Slot2->SlotData.SlotAmount -= Combat->GetAmmoToReload() - Slot->SlotData.SlotAmount;
											Slot2->SetSlotQuantity(UpdatedSlotValue2);
											Slot2->SlotData.SlotAmount = UpdatedSlotValue2;
											Slot->ClearSlot();
											break;
										}
									}
								}
								else
								{
									/*Slot has enough ammo*/
									if (Slot->SlotData.bIsSlotToModify)
									{
										int32 UpdatedSlotValue = Slot->SlotData.SlotAmount -= Combat->GetAmmoToReload();
										Slot->SetSlotQuantity(UpdatedSlotValue);
										Slot->SlotData.SlotAmount = UpdatedSlotValue;
										if (Slot->SlotData.SlotAmount <= 0)
										{
											Slot->ClearSlot();
											Slot->SlotData.bIsSlotToModify = false;
											for (auto& Slot3 : InventoryWidget->InventorySlots)
											{
												if (Slot3->SlotData.WeaponType == Combat->GetEquippedWeapon()->GetWeaponType() && !Slot3->SlotData.bIsSlotToModify && Slot3->SlotReachedLimit())
												{
													Slot3->SlotData.bIsSlotToModify = true;
													break;
												}
											}
										}
										break;
									}

								}
							}
						}
					}
				}
			}
		}
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
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	//������ �������� �����
	float TimeServerRecievedClientRequest = GetWorld()->GetTimeSeconds();//��������� �����, ����� ������ ������� ������ �� �������(��� ��������� � TimeOfClientRequest)

	//������ ���������� ����� �� ������
	ClientReportServerTime(TimeOfClientRequest, TimeServerRecievedClientRequest);//TimeofClientRequest - ����� ������� � ������ �������� �������
}

//��������� ������ ������� �� ������, ������� ��� ��������� �� ������� ServerRequestServerTime
void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecievedClientRequest)
{
	//round-trip time - ������� ����� ��������, ����� ��� ������� ����� �� �������, � ��������, ����� ��� ��������� ������ �� ������.
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;//������� ��������� ����� ������� (�����) ����� ������� � ������ �������� �������

	//calculate server's current time
	float CurrentServerTime = TimeServerRecievedClientRequest + (RoundTripTime * 0.5);//����� �� ������� ������ ������� ������

	//����� �� ������� ������ ����� �������, �������, ����� ���������� ��������, �������� �� �������� ������� ������� ������� ����� �������.
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();//(��������)������� ����� ������� �������� ������� � �������� �������, ������������ ��� ��������� ������� �� �������.

	//now playerController on the client knows the difference between server's starting time and client's starting time
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ReceivedPlayer()//���������� �� �������, ����� ����� ����� �������������� � ����
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
		if(BlasterHUD->ScoreBoardWidget == nullptr) BlasterHUD->AddScoreBoardWidget();
		if (BlasterHUD->InventoryWidget == nullptr) BlasterHUD->AddInventoryWidget();
		if (BlasterHUD->StoreWidget == nullptr) BlasterHUD->AddStoreWidget();
		
		if (BlasterHUD->AnnouncementWidget && BlasterHUD->InventoryWidget)
		{
			BlasterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->ScoreBoardWidget->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
			BlasterHUD->StoreWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		BlasterHUD->InventoryWidget->RemoveFromParent();
		BlasterHUD->StoreWidget->RemoveFromParent();
		BlasterHUD->AddScoreBoardWidget();

		bool bHUDValid = BlasterHUD->ScoreBoardWidget &&
			BlasterHUD->ScoreBoardWidget->PlayersS &&
			BlasterHUD->AnnouncementWidget &&
			BlasterHUD->AnnouncementWidget->AnnouncementText;

		//����
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

int32 ABlasterPlayerController::GetStoreSlotIndex(int32 SlotIndex)
{
	return StoreSlotIndex = SlotIndex;
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
			BlasterHUD->AnnouncementWidget->InfoText->SetVisibility(ESlateVisibility::Visible);//�������� ����� ���������� EndGameTime
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
				// ������� ������� ��� ���� ������� � ��������� �� � ScoreBoardWidget
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
	if (BlasterHUD && BlasterHUD->ScoreBoardWidget)
	{
		HideActiveWidget();
		ActiveWidget = BlasterHUD->ScoreBoardWidget;
		BlasterHUD->ScoreBoardWidget->SetVisibility(ESlateVisibility::Visible);
		SBIsVisible = true;

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
		BlasterHUD->SetGameOnlyInputMode();
		ActiveWidget = nullptr;
	}
}

void ABlasterPlayerController::OpenInventory()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->InventoryWidget)
	{
		HideActiveWidget();
		ActiveWidget = BlasterHUD->InventoryWidget;
		BlasterHUD->InventoryWidget->SetVisibility(ESlateVisibility::Visible);
		BlasterHUD->SetGameAndUIInputMode();
		bInventotyIsActive = true;
	}
}

void ABlasterPlayerController::CloseInventory()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->InventoryWidget)
	{
		BlasterHUD->InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
		BlasterHUD->SetGameOnlyInputMode();
		bInventotyIsActive = false;
		ActiveWidget = nullptr;
	}
}

void ABlasterPlayerController::OpenStore()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->StoreWidget)
	{
		HideActiveWidget();
		ActiveWidget = BlasterHUD->StoreWidget;
		BlasterHUD->StoreWidget->SetVisibility(ESlateVisibility::Visible);
		BlasterHUD->SetGameAndUIInputMode();
		bStoreIsActive = true;
	}
}

void ABlasterPlayerController::CloseStore()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD && BlasterHUD->StoreWidget)
	{
		BlasterHUD->StoreWidget->SetVisibility(ESlateVisibility::Hidden);
		BlasterHUD->SetGameOnlyInputMode();
		bStoreIsActive = false;
		ActiveWidget = nullptr;
	}
}

void ABlasterPlayerController::BuyGrenade(EGrenadeType GrenadeType, int32 GrenadesAmount)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	//SetGrenadeHUD(GrenadesAmount);

	AddGrenadeToInventory(GrenadeType, GrenadesAmount);
	
}

void ABlasterPlayerController::HideActiveWidget()
{
	if(ActiveWidget)
	{
		ActiveWidget->SetVisibility(ESlateVisibility::Hidden);
		if (ActiveWidget == BlasterHUD->ScoreBoardWidget) SBIsVisible = false;
		if (ActiveWidget == BlasterHUD->InventoryWidget) bInventotyIsActive = false;
		if (ActiveWidget == BlasterHUD->StoreWidget) bStoreIsActive = false;
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