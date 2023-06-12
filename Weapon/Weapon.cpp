#include "Weapon.h"
#include "BulletShell.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Blaster/BlaserComponents/CombatComponent.h"
#include "Blaster/Character/BlasterAnimInstance.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Scopes/Scope.h"
#include "Sound/SoundCue.h"

AWeapon::AWeapon()
{
	bReplicates = true; //class is replicated(spawns on all machines) will exist independetly of the server
	PrimaryActorTick.bCanEverTick = false;
	SetReplicateMovement(true);//actor's movement replicates to clients

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}


void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority()) 
	{//for server
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState); //here we register variable that we want to replicate
	DOREPLIFETIME(AWeapon, Ammo); //here we register variable that we want to replicate
	DOREPLIFETIME(AWeapon, EquippedScope);//when changes, it will be reflected on all clients
	DOREPLIFETIME_CONDITION(AWeapon, OpticIndex, COND_SkipOwner);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);//проверка что пересекает именно ABlasterCharacter
	if(BlasterCharacter)//если преобразование AActor -> ABlasterCharacter успешно - выполнить код(OtherActor == ABlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SpendRound()//update for server
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
	if (WeaponType == EWeaponType::EWT_SniperRifle)
	{
		BlasterOwnerCharacter->GetCombatComponent()->SetAiming(false);
		BlasterOwnerCharacter->ShowSniperScopeWidget(false);
	}
}

void AWeapon::OnRep_Ammo()//update for client
{
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if(Owner == nullptr)//for clients
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		SetHUDAmmo();//if Owner != nullptr -> set HUD
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		break;
	case EWeaponState::EWS_Dropped:
		ShowPickupWidget(false);
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
			WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
			WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		}
		break;
	}
}

void AWeapon::OnRep_WeaponState()//for client
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if(WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		break;
	case EWeaponState::EWS_Dropped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
			WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
			WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		}
		break;
	};
}


void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if(PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (ShellClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName("AmmoEject");
		if(AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);//координаты затвора сокета
			if (ShellClass)
			{
				UWorld* World = GetWorld();
				if (World)
				{
					World->SpawnActor<ABulletShell>(
						ShellClass,
						SocketTransform.GetLocation(),
						SocketTransform.GetRotation().Rotator()//Fquat rotation, so getting rotation with Rotator()
					);
				}
			}
		}
	}
	SpendRound();
}

void AWeapon::Dropped()//detach
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;//to not change the hudd for last owner character
}

bool AWeapon::IsEmpty()
{
	return Ammo <= 0;//если пустая обойма -> вернет false
}

bool AWeapon::MagIsFull()
{
	return Ammo == MagCapacity;
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}


void AWeapon::EquipScope(AScope* ScopeToEquip)
{
	EquippedWeapon = BlasterOwnerCharacter->GetCombatComponent()->GetEquippedWeapon();

	if (BlasterOwnerCharacter == nullptr || ScopeToEquip == nullptr || EquippedWeapon == nullptr || Optics.Num() >= 3) return;
	AnimInstance = AnimInstance == nullptr ? Cast<UBlasterAnimInstance>(BlasterOwnerCharacter->GetMesh()->GetAnimInstance()) : AnimInstance;

	EquippedScope = ScopeToEquip;
	EquippedScope->SetScopeState(EScopeState::ESS_Equipped);
	if (CurrentScope == nullptr)
	{
		CurrentScope = EquippedScope;
	}

	Optics.AddUnique(EquippedScope);

	FString SocketName = FString::Printf(TEXT("WeaponSightSocket%d"), Optics.Num() - 1);

	const USkeletalMeshSocket* WeaponSightSocket = EquippedWeapon->GetWeaponMesh()->GetSocketByName(*SocketName);

	if (WeaponSightSocket)
	{
		WeaponSightSocket->AttachActor(EquippedScope, EquippedWeapon->GetWeaponMesh());
	}

	if (AnimInstance && EquippedWeapon)
	{
		AnimInstance->SetRelativeHandTransform();
		AnimInstance->ChangeOptic();
	}

	if (EquippedScope->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedScope->EquipSound,
			BlasterOwnerCharacter->GetActorLocation()
		);
	}

	EquippedScope->SetOwner(BlasterOwnerCharacter);
}

void AWeapon::OnRep_EquippedScope()
{
	EquippedWeapon = BlasterOwnerCharacter->GetCombatComponent()->GetEquippedWeapon();

	if (EquippedWeapon && EquippedScope && BlasterOwnerCharacter)
	{
		AnimInstance = AnimInstance == nullptr ? Cast<UBlasterAnimInstance>(BlasterOwnerCharacter->GetMesh()->GetAnimInstance()) : AnimInstance;

		EquippedScope->SetScopeState(EScopeState::ESS_Equipped);
		if (CurrentScope == nullptr)
		{
			CurrentScope = EquippedScope;
		}

		Optics.AddUnique(EquippedScope);

		FString SocketName = FString::Printf(TEXT("WeaponSightSocket%d"), Optics.Num() - 1);

		const USkeletalMeshSocket* WeaponSightSocket = EquippedWeapon->GetWeaponMesh()->GetSocketByName(*SocketName);

		if (WeaponSightSocket)
		{
			WeaponSightSocket->AttachActor(EquippedScope, EquippedWeapon->GetWeaponMesh());
		}

		if (AnimInstance && EquippedWeapon)
		{
			AnimInstance->SetRelativeHandTransform();
			AnimInstance->ChangeOptic();
		}

		if (EquippedScope->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				EquippedScope->EquipSound,
				BlasterOwnerCharacter->GetActorLocation()
			);
		}

		EquippedScope->SetOwner(BlasterOwnerCharacter);
	}
}

void AWeapon::CycleThroughOptics()
{
	if (BlasterOwnerCharacter == nullptr || EquippedScope == nullptr) return;
	AnimInstance = AnimInstance == nullptr ? Cast<UBlasterAnimInstance>(BlasterOwnerCharacter->GetMesh()->GetAnimInstance()) : AnimInstance;

	if (AnimInstance && CurrentScope && EquippedScope && BlasterOwnerCharacter->GetMesh())
	{
		if (++OpticIndex >= Optics.Num())
		{
			OpticIndex = 0;
		}
		CurrentScope = Optics[OpticIndex];

		if (!BlasterOwnerCharacter->HasAuthority())
		{
			ServerSetOpticIndex(OpticIndex);
		}
		AnimInstance->ChangeOptic();
	}
}

void AWeapon::ServerSetOpticIndex_Implementation(uint8 CurrentIndex)
{
	AnimInstance = AnimInstance == nullptr ? Cast<UBlasterAnimInstance>(BlasterOwnerCharacter->GetMesh()->GetAnimInstance()) : AnimInstance;

	OpticIndex = CurrentIndex;
	CurrentScope = Optics[OpticIndex];

	AnimInstance->ChangeOptic();
}

void AWeapon::OnRep_OpticIndex()
{
	if (BlasterOwnerCharacter == nullptr || EquippedScope == nullptr) return;
	AnimInstance = AnimInstance == nullptr ? Cast<UBlasterAnimInstance>(BlasterOwnerCharacter->GetMesh()->GetAnimInstance()) : AnimInstance;

	CurrentScope = Optics[OpticIndex];

	AnimInstance->ChangeOptic();
}





