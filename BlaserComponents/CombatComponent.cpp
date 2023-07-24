#include "CombatComponent.h"
#include "Components/SphereComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "DrawDebugHelpers.h"
#include "ParameterCollection.h"
#include "Camera/CameraComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "TimerManager.h"
#include "Blaster/Character/BlasterAnimInstance.h"
#include "Blaster/HUD/InventoryWidget.h"
#include "Blaster/InventorySystem/InventorySlot.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if(Character->GetFollowCamera())
	{
		DefaultFov = Character->GetFollowCamera()->FieldOfView;
		CurrentFOV = DefaultFov;
	}
	if(Character->HasAuthority())
	{
		InitializeCarriedAmmo();
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Character && Character->IsLocallyControlled())//only occur for the local player(such as playing certain sounds or spawning visual effects)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint; //точка попадания
		SetHUDCrosshairs(DeltaTime, HitResult);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);//when changes, it will be reflected on all clients
	DOREPLIFETIME(UCombatComponent, PrimaryWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, AmmoToReload);
	DOREPLIFETIME_CONDITION(UCombatComponent, bAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);//replicates only to a client that actively controlled
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::EquipWeapon(class AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	if (PrimaryWeapon && SecondaryWeapon)
	{
		EquippedWeapon->Dropped();

		if(EquippedWeapon == PrimaryWeapon)//в руках PrimaryWeapon
		{
			EquipPrimaryWeapon(WeaponToEquip);
		}
		else if(EquippedWeapon == SecondaryWeapon)//в руках SecondaryWeapon
		{
			EquipSecondaryWeapon(WeaponToEquip);
		}
	}

	if (EquippedWeapon == nullptr)
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
	else if(EquippedWeapon && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}

	AnimInstance = AnimInstance == nullptr ? Cast<UBlasterAnimInstance>(Character->GetMesh()->GetAnimInstance()) : AnimInstance;

	if (AnimInstance && EquippedWeapon)
	{
		AnimInstance->SetRelativeHandTransform();
		AnimInstance->ChangeOptic();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WTF"));
	}
	EquippedWeapon->SetHUDAmmo();

	UpdateCarriedAmmo();

	if (EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->EquipSound,
			Character->GetActorLocation());
	}

	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon()//client
{
	AnimInstance = AnimInstance == nullptr ? Cast<UBlasterAnimInstance>(Character->GetMesh()->GetAnimInstance()) : AnimInstance;
	
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		UpdateCarriedAmmo();

		AttachWeaponToSocket(EquippedWeapon);

		if (AnimInstance && EquippedWeapon)
		{
			AnimInstance->SetRelativeHandTransform();
			AnimInstance->ChangeOptic();
		}
	}

	if (EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->EquipSound,
			Character->GetActorLocation()
		);
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	PrimaryWeapon = WeaponToEquip;
	EquippedWeapon = PrimaryWeapon;
	EquippedWeapon->SetOwner(Character);

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachWeaponToSocket(EquippedWeapon);
	
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	if (SecondaryWeapon)
	{
		SecondaryWeapon = WeaponToEquip;

		EquippedWeapon = SecondaryWeapon;
		EquippedWeapon->SetOwner(Character);
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachWeaponToSocket(EquippedWeapon);
		
	}
	else
	{
		SecondaryWeapon = WeaponToEquip;
		SecondaryWeapon->bHideWeapon = true;
		SecondaryWeapon->GetWeaponMesh()->SetVisibility(false);
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	}
}

void UCombatComponent::AttachWeaponToSocket(AWeapon* WeaponToEquip)
{
	const USkeletalMeshSocket* HandSocket = GetWeaponSocket(Character->GetMesh());
	if (HandSocket)
	{
		HandSocket->AttachActor(WeaponToEquip, Character->GetMesh());
	}
}

void UCombatComponent::ChoosePrimaryWeapon()
{
	if (EquippedWeapon && PrimaryWeapon && SecondaryWeapon && EquippedWeapon != PrimaryWeapon && Character)
	{
		EquippedWeapon->Deactivate();
		EquippedWeapon = PrimaryWeapon;
		EquippedWeapon->SetOwner(Character);
		EquippedWeapon->Activate(Character);

		UpdateCarriedAmmo();
		EquippedWeapon->SetHUDAmmo();

		AttachWeaponToSocket(EquippedWeapon);
	}
}

void UCombatComponent::ChooseSecondaryWeapon()
{
	if (EquippedWeapon && PrimaryWeapon && SecondaryWeapon && EquippedWeapon != SecondaryWeapon && Character)
	{
		EquippedWeapon->Deactivate();
		EquippedWeapon = SecondaryWeapon;
		EquippedWeapon->SetOwner(Character);
		EquippedWeapon->Activate(Character);

		UpdateCarriedAmmo();
		EquippedWeapon->SetHUDAmmo();

		AttachWeaponToSocket(EquippedWeapon);
	}
}

void UCombatComponent::UpdateCarriedAmmo_Implementation()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->ShowAmmoHUD(true);
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);//carriedAmmo это патроны equippedweapon
	}
}

void UCombatComponent::ClientUpdateSlotAmmo_Implementation()
{
	if (Character == nullptr || Character->Controller == nullptr) return;//so we can access controller via character

	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController; //if Controller !null -> equel to itself
	if (PlayerController)//check if is valid
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(PlayerController->GetHUD()) : HUD;//if HUD !null -> equel to itself(we are sure that our HUD is set)(для подстраховки/для избежания багов)
		if (HUD)
		{
			InventoryWidget = HUD->InventoryWidget;
			if (InventoryWidget)
			{
				if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White, FString("Im here, now not return"));
				if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Black, FString::Printf(TEXT("Ammo to reload = %d"), AmmoToReload));

				if (AmmoToReload > 0)
				{
					if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White, FString("Im here)))"));
					for (auto& Slot : InventoryWidget->InventorySlots)
					{
						if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString("For the begining"));
						if (Slot->SlotType == EquippedWeapon->GetWeaponType())
						{
							if (Slot->bIsSlotToModify && Slot->SlotAmmo >= 0)
							{
								//GetCarriedAmmo();
								if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString("Here"));
								//int32 AmmoToReload = AmountToReload();
								if (AmmoToReload > Slot->SlotAmmo)
								{
									if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("AmountToReload > SlotAmmo | Name = %s"), *Slot->GetName()));

									SetCarriedAmmo(EquippedWeapon->GetWeaponType(), -Slot->SlotAmmo);
									InventoryWidget->bSlotNoLongerModified = true;
									Slot->bIsSlotToModify = false;
									for (auto& Slot2 : InventoryWidget->InventorySlots)
									{
										if (Slot2->SlotType == EquippedWeapon->GetWeaponType() && !Slot2->bIsSlotToModify && Slot2->SlotReachedLimit() && InventoryWidget->bSlotNoLongerModified)
										{
											if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("AmountToReload > SlotAmmo | New SlotToModify Name = %s"), *Slot->GetName()));
											if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("AmountToReload() | How much ammo to dicrease from new slot that is 80 = %d"), AmmoToReload));
											if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("should be 80 | Slot->SlotAmmo = %d"), Slot->SlotAmmo));
											Slot2->bIsSlotToModify = true;
											InventoryWidget->bSlotNoLongerModified = false;
											int32 UpdatedSlotValue2 = Slot2->SlotAmmo -= AmmoToReload - Slot->SlotAmmo;
											if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("UpdatedSlotAmmo = %d"), Slot2->SlotAmmo));
											Slot2->SetSlotQuantity(UpdatedSlotValue2);
											Slot2->SlotAmmo = UpdatedSlotValue2;
											Slot->ClearSlot();
											break;
										}
									}
								}
								else
								{
									if (!InventoryWidget->bSlotNoLongerModified && Slot->bIsSlotToModify)
									{
										//GetCarriedAmmo();
										if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Reload just this slot | Slot->SlotAmmo = %d"), Slot->SlotAmmo));
										if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Should be less than prev | AmountToReload() = %d"), AmmoToReload));
										int32 UpdatedSlotValue = Slot->SlotAmmo -= AmmoToReload;
										Slot->SetSlotQuantity(UpdatedSlotValue);
										Slot->SlotAmmo = UpdatedSlotValue;
										if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("Slot ammo after Reloading = %d"), UpdatedSlotValue));
										if (Slot->SlotAmmo <= 0)
										{
											if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("If after reload slotAmmo == 0 | SlotName = %s"), *Slot->GetName()));
											if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString("SlotAmmo is empty"));
											Slot->ClearSlot();
											//InventoryWidget->bSlotNoLongerModified = true;
											Slot->bIsSlotToModify = false;
											for (auto& Slot3 : InventoryWidget->InventorySlots)
											{
												if (Slot3->SlotType == EquippedWeapon->GetWeaponType() && !Slot3->bIsSlotToModify && Slot3->SlotReachedLimit())
												{
													if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("v2 SlotAmmo = 0 | NewSlotName = %s"), *Slot3->GetName()));
													Slot3->bIsSlotToModify = true;
													break;
												}
											}
										}
										if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString("Just break"));

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

int32 UCombatComponent::SetCarriedAmmo(EWeaponType WeaponType, int32 RemoveAmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("CarriedAmmo = %d"), CarriedAmmoMap[WeaponType]));
		UpdateCarriedAmmo();
		CarriedAmmo -= RemoveAmmoAmount;
		if (InventoryWidget)
		{
			if (CarriedAmmo == 0)
			{
				InventoryWidget->bTypeOfAmmoRunOut = true;
			}
		}
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("CarriedAmmo = %d"), CarriedAmmoMap[WeaponType]));
	}
	return 0;
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] += AmmoAmount;
		UpdateCarriedAmmo();
		ClientAddItemToInventory(WeaponType, AmmoAmount);
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("CarriedAmmo = %d"), CarriedAmmoMap[WeaponType]));
	}
	if(EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

void UCombatComponent::ClientAddItemToInventory_Implementation(EWeaponType WeaponType, int32 Quantity)
{
	if (Character == nullptr || Character->Controller == nullptr) return;//so we can access controller via character

	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController; //if Controller !null -> equel to itself
	if (PlayerController)//check if is valid
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(PlayerController->GetHUD()) : HUD;//if HUD !null -> equel to itself(we are sure that our HUD is set)(для подстраховки/для избежания багов)
		if (HUD)
		{
			InventoryWidget = HUD->InventoryWidget;
			if (InventoryWidget && InventoryWidget->InventorySlots.Num() > 0)
			{
				for(auto& Slot : InventoryWidget->InventorySlots)
				{
					if (Slot->bIsSlotToModify) if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("IsSlotToModify slot name = %s"), *Slot->GetName()));
					if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString("Im here, now return"));
				}
				if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::White, FString("Call"));
				if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Blue, FString::Printf(TEXT("bTypeOfAmmoRunOut = %d"), InventoryWidget->bTypeOfAmmoRunOut));
				if(InventoryWidget->bTypeOfAmmoRunOut)
				{
					if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Green, FString("Toretto"));
					InventoryWidget->AddItemToInventory(InventoryWidget->SetContentForSlot(WeaponType), Quantity, WeaponType);
					InventoryWidget->CurrentSlot->bIsSlotToModify = true;
					InventoryWidget->bTypeOfAmmoRunOut = false;
					return;
				}
				if (!InventoryWidget->ExistingItemTypesInInventory.Contains(WeaponType))
				{
					if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Red, FString::Printf(TEXT("1SlotNumber: %d"), InventoryWidget->SlotNumber));
					InventoryWidget->AddItemToInventory(InventoryWidget->SetContentForSlot(WeaponType), Quantity, WeaponType);//можно исползовать и CarrieAmmoMap вместо quantity
					InventoryWidget->CurrentSlot->bIsSlotToModify = true;
					if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CurrentSlot->SlotAmmo0 = %d"), InventoryWidget->CurrentSlot->SlotAmmo));
					if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CurrentSlot->SlotAmmo1 = %d"), InventoryWidget->CurrentSlot->SlotAmmo));

					if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Blue, FString("New Type of Item added"));
				}
				else
				{
					if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString("Else1"));
					for(auto& Item : InventoryWidget->InventorySlots)
					{
						if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Item->SlotAmmo0 = %d"), Item->SlotAmmo));
						/*if (Item->bSlotWasCleared)
						{
							if (Item->SlotAmmo + Quantity > Item->MaxSlotQuantity)
							{
								if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("2. %s"), *Item->GetName()));
								if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString("AddItemToInventory1"));
								Item->SlotAmmo += Quantity;
								InventoryWidget->AddItemToInventory(InventoryWidget->SetContentForSlot(WeaponType), Item->SlotAmmo, WeaponType);
							}
							else
							{
								if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString("SetSlotData0"));
								Item->SetSlotData(InventoryWidget->SetContentForSlot(WeaponType), Item->SlotAmmo + Quantity);
								Item->SlotAmmo += Quantity;
							}
						}*/
						if(Item->SlotType == WeaponType)//слот с тем же типом патронов
						{
							
							Item->bIsSlotToModify = false;
							//if(InventoryWidget->bNewItemAdded) Item->SlotAmmo = CarriedAmmoMap[WeaponType];//задаём начальное значение слота с новым типом амуниции
							//здесь можно присвоить или создать и присвоить объект слотвиджета который соответствует виду патронов что уже есть в инв
							if (Item->SlotAmmo + Quantity > Item->MaxSlotQuantity)
							{
								if (!Item->SlotReachedLimit())
								{
									if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("Here?"));

									if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Item->SlotAmmo1 = %d"), Item->SlotAmmo));
									if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Red, FString::Printf(TEXT("2SlotNumber: %d"), InventoryWidget->SlotNumber));
									Item->bSlotWasCleared = false;

									int32 AmmoLeft = Item->SlotAmmo + Quantity - Item->MaxSlotQuantity;//10
									if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("AmmoLeft1 = %d"), AmmoLeft));

									Item->SetSlotData(InventoryWidget->SetContentForSlot(WeaponType), Item->SlotAmmo + Quantity - AmmoLeft);//устанавливает текущее значение слота в максимум (80)
									Item->bIsSlotToModify = false;
									if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString("SetSlotData1"));
									if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Red, FString::Printf(TEXT("3SlotNumber: %d"), InventoryWidget->SlotNumber));

									Item->bMximumAmountOfAmmoReached = true;

									InventoryWidget->AddItemToInventory(InventoryWidget->SetContentForSlot(WeaponType), AmmoLeft, WeaponType);//новый слот с 10
									InventoryWidget->CurrentSlot->bIsSlotToModify = true;
									if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString("AddItemToInventory2"));

									InventoryWidget->CurrentSlot->SlotAmmo = AmmoLeft;
									if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("AmmoLeft2 = %d"), AmmoLeft));
									break;
								}
								if(Item->SlotAmmo == Item->MaxSlotQuantity && !Item->bMximumAmountOfAmmoReached)
								{
									Item->bMximumAmountOfAmmoReached = true;
									if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("80CurrentSlot->SlotAmmo1 = %d"), InventoryWidget->CurrentSlot->SlotAmmo));
									InventoryWidget->AddItemToInventory(InventoryWidget->SetContentForSlot(WeaponType), Quantity, WeaponType);
									InventoryWidget->CurrentSlot->bIsSlotToModify = true;
									InventoryWidget->CurrentSlot->SlotAmmo = Quantity;
									if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("80CurrentSlot->SlotAmmo2 = %d"), InventoryWidget->CurrentSlot->SlotAmmo));
									break;
								}
							}
							else
							{
								if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString("Else2"));
								if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Red, FString::Printf(TEXT("4SlotNumber: %d"), InventoryWidget->SlotNumber));
								if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString("SetSlotData2"));
								Item->SetSlotData(InventoryWidget->SetContentForSlot(WeaponType), Item->SlotAmmo + Quantity);
								Item->bIsSlotToModify = true;
								if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Item->SlotAmmo2 = %d"), Item->SlotAmmo));
								if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Red, FString::Printf(TEXT("5SlotNumber: %d"), InventoryWidget->SlotNumber));
								break;
							}
						}
					}
				}
			}
		}
	}
}

const USkeletalMeshSocket* UCombatComponent::GetWeaponSocket(USkeletalMeshComponent* SkeletalMesh)
{
	if (EquippedWeapon && Character && SkeletalMesh)
	{
		if (EquippedWeapon->GetWeaponSocketType() == EWeaponSocketType::EWST_SilverWeapon)
		{
			const USkeletalMeshSocket* SilverWeaponSocket = SkeletalMesh->GetSocketByName("RightHandSocket");
			return SilverWeaponSocket;
		}
		if (EquippedWeapon->GetWeaponSocketType() == EWeaponSocketType::EWST_SciFiWeapon)
		{
			const USkeletalMeshSocket* SciFiWeaponSocket = SkeletalMesh->GetSocketByName("RightHandSciFiSocket");
			return SciFiWeaponSocket;
		}
		if (EquippedWeapon->GetWeaponSocketType() == EWeaponSocketType::EWST_Custom)
		{
			const USkeletalMeshSocket* MeshSocket;
			switch (EquippedWeapon->GetWeaponType())
			{
			case EWeaponType::EWT_SF_Pistol:
				return MeshSocket = SkeletalMesh->GetSocketByName("RightHandSciFiSocket");//pistol socket
			case EWeaponType::EWT_SF_ShotGun:
				return MeshSocket = SkeletalMesh->GetSocketByName("SciFi_ShotGunSocket");
			}
		}
	}
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("You forgot to select EWST"));
	return nullptr;
}

void UCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && !EquippedWeapon->MagIsFull() && CombatState != ECombatState::ECS_Reloading)
	{
		//GetCarriedAmmo();
		HandleReload();
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	//потом тут
	CombatState = ECombatState::ECS_Reloading;//because CombatState is replicated variable, it will be replicated to clients and server.
	HandleReload();
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;

	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString("There"));
		UpdateAmmoValues();
		ClientUpdateSlotAmmo();

		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("AmmoToReload0 = %d"), AmmoToReload));
	}
	if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("AmmoToReload1 = %d"), AmmoToReload));

	if (Character->IsLocallyControlled())
	{
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("AmmoToReload2 = %d"), AmmoToReload));
	}

	//here refresh inventory
	
	if(bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	AmmoToReload = AmountToReload();
	SetAmmoToReloadForClient(AmmoToReload);
	if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString("AmmoToReload is set"));
	if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString::Printf(TEXT("AmmoToReload = %d"), AmmoToReload));
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= AmmoToReload;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->ShowAmmoHUD(true);
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-AmmoToReload);
}

bool UCombatComponent::isOutOfAmmo()
{
	return CarriedAmmo == 0 && EquippedWeapon->GetAmmo() == 0;
}

void UCombatComponent::PlayOutOfAmmoSound()
{
	if(EquippedWeapon->OutOfAmmoSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->OutOfAmmoSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadingMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		if(GEngine)GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red, FString("Im in )"));
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("AmountCarried: %d"), AmountCarried));
		int32 least = FMath::Min(RoomInMag, AmountCarried);
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("least: %d"), least));
		return FMath::Clamp(RoomInMag, 0, least);
	}
	return 0;
}

void UCombatComponent::SetAmmoToReloadForClient_Implementation(int32 ToReload)
{
	AmmoToReload = ToReload;
}

void UCombatComponent::OnRep_CombatState()//logic for client
{
	switch(CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return; // if dont have EquippedWeapon - return out of this function
	if(bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());//how fast zoom when aiming
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFov, DeltaTime, UnZoomInterpSpeed);//EquippedWeapon->GetZoomedFOV()-заменить на CurrentFOV
	}

	if(Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (GetEquippedWeapon()->GetWeaponSocketType() != EWeaponSocketType::EWST_SilverWeapon) return;

	bAiming = bIsAiming;//для тебя в аиме
	if (!Character->HasAuthority())
	{
		ServerSetAiming(bIsAiming);//передаёт инфу серверу и остальным клиентам
	}

	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)//реплицирует информацию всем клиентам
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime, FHitResult& TraceHitResult)
{
	if (Character == nullptr || Character->Controller == nullptr) return;//so we can access controller via character

	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController; //if Controller !null -> equel to itself
	if (PlayerController)//check if is valid
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(PlayerController->GetHUD()) : HUD;//if HUD !null -> equel to itself(we are sure that our HUD is set)(для подстраховки/для избежания багов)
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			//Calculate crosshair spread

			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			//[0, 600] -> [0, 1]
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 5.f);//прицел расширяется в воздухе
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);//возвращается в исходное сост.
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0, DeltaTime, 20.f);

			HUDPackage.CrosshairSpread =
				0.65f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;

			if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
			{
				HUDPackage.CrosshairsColor = FLinearColor::Red;
				HUDPackage.CrosshairSpread -= 0.15f;
			}
			else
			{
				HUDPackage.CrosshairsColor = FLinearColor::White;
			}

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult) //tracing projectile
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); //Set crosshair position to center
	FVector CrosshairWorldPosition;//пустой вектор
	FVector CrosshairWorldDirection;//пустой вектор
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,//заполняем вектор 3D координатами
		CrosshairWorldDirection//заполняем вектор 3D координатами
	);

	if(bScreenToWorld)//if DeprojectScreenToWorld was success
	{
		FVector Start = CrosshairWorldPosition;//Расположение прицела

		if(Character)//фикс бага, когда прицел сталкивается с объектами за спиной у character
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Length();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f); //Начало трейсирвки спереди от character
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;//длина луча

		GetWorld()->LineTraceSingleByChannel(//perform lineTrace
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed && EquippedWeapon && EquippedWeapon->GetWeaponType() != EWeaponType::EWT_GrapplingHook)
	{
		Fire();
		if(Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle && bAiming)
		{
			Character->AimButtonReleased();
		}
	}
}

void UCombatComponent::Fire()
{
	if (EquippedWeapon == nullptr) return;

	if (isOutOfAmmo())
	{
		PlayOutOfAmmoSound();
	}
	
	if (CanFire())
	{
		ServerFire(HitTarget);
		if (EquippedWeapon)
		{
			bCanFire = false;
			CrosshairShootingFactor = 0.75f;
		}
		StartFireTimer();
	}
}
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished() 
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	bCanFire = true;
	
	if (bFireButtonPressed && EquippedWeapon->bAutomatic) //if weapon is automatic
	{
		Fire();
	}
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
	
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied && Character->GetIsSprinting() == false;//если есть патроны - true
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	PlayerController = PlayerController == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::GetCarriedAmmo_Implementation()
{
	if (EquippedWeapon == nullptr) return;
	if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("GetCarriedAmmo!"));
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);//Emplace() ~= add, but avoid any temporaries.  EWT_AssaultRifle value = 30
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SF_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SF_ShotGun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrapplingHook, StartingGrenadeLauncherAmmo);
}
