#include "BlasterCharacter.h"
#include "Blaster/BlaserComponents/CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter() //Constructor
{
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 250.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCameraa = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCameraa->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCameraa->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject <UWidgetComponent> (TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combatt = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combatt->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 120.f;
	MinNetUpdateFrequency = 120.f;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	UELogInfo(FMath::RoundToInt(5.1));

	UpdateHUDHealth();

	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::RecieveDamage);//????????? ??????? ??????? ????? ??????? ??? ????????????? ??????? OnTakeAnyDamage
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Both characters on the server have local role "Authority"
	//SimulatedProxy - is a server client on AutonomousProxy(client 1)(left window)
	if(GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled()) //using offset only for players who are controlling the character
	{
		AimOffset(DeltaTime);
	}
	else if(GetLocalRole() == ENetRole::ROLE_SimulatedProxy || HasAuthority())
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}

	HideCharacterWhenCameraClose();
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, isSprinting);
	DOREPLIFETIME(ABlasterCharacter, BaseSpeed);
	DOREPLIFETIME(ABlasterCharacter, SprintSpeed);
	DOREPLIFETIME(ABlasterCharacter, CurrentHealth);
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ABlasterCharacter::SprintButtonPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ABlasterCharacter::SprintButtonReleased);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combatt)
	{
		Combatt->Character = this;
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();

	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combatt == nullptr || Combatt->EquippedWeapon == nullptr) return;

	if (bPlayMontage == true) {
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();//?????????? ? ???????? animinstance
		if (AnimInstance && FireWeaponMontage)
		{
			AnimInstance->Montage_Play(FireWeaponMontage);//??????????? ??????????
			FName SectionName;
			SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
			AnimInstance->Montage_JumpToSection(SectionName);
		}
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combatt == nullptr || Combatt->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();//?????????? ? ???????? animinstance
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}


void ABlasterCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);//"." - from that rotation is getting the Yaw
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
		if(Value < 0)
		{
			isMovingRight = true;
		}
		else
		{
			isMovingRight = false;
		}
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		const float ClampedValue = FMath::Clamp(Value, -300.f, 300.f);
		AddMovementInput(Direction, ClampedValue);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if(Combatt)
	{
		if (HasAuthority()) { //?? ???????
			Combatt->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combatt)
	{
		Combatt->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else 
	{
		Crouch();
	}
}

void ABlasterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if(Combatt)
	{
		Combatt->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (Combatt)
	{
		Combatt->SetAiming(false);
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (Combatt)
	{
		Combatt->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (Combatt)
	{
		Combatt->FireButtonPressed(false);
	}
}

void ABlasterCharacter::SprintButtonPressed()
{
	SetSprint(true);
}

void ABlasterCharacter::SprintButtonReleased()
{
	SetSprint(false);
}

void ABlasterCharacter::SetSprint(bool bIsSprinting)
{
	isSprinting = bIsSprinting;
	if (isSprinting && !isMovingRight)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	}
	ServerSetSprint(bIsSprinting);
}

void ABlasterCharacter::ServerSetSprint_Implementation(bool bIsSprinting)
{
	isSprinting = bIsSprinting;
	if (isSprinting && !isMovingRight)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combatt && Combatt->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if(Speed == 0.f && !bIsInAir)//can AimOffset while Standing still && not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if(TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if(Speed > 0.f || bIsInAir)//dont use AimOffset while moving
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning; 
	}
	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (Combatt == nullptr || Combatt->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	bRotateRootBone = false;

	if(Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if(FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if(ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();
}


void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 45.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -45.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if(TurningInPlace != ETurningInPlace::ETIP_NotTurning)//if we are turning
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 6.f);
		AO_Yaw = InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw) < 10.f)//???????? ????????
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::HideCharacterWhenCameraClose()
{
	if (!IsLocallyControlled()) return; //should only be executed by the locally controlled player

	if((FollowCameraa->GetComponentLocation() - GetActorLocation()).Size() < CameraTrashold)
	{
		bPlayMontage = false;
		GetMesh()->SetVisibility(false);
		if(Combatt && Combatt->EquippedWeapon && Combatt->EquippedWeapon->GetWeaponMesh())
		{
			Combatt->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;//SetVisibility(false)
		}
	}
	else
	{
		bPlayMontage = true;
		GetMesh()->SetVisibility(true);
		if (Combatt && Combatt->EquippedWeapon && Combatt->EquippedWeapon->GetWeaponMesh())
		{
			Combatt->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;//SetVisibility(false)
		}
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool ABlasterCharacter::isWeaponEquipped()
{
	return (Combatt && Combatt->EquippedWeapon);
}

bool ABlasterCharacter::isAiming()//getter(use getter to set in another class(animInstance)
{
	return (Combatt && Combatt->bAiming); //return true if Combat is valid && bAiming is true
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()//return currently equipped weapon
{
	if (Combatt == nullptr) return nullptr;
	return Combatt->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if(Combatt == nullptr) return FVector();
	return Combatt->HitTarget;
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;//allows to not cast multiple times(make sure that BlasterPlayerController is set) 
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(CurrentHealth, MaxHealth);
	}
}

void ABlasterCharacter::UELogInfo(float Value)
{
	float Speed = GetCharacterMovement()->Velocity.Length();
	UE_LOG(LogTemp, Error, TEXT("value: %f"), Value);
}

void ABlasterCharacter::PrintNetModeAndRole()
{
	FString NetModeStr;
	switch (GetNetMode())
	{
	case NM_Standalone:
		NetModeStr = TEXT("Standalone");
		break;
	case NM_ListenServer:
		NetModeStr = TEXT("ListenServer");
		break;
	case NM_Client:
		NetModeStr = TEXT("Client");
		break;
	case NM_DedicatedServer:
		NetModeStr = TEXT("DedicatedServer");
		break;
	case NM_MAX:
		NetModeStr = TEXT("Unknown");
		break;
	}
	FString RoleStr;
	switch (GetLocalRole())
	{
	case ROLE_None:
		RoleStr = TEXT("None");
		break;
	case ROLE_SimulatedProxy:
		RoleStr = TEXT("SimulatedProxy");
		break;
	case ROLE_AutonomousProxy:
		RoleStr = TEXT("AutonomousProxy");
		break;
	case ROLE_Authority:
		RoleStr = TEXT("Authority");
		break;
	case ROLE_MAX:
		RoleStr = TEXT("Unknown");
		break;
	}
	if(!HasAuthority())
	GEngine->AddOnScreenDebugMessage(-1, 120.f, FColor::Red, FString::Printf(TEXT("NetMode: %s, Role: %s \n"), *NetModeStr, *RoleStr));
}

//1. ??????? ActionMapping
//2. ??????? void *Pressed/Released
//3. ??????? PlayerInputComponent
//4. ??????? foo ? bool ??????????, ? ?????? ???????? ?? ?????? ??????????? ???????? fe: "Fire" - Combat class
//5. ???????? ??????? ?? ?????? "Combat" ? foo Pr/Re ? ????????? ???????? true/false