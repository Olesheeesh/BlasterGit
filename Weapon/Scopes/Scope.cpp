#include "Scope.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"

AScope::AScope()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	SetReplicateMovement(true);

	Scope = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Scope"));
	Scope->SetupAttachment(RootComponent);
	SetRootComponent(Scope);

	Scope->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	Scope->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	Scope->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	Scope->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

}

void AScope::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{//for server
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AScope::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AScope::OnSphereEndOverlap);
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

void AScope::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AScope::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AScope, ScopeState);
}

void AScope::SetScopeState(EScopeState State)
{
	ScopeState = State;
	switch (ScopeState)
	{
	case EScopeState::ESS_Equipped:
		ShowPickupWidget(false);
		Scope->SetSimulatePhysics(false);
		Scope->SetEnableGravity(false);
		Scope->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EScopeState::ESS_Dropped:
		ShowPickupWidget(true);
		Scope->SetSimulatePhysics(true);
		Scope->SetEnableGravity(true);
		Scope->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Current State is: %hhd"), State));
}

void AScope::OnRep_ScopeState()
{
	switch (ScopeState)
	{
	case EScopeState::ESS_Equipped:
		ShowPickupWidget(false);
		Scope->SetSimulatePhysics(false);
		Scope->SetEnableGravity(false);
		Scope->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EScopeState::ESS_Dropped:
		ShowPickupWidget(true);
		Scope->SetSimulatePhysics(true);
		Scope->SetEnableGravity(true);
		Scope->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Orange, FString::Printf(TEXT("Current State is: %hhd"), ScopeState));
}

void AScope::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor);//проверка что пересекает именно ABlasterCharacter
	if (Character)//если преобразование AActor -> ABlasterCharacter успешно - выполнить код(OtherActor == ABlasterCharacter)
	{
		Character->SetOverlappingScope(this);
	}
}

void AScope::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor);
	if (Character)
	{
		Character->SetOverlappingScope(nullptr);
	}
}

void AScope::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}
void AScope::FillOpticsArray()
{
	
}



