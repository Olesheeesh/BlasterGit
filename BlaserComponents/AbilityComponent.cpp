#include "AbilityComponent.h"

#include "CombatComponent.h"
#include "NiagaraComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "TimerManager.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UAbilityComponent::UAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME()
}


void UAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
	OnAfterImageFinished = FOnAfterImageFinished();
}

void UAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UAbilityComponent::BlinkAbility()
{
	if (Character->GetCharacterMovement()->IsFalling()) return;
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString("Blink is calling"));
	if (Character->GetVelocity().Size() > 0.f)
	{
		FVector LaunchDirection = Character->GetVelocity().GetSafeNormal();
		Character->LaunchCharacter(LaunchDirection * ImpulseStrength, false, false);
	}
	else
	{
		FVector LaunchDirection = Character->GetActorForwardVector();
		Character->LaunchCharacter(LaunchDirection * ImpulseStrength, false, false);
	}
}

void UAbilityComponent::OnAfterImageSystemFinished(UNiagaraComponent* NiagaraComponent)
{
	UE_LOG(LogTemp, Warning, TEXT("AfterImageComponent System Finished"));
	OnAfterImageFinished.Broadcast();
	BlinkAbility();
}

void UAbilityComponent::ActicateShiftAbility()
{
	if (!CanShift) return;
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Character && CanShift && Character->GetAfterImageComponent())
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, FString("Activate is calling"));

		ShiftActivationLocation = Character->GetActorLocation();

		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);

		Character->GetAfterImageComponent()->Activate();
		Character->GetAfterImageComponent()->OnSystemFinished.AddDynamic(this, &UAbilityComponent::OnAfterImageSystemFinished);

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
	if (Character && CanShift && Character->GetAfterImageComponent())
	{
		ShiftActivationLocation = Character->GetActorLocation();

		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);

		Character->GetAfterImageComponent()->Activate();

		StartShiftTimer();
		CanShift = false;
	}
}

void UAbilityComponent::StartShiftTimer()
{
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

	Character->GetAfterImageComponent()->Deactivate();

	CanShift = true;
}
