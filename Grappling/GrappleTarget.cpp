#include "GrappleTarget.h"

#include "Components/BillboardComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AGrappleTarget::AGrappleTarget()
{
	PrimaryActorTick.bCanEverTick = true;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("TargetBillboard"));
	SetRootComponent(Billboard);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TargetWidget"));
	WidgetComponent->SetupAttachment(Billboard);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TargetSphere"));
	AreaSphere->SetupAttachment(WidgetComponent);

}

void AGrappleTarget::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGrappleTarget::SetActive(bool bActive)
{
	WidgetComponent->SetVisibility(bActive);
}

void AGrappleTarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

