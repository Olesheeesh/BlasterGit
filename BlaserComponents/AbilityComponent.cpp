#include "AbilityComponent.h"

#include "CombatComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "TimerManager.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
//в .cpp будут вызываться способности и реализованы функция переключения способностей и тд
UAbilityComponent::UAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	NiagaraComponent= CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponentt"));
}

void UAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UAbilityComponent, ShiftActivationLocation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UAbilityComponent, ShiftActivationRotation, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UAbilityComponent, CanShift, COND_SkipOwner);
}


void UAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAbilityComponent::BlinkAbility()
{
	if (Character->GetCharacterMovement()->IsFalling()) return;
	if (Character->GetVelocity().Size() > 0.f)
	{
		LaunchDirection = Character->GetVelocity().GetSafeNormal();
		Character->LaunchCharacter(LaunchDirection * ImpulseStrength, false, false);

	}
	else
	{
		LaunchDirection = Character->GetActorForwardVector();
		Character->LaunchCharacter(LaunchDirection * ImpulseStrength, false, false);

	}
}

void UAbilityComponent::ActicateShiftAbility()
{
	if (!CanShift || Character->GetMovementComponent()->IsFalling()) return;
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Character && CanShift)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString("Activate is calling"));

		ShiftActivationLocation = Character->GetActorLocation();
		ShiftActivationRotation = Character->GetActorRotation();

		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
		
		UE_LOG(LogTemp, Warning, TEXT("AfterImageSystem: %s"), *AfterImageSystem->GetName());
		UE_LOG(LogTemp, Warning, TEXT("ShiftActivationLocation: %s"), *ShiftActivationLocation.ToString());
		UE_LOG(LogTemp, Warning, TEXT("ShiftActivationRotation: %s"), *ShiftActivationRotation.ToString());

		NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			AfterImageSystem,
			ShiftActivationLocation,
			ShiftActivationRotation
		);
		if (NiagaraComponent)
		{
			NiagaraComponent->Activate();
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString("AfterImageSystem is calling"));
		}

		if (NiagaraComponent->IsActive()){ if (GEngine)GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("NiagaraComponent IsActive"));}

	}
	BlinkAbility();

	StartShiftTimer();
	StartShiftDelayTimer();
	CanShift = false;

	ServerActivateShift(ShiftActivationLocation, ShiftActivationRotation);
}


void UAbilityComponent::ServerActivateShift_Implementation(FVector Loc, FRotator Rot)
{
	MulticastActivateShift(Loc, Rot);
}

void UAbilityComponent::MulticastActivateShift_Implementation(FVector Loc, FRotator Rot)
{
	if (Character && CanShift)
	{
		// Активация сдвига (shift) на всех клиентах
		ShiftActivationLocation = Loc;
		ShiftActivationRotation = Rot;

		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);

		UE_LOG(LogTemp, Warning, TEXT("AfterImageSystem: %s"), *AfterImageSystem->GetName());
		UE_LOG(LogTemp, Warning, TEXT("ShiftActivationLocation: %s"), *ShiftActivationLocation.ToString());
		UE_LOG(LogTemp, Warning, TEXT("ShiftActivationRotation: %s"), *ShiftActivationRotation.ToString());

		NiagaraComponent = Character->ShiftAbilitySystemComponent;

		NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			AfterImageSystem,
			ShiftActivationLocation,
			ShiftActivationRotation
		);
		if (NiagaraComponent)
		{
			NiagaraComponent->Activate();
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString("AfterImageSystem is calling"));
		}
		if(NiagaraComponent->IsActive())
		{
			if(GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("NiagaraComponent IsActive"));
		}

		// Выполнение вашей логики сдвига (shift) на всех клиентах
		BlinkAbility();

		StartShiftTimer();
		StartShiftDelayTimer();
		CanShift = false;
	}
}


void UAbilityComponent::StartShiftTimer()
{
	Character->GetWorldTimerManager().SetTimer(
		ShiftDurationTimer,
		this,
		&UAbilityComponent::ShiftTimerFinished,
		ShiftDuration
	);
}

void UAbilityComponent::StartShiftDelayTimer()
{
	Character->GetWorldTimerManager().SetTimer(
		ShiftDelayTimer,
		this,
		&UAbilityComponent::ShiftTimerDelayFinished,
		ShiftAbilityDelay
	);
}

void UAbilityComponent::ShiftTimerFinished()
{
	if (Character == nullptr) return;
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
}

void UAbilityComponent::ShiftTimerDelayFinished()
{
	if (Character == nullptr) return;
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString("You can shift now"));
	CanShift = true;
}
