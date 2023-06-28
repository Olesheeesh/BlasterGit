#include "GrapplingHook.h"

AGrapplingHook::AGrapplingHook()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

}

void AGrapplingHook::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGrapplingHook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

