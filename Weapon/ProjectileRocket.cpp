// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Engine/StaticMeshSocket.h"
#include "Kismet/GameplayStatics.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();
	if(FiringPawn)
	{
		AController* FiringContoler = FiringPawn->GetController();
		if(FiringContoler)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, //world context object
				Damage, //base damage
				10.f, //min damage
				GetActorLocation(), //origin(center of radius)
				DamageInnerRadius,
				DamageOuterRadius,
				1.f, //damage will decrease steadily for actors, the farther away they are from InnerRadius and that damage decrease will be linear
				UDamageType::StaticClass(), //DamageTypeClass
				TArray<AActor*>(),//can add actors to ignore
				this,
				FiringContoler //instigator controller
			);
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
