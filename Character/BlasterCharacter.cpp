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
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "TimerManager.h"
#include "Blaster/BlaserComponents/AbilityComponent.h"
#include "Blaster/World/DamageArea.h"
#include "Sound/SoundCue.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/HUD/OverheadWidget.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/Weapon/Scopes/Scope.h"
#include "Blaster/GAS/GASAttributeSet.h"
#include "Blaster/GAS/GASComponent.h"
#include "Blaster/GAS/GASGameplayAbility.h"
#include <GameplayEffectTypes.h>
#include "Blaster/BlaserComponents/GrappleComponent.h"
#include "Blaster/BlaserComponents/InventoryComponent.h"
#include "Blaster/Grappling/GrappleTarget.h"
#include "Blaster/InventorySystem/Items/Item.h"
#include "Blaster/Weapon/Grenade/SingularityGrenade.h"
#include "Engine/SkeletalMeshSocket.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	/*
	 * TP
	 */
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	GetMesh()->bOwnerNoSee = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 250.f;
	CameraBoom->bUsePawnControlRotation = true;

	TPCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TPCamera"));
	TPCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TPCamera->bUsePawnControlRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	/*
	 * FP
	 */
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = true;
	bUseControllerRotationYaw = true;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("FPS Scene"));
	Scene->SetupAttachment(PlayerCamera);

	FPSMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSMesh"));
	FPSMesh->SetupAttachment(Scene);
	FPSMesh->SetCollisionObjectType(ECC_SkeletalMesh);
	FPSMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	FPSMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	FPSMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	FPSMesh->bOnlyOwnerSee = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combatt = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combatt->SetIsReplicated(true);

	Abilitiess = CreateDefaultSubobject<UAbilityComponent>(TEXT("AbilityComponent"));
	Abilitiess->SetIsReplicated(true);

	AbilitySystemComponent = CreateDefaultSubobject<UGASComponent>(TEXT("AbilitySystemComp"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	Attributes = CreateDefaultSubobject<UGASAttributeSet>(TEXT("Attributes"));

	ShiftAbilitySystemComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("shiftniagarasyst"));
	ShiftAbilitySystemComponent = Abilitiess->GetNiagaraComp();

	GrappleComponent = CreateDefaultSubobject<UGrappleComponent>(TEXT("GrappleComponent"));
	GrappleComponent->SetIsReplicated(true);

	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	Inventory->SetIsReplicated(true);

	ChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("ChildActor"));
	ChildActor->SetupAttachment(GetMesh());

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 120.f;
	MinNetUpdateFrequency = 120.f;
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>("Attached Grenade");
	AttachedGrenade->SetupAttachment(FPSMesh, FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UAbilitySystemComponent* ABlasterCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABlasterCharacter::InitializeAttributes()
{
	if (AbilitySystemComponent && DefaultAttributeEffect)//applying effects to a character
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1, EffectContext);

		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle GEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void ABlasterCharacter::GiveAbilities()
{
	if (HasAuthority() && AbilitySystemComponent)
	{
		for (TSubclassOf<UGASGameplayAbility>& StartupAbility : DefaultGASAbilities)
		{
			AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(StartupAbility, 1, static_cast<int32>(StartupAbility.GetDefaultObject()->AbilityInputID), this));

		}
	}
}

void ABlasterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	//server GAS init
	AbilitySystemComponent->InitAbilityActorInfo(this, this);//tells abilitysystem who the owner is
	AGrappleTarget* ChildrenActor = Cast<AGrappleTarget>(ChildActor->GetChildActor());
	UWidgetComponent* TargetWidget = ChildrenActor->WidgetComponent;
	if (IsLocallyControlled()) TargetWidget->SetHiddenInGame(true);

	InitializeAttributes();
	GiveAbilities();
}

void ABlasterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);//tells abilitysystem who the owner is
	InitializeAttributes();
	if (AbilitySystemComponent && InputComponent)
	{
		const FGameplayAbilityInputBinds Binds("Confirm", "Cancel", "EGASAbilityInputID", static_cast<int32>(EGASAbilityInputID::Confirm), static_cast<int32>(EGASAbilityInputID::Cancel));
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, Binds);
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	/*if (OverheadWidget)
	{
		UOverheadWidget* OverheadWidgetInstance = Cast<UOverheadWidget>(OverheadWidget->GetUserWidgetObject());
		if (OverheadWidgetInstance)
		{
			OverheadWidgetInstance->ShowPlayerNetRole(this);
		}
	}*/
	//UE_LOG(LogTemp, Warning, TEXT("I am just respawned: %f"), GetFollowCamera()->FieldOfView);
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		if (GetEquippedWeapon() == nullptr)
		{
			BlasterPlayerController->ShowAmmoHUD(false);
		}
		else
		{
			BlasterPlayerController->ShowAmmoHUD(true);
		}
	}

	UpdateHUDHealth();

	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::RecieveDamage);//��������� ������� ������� ����� ������� ��� ������������� ������� OnTakeAnyDamage
	}
	if(AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PollInit();
	
	/*float Speed = GetCharacterMovement()->Velocity.Size();
	if (HighestSpeed < Speed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Highest speed: %f"), Speed);
		HighestSpeed = Speed;
	}*/
	//Both characters on the server have local role "Authority"
	//SimulatedProxy - is a client at server(AutonomousProxy(client 1)(left window))
	/*if (currentbUseControllerRotationYaw != bUseControllerRotationYaw)
	{
		UE_LOG(LogTemp, Error, TEXT("bUseControllerRotationYaw: %d"), bUseControllerRotationYaw);
		currentbUseControllerRotationYaw = bUseControllerRotationYaw;
	}*/
	//RotateInPlace(DeltaTime);
	
	if (GetLocalRole() == ENetRole::ROLE_SimulatedProxy || HasAuthority())
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	//HideCharacterWhenCameraClose();
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingScope, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, isSprinting);
	DOREPLIFETIME(ABlasterCharacter, BaseSpeed);
	DOREPLIFETIME(ABlasterCharacter, SprintSpeed);
	DOREPLIFETIME(ABlasterCharacter, CurrentHealth);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
	DOREPLIFETIME(ABlasterCharacter, bIsRestoringHealth);
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
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("ShowScoreBoard", IE_Pressed, this, &ABlasterCharacter::ShowScoreBoardPressed);
	PlayerInputComponent->BindAction("ShowScoreBoard", IE_Released, this, &ABlasterCharacter::ShowScoreBoardReleased);
	PlayerInputComponent->BindAction("ChangeOptic", IE_Pressed, this, &ABlasterCharacter::ChangeOpticButtonPressed);
	PlayerInputComponent->BindAction("Shift", IE_Pressed, this, &ABlasterCharacter::ShiftAbilityButtonPressed);
	PlayerInputComponent->BindAction("ChangeView", IE_Pressed, this, &ABlasterCharacter::ChangeViewButtonPressed);
	PlayerInputComponent->BindAction("InitializeHook", IE_Pressed, this, &ABlasterCharacter::InitializeHookButtonPressed);
	PlayerInputComponent->BindAction("EquipFirstWeapon", IE_Pressed, this, &ABlasterCharacter::EquipFirstWeaponButtonPressed);
	PlayerInputComponent->BindAction("EquipSecondWeapon", IE_Pressed, this, &ABlasterCharacter::EquipSecondWeaponButtonPressed);
	PlayerInputComponent->BindAction("OpenInventory", IE_Pressed, this, &ABlasterCharacter::OpenInventory);
	PlayerInputComponent->BindAction("OpenStore", IE_Pressed, this, &ABlasterCharacter::OpenStore);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::ThrowGrenadeButtonPressed);

	if (AbilitySystemComponent && InputComponent)
	{
		const FGameplayAbilityInputBinds Binds("Confirm", "Cancel", "EGASAbilityInputID", static_cast<int32>(EGASAbilityInputID::Confirm), static_cast<int32>(EGASAbilityInputID::Cancel));
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, Binds);
	}
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combatt)
	{
		Combatt->Character = this;
	}
	if(Abilitiess)
	{
		Abilitiess->Character = this;
	}
	if(GrappleComponent)
	{
		GrappleComponent->Character = this;
	}
	if(Inventory)
	{
		Inventory->OwningCharacter = this;
	}
}

USkeletalMeshComponent* ABlasterCharacter::GetCharacterMesh()
{
	if(IsLocallyControlled())
	{
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString("0"));
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("FP Mesh name = %s"), *FPSMesh->GetName()));
		return FPSMesh;
	}
	else
	{
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString("1"));
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("TP Mesh name = %s"), *GetMesh()->GetName()));
		return GetMesh();
	}
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled()) //using offset only for players who are controlling the character
	{
		AimOffset(DeltaTime);
	}
	else if (GetLocalRole() == ENetRole::ROLE_SimulatedProxy || HasAuthority())
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	ABlasterGameMode* BlasterGameMode = Cast< ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchStateNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;

	if(Combatt->EquippedWeapon && bMatchStateNotInProgress)
	{
		Combatt->EquippedWeapon->Destroy();
	}
}

void ABlasterCharacter::Elim()
{
	if (BlasterPlayerController)
	{
		BlasterPlayerController->ShowAmmoHUD(false);
	}

	if (Combatt && Combatt->EquippedWeapon)
	{
		Combatt->EquippedWeapon->Dropped();
	}

	MulticastElim();
	GetWorldTimerManager().SetTimer(//respawn timer
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
	UE_LOG(LogTemp, Warning, TEXT("ElimTimer started"));
}

void ABlasterCharacter::DropItem(class UItem* Item)
{
	if (Item)
	{
		Item->Drop(this);
		Item->OnDrop(this);
	}
}

void ABlasterCharacter::MulticastElim_Implementation()//destroy/respawn/anims/effects
{
	PlayElimMontage();
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}

	if(DissolveMaterialInstance && DissolveMaterialInstance2)//����� DynamicDissolveMaterial ��� ���� character �� �������
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		DynamicDissolveMaterialInstance2 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance2, this);

		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);

		GetMesh()->SetMaterial(1, DynamicDissolveMaterialInstance2);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Dissolve2"), 0.55f);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Glow2"), 200.f);
	}
	StartDissolve();

	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	if(Combatt)
	{
		Combatt->FireButtonPressed(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Spawn Elim bot
	FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);

	if(ElimBotEffect)
	{
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
			ElimBotEffect, 
			ElimBotSpawnPoint, 
			GetActorRotation()
		);
	}
	if(ElimBotSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ElimBotSound, ElimBotSpawnPoint);
	}
	UE_LOG(LogTemp, Warning, TEXT("Multicast started"));

	bool bSniperScopeIsValid = IsLocallyControlled() && 
		isAiming() == true && 
		Combatt->EquippedWeapon && 
		Combatt->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;

	if (bSniperScopeIsValid)
	{
		ShowSniperScopeWidget(false);
		ShowGrenadeLauncherScopeWidget(false);
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>(); //gamemode
	if(BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);//dont check Controller, because already checked it in RequestRespawn()
		bElimmed = false;
	}
	UE_LOG(LogTemp, Warning, TEXT("ElimTimer Finished"));
}

void ABlasterCharacter::UpdateDissoveMaterial(float DissolveValue)//DissolveValue - float value that are getting from curve
{
	if(DynamicDissolveMaterialInstance && DynamicDissolveMaterialInstance2)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);//[0.55, -0.55]
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Dissolve2"), DissolveValue);//[0.55, -0.55]
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissoveMaterial);//when DissolveTrack float delegate updates - updates DissolveValue
	if(DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);//add curve to a timeline
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combatt == nullptr || Combatt->EquippedWeapon == nullptr) return;

	if (bPlayMontage == true) 
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();//���������� � �������� animinstance
		if (AnimInstance && FireWeaponMontage)
		{
			AnimInstance->Montage_Play(FireWeaponMontage);//����������� ����������
			FName SectionName;
			SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
			AnimInstance->Montage_JumpToSection(SectionName);
		}
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimDeathInstance = GetMesh()->GetAnimInstance();
	if(AnimDeathInstance && ElimMontage)
	{
		AnimDeathInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combatt == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();//���������� � �������� animinstance
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayGrenadeThrowMontage()
{
	if (Combatt == nullptr) return;

	UAnimInstance* AnimInstance = FPSMesh->GetAnimInstance();//���������� � �������� animinstance
	if (AnimInstance && GrenadeThrowMontage)
	{
		AnimInstance->Montage_Play(GrenadeThrowMontage);
	}
}

void ABlasterCharacter::MoveForward(float Value)
{
	if (bDisableGameplay)return;
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
	if (bDisableGameplay)return;
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
	if (bDisableGameplay)return;
	if(Combatt)
	{
		if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString("One"));

		if (HasAuthority()) { //�� �������
			Combatt->EquipWeapon(OverlappingWeapon);
			if (Combatt->GetEquippedWeapon() && Combatt->GetEquippedWeapon()->GetWeaponType() == EWeaponType::EWT_AssaultRifle)
			{
				GetEquippedWeapon()->EquipScope(OverlappingScope);
			}
			
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
		
		if(Combatt->GetEquippedWeapon() && Combatt->GetEquippedWeapon()->GetWeaponType() == EWeaponType::EWT_AssaultRifle)
		{
			GetEquippedWeapon()->EquipScope(OverlappingScope);
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay)return;
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
	if (bDisableGameplay)return;
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
	if (bDisableGameplay)return;
	if (!Combatt->EquippedWeapon->bWeaponCanUseOptic)
	{
		GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Red, FString(TEXT("Weapon cant use optic")));
		return;
	}
	if(Combatt)
	{
		Combatt->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (bDisableGameplay || !Combatt->bAiming)return;
	if (Combatt)
	{
		Combatt->SetAiming(false);
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (bDisableGameplay)return;
	if (Combatt)
	{
		Combatt->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (bDisableGameplay)return;
	if (Combatt)
	{
		Combatt->FireButtonPressed(false);
	}
}

void ABlasterCharacter::ChangeOpticButtonPressed()
{
	if (GetEquippedWeapon())
	{
		GetEquippedWeapon()->CycleThroughOptics();
	}
}

void ABlasterCharacter::ShiftAbilityButtonPressed()
{
	if(Abilitiess)
	{
		Abilitiess->ActicateShiftAbility();
	}
}

void ABlasterCharacter::ChangeViewButtonPressed()//false
{
	if(PlayerCamera && TPCamera)
	{
		bChangeCamera = !bChangeCamera;
		if(bChangeCamera)
		{
			TPCamera->SetActive(true);
			PlayerCamera->SetActive(false);
		}
		else
		{
			PlayerCamera->SetActive(true);
			TPCamera->SetActive(false);
		}
	}
}

void ABlasterCharacter::InitializeHookButtonPressed()
{
	if (GrappleComponent)
	{
		GrappleComponent->StartHook();
	}
}

void ABlasterCharacter::EquipFirstWeaponButtonPressed()
{
	if (HasAuthority())
	{
		if (Combatt && Combatt->GetEquippedWeapon())
		{
			Combatt->ChoosePrimaryWeapon();
		}
	}
	else
	{
		ServerEquipFirstWeaponButtonPressed();
	}
}

void ABlasterCharacter::ServerEquipFirstWeaponButtonPressed_Implementation()
{
	if(Combatt)
	{
		if (Combatt->GetEquippedWeapon())
		{
			Combatt->ChoosePrimaryWeapon();
		}
	}
}

void ABlasterCharacter::EquipSecondWeaponButtonPressed()
{
	if (HasAuthority())
	{
		if (Combatt && Combatt->GetEquippedWeapon())
		{
			Combatt->ChooseSecondaryWeapon();
		}
	}
	else
	{
		ServerEquipSecondWeaponButtonPressed();
	}
}

void ABlasterCharacter::ServerEquipSecondWeaponButtonPressed_Implementation()
{
	if (Combatt)
	{
		if (Combatt->GetEquippedWeapon())
		{
			Combatt->ChooseSecondaryWeapon();
		}
	}
}

void ABlasterCharacter::ServerChangeOpticButtonPressed_Implementation()
{
	GetEquippedWeapon()->CycleThroughOptics();
	UE_LOG(LogTemp, Warning, TEXT("ServerC_O Im calling!"));
}

void ABlasterCharacter::SprintButtonPressed()
{
	if (bDisableGameplay)return;
	SetSprint(true);
}

void ABlasterCharacter::SprintButtonReleased()
{
	if (bDisableGameplay)return;
	SetSprint(false);
	if (Combatt->EquippedWeapon && Combatt->bFireButtonPressed)Combatt->Fire();
}

bool ABlasterCharacter::GetIsSprinting()
{
	if (GetCharacterMovement()->Velocity.Size() > BaseSpeed && bPressedJump || GetCharacterMovement()->IsFalling())
	{
		return false;
	}
	if(isSprinting)
	{
		return true;
	}
	return false;
}

void ABlasterCharacter::SetCollisionSettings(ECollisionSettings CurrentCollisionSetting)
{
	switch(CurrentCollisionSetting)
	{
		case ECollisionSettings::ECS_CharacterDefault:
			GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
			GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
			GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
			GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
			GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
			break;
		
		case ECollisionSettings::ECS_WeaponDefault://����� ����� �������� �� currentWeapon
			GetEquippedWeapon()->GetWeaponMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
			GetEquippedWeapon()->GetWeaponMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
			GetEquippedWeapon()->GetWeaponMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
			GetEquippedWeapon()->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;

		case ECollisionSettings::ECS_CharacterGhost:
			GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
			break;

		case ECollisionSettings::ECS_WeaponGhost:
			GetEquippedWeapon()->GetWeaponMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
			break;
	}
}

void ABlasterCharacter::OpenInventory()//temporary foo
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController)
	{
		if (!BlasterPlayerController->bInventotyIsActive)
		{
			BlasterPlayerController->OpenInventory();
			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString("InventoryIsOpened"));
		}
		else
		{
			BlasterPlayerController->CloseInventory();
			if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString("InventoryIsClosed"));
		}
	}
}

void ABlasterCharacter::OpenStore()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		if (!BlasterPlayerController->bStoreIsActive)
		{
			BlasterPlayerController->OpenStore();
		}
		else
		{
			BlasterPlayerController->CloseStore();
		}
	}
}


void ABlasterCharacter::ThrowGrenadeButtonPressed()
{
	if(Combatt)
	{
		Combatt->ThrowGrenade();
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay)return;
	if(Combatt)
	{
		Combatt->Reload();
	}
}

void ABlasterCharacter::PlayReloadingMontage()
{
	if (Combatt == nullptr || Combatt->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = FPSMesh->GetAnimInstance();
	if (ReloadingMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(ReloadingMontage);
		FName SectionName;

		switch (Combatt->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle :
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_ShotGun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
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

void ABlasterCharacter::ShowScoreBoardPressed()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController && !BlasterPlayerController->GetCooldownIsHandled()) BlasterPlayerController->ShowScoreBoard();
}

void ABlasterCharacter::ShowScoreBoardReleased()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if(BlasterPlayerController && !BlasterPlayerController->GetCooldownIsHandled()) BlasterPlayerController->CloseScoreBoard();
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
	//UE_LOG(LogTemp, Warning, TEXT("AO_Pitch1 %f: "), AO_Pitch);
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
	//UE_LOG(LogTemp, Error, TEXT("AO_Pitch2 %f: "), AO_Pitch);
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



void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if(TurningInPlace != ETurningInPlace::ETIP_NotTurning)//if we are turning
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 6.f);
		AO_Yaw = InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw) < 10.f)//�������� ��������
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

/*void ABlasterCharacter::HideCharacterWhenCameraClose()
{
	if (!IsLocallyControlled()) return; //should only be executed by the locally controlled player

	if((FollowCameraa->GetComponentLocation() - GetActorLocation()).Size() < CameraTrashold)
	{
		bPlayMontage = false;
		GetMesh()->SetVisibility(false);
		if(Combatt && Combatt->EquippedWeapon && Combatt->EquippedWeapon->GetWeaponMesh())
		{
			Combatt->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;//SetVisibility(true)
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
}*/

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())//���������� PickupWidget ������ ���� ��� overlap sphere
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::SetOverlappingScope(AScope* Scope)
{
	if(OverlappingScope)
	{
		OverlappingScope->ShowPickupWidget(false);
	}
	OverlappingScope = Scope;
	if(IsLocallyControlled())
	{
		if(OverlappingScope)
		{
			OverlappingScope->ShowPickupWidget(true);
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

void ABlasterCharacter::OnRep_OverlappingScope(AScope* LastScope)
{
	if (OverlappingScope)
	{
		OverlappingScope->ShowPickupWidget(true);
	}
	if (LastScope)
	{
		LastScope->ShowPickupWidget(false);
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

AScope* ABlasterCharacter::GetEquippedScope()
{
	if (Combatt == nullptr) return nullptr;
	return Combatt->EquippedWeapon->EquippedScope;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if(Combatt == nullptr) return FVector();
	return Combatt->HitTarget;
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (Combatt == nullptr) return ECombatState::ECS_MAX;
	return Combatt->CombatState;
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

void ABlasterCharacter::RestoreSomeHealth(int32 AmountToRestore)
{
	if (CurrentHealth >= MaxHealth) return;
	if(CurrentHealth > 0.f)
	{
		bIsRestoringHealth = true;
		int32 NewHealth = CurrentHealth + AmountToRestore;
		GetWorldTimerManager().SetTimer(HealthRestoreTimer, [this, NewHealth]()
			{
				IncrementHealth(NewHealth);
			}, HealthRestoringDelay, true);
	}
}

void ABlasterCharacter::IncrementHealth(int32 DesiredHealth)
{
	if (CurrentHealth >= DesiredHealth)
	{
		UE_LOG(LogTemp, Error, TEXT("DesiredHealth is out of range."));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("Current health: %f"), CurrentHealth));

		return;
	}
	CurrentHealth = FMath::Clamp(FMath::Lerp(CurrentHealth, DesiredHealth, GetWorld()->GetDeltaSeconds() * 5.f), 0.0f, MaxHealth);
	UpdateHUDHealth();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Current health: %f"), CurrentHealth));

	if (FMath::IsNearlyEqual(CurrentHealth, DesiredHealth, 0.9f))
	{
		GetWorldTimerManager().ClearTimer(HealthRestoreTimer);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("Stop"));
		bIsRestoringHealth = false;
		CurrentHealth = DesiredHealth;
	}
}

void ABlasterCharacter::RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (CurrentHealth == 0.f)//not "<=" cause CurrentHealth clamped between 0 and 100
	{
		bElimmed = true;
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>(); //gamemode
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;//make sure that BlasterPlayerController is set
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);// this - this character
		}
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;//allows to not cast multiple times(make sure that BlasterPlayerController is set) 
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(CurrentHealth, MaxHealth);
	}
	//UELogInfo(CurrentHealth);
}

void ABlasterCharacter::PollInit()//update hud values
{
	if (BlasterPlayerState == nullptr)//player state ����������� ��������� �� � ������� ������ -> ��� �� �������� � BeginPlay, ���� � ������� == nullptr
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if(BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
		}
	}
}

void ABlasterCharacter::Buy(int32 StoreSlotIndex)
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;//allows to not cast multiple times(make sure that BlasterPlayerController is set) 
	if (BlasterPlayerController)
	{
		
	}
}

void ABlasterCharacter::UELogInfo(float Value)
{
	float Speed = GetCharacterMovement()->Velocity.Length();
	UE_LOG(LogTemp, Error, TEXT("value: %d"), Value);
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


//1. ������ ActionMapping
//2. ������� void *Pressed/Released
//3. ������ PlayerInputComponent
//4. ������ foo � bool ����������, � ������ �������� �� ������ ����������� �������� fe: "Fire" - Combat class
//5. �������� ������� �� ������ "Combat" � foo Pr/Re � ��������� �������� true/false