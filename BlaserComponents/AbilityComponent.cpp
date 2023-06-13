#include "AbilityComponent.h"

#include "CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "TimerManager.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

UAbilityComponent::UAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME()
}

void UAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAbilityComponent::ActicateShiftAbility()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString("Activate is calling"));
	if (Character && CanShift)
	{
		ShiftActivationLocation = Character->GetActorLocation();

		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);

		bIsCollisionEnabled = false;
		UE_LOG(LogTemp, Error, TEXT("Collision: %d"), bIsCollisionEnabled)

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString("Im invincible now"));
		StartShiftTimer();
		CanShift = false;

		ServerActivateShift();
	}
}

void UAbilityComponent::ServerActivateShift_Implementation()
{
	MulticastActivateShift();
}

void UAbilityComponent::MulticastActivateShift_Implementation()
{
	if (Character && CanShift)
	{
		ShiftActivationLocation = Character->GetActorLocation();

		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);

		bIsCollisionEnabled = false;

		StartShiftTimer();
		CanShift = false;
	}
}

void UAbilityComponent::StartShiftTimer()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString("Start timer Is calling"));
	Character->GetWorldTimerManager().SetTimer(
		ShiftTimer,
		this,
		&UAbilityComponent::ShiftTimerFinished,
		ShiftDuration
	);
}

void UAbilityComponent::ShiftTimerFinished()
{
	if (Character == nullptr) return;
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	bIsCollisionEnabled = true;
	CanShift = true;
	UE_LOG(LogTemp, Error, TEXT("Collision: %d"), bIsCollisionEnabled)
	if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString("Im not invincible anymore"));
}
